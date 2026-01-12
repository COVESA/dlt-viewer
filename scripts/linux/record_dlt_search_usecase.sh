#!/usr/bin/env bash
set -euo pipefail

# Records CPU + memory metrics for the "search use-case" window of dlt-viewer.
# Typical flow:
#   1) Start dlt-viewer, load log file, get to the point right before Search.
#   2) Run this script.
#   3) Perform two searches (text + regex).
#   4) Press Enter in this script to stop and save results.

usage() {
  cat <<'EOF'
Usage:
  record_dlt_search_usecase.sh [--tag TAG] [--pid PID] [--process REGEX] [--interval SEC] [--outdir DIR]
                             [--no-perf] [--perf-record] [--perf-freq HZ] [--callgraph MODE]

Examples:
  ./record_dlt_search_usecase.sh --tag baseline
  ./record_dlt_search_usecase.sh --tag improvement_1 --interval 1
  ./record_dlt_search_usecase.sh --pid 12345 --tag baseline
  ./record_dlt_search_usecase.sh --tag baseline_callgraph --perf-record
  ./record_dlt_search_usecase.sh --tag baseline --perf-record --callgraph dwarf

What it captures:
  - pidstat (CPU%, RSS, threads, minor/major faults, ctx switches)
  - perf stat interval counters (optional; requires perf and usually sudo)
  - perf record samples (optional; produces perf.data for flamegraphs/profiles)
  - /proc/<pid>/status and smaps_rollup snapshots (before/after)
  - metadata (uname, git rev, cmdline)

Callgraph behavior:
  - If --callgraph is provided, it is used.
  - Otherwise, if --tag contains "callgraph" or "cg", callgraph is enabled with MODE=dwarf.
  - Otherwise, perf record runs without callgraph.

Callgraph modes:
  fp    : lower overhead, requires frame pointers for best results.
  dwarf : higher overhead, works without frame pointers.
EOF
}

TAG=""
PID=""
PROCESS_REGEX="dlt-viewer|dlt_viewer|dltviewer"
INTERVAL_SEC=1
OUTDIR_BASE=""
ENABLE_PERF=1
ENABLE_PERF_RECORD=0
PERF_FREQ_HZ=99
CALLGRAPH_MODE=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --tag)
      TAG="${2:-}"; shift 2 ;;
    --pid)
      PID="${2:-}"; shift 2 ;;
    --process)
      PROCESS_REGEX="${2:-}"; shift 2 ;;
    --interval)
      INTERVAL_SEC="${2:-}"; shift 2 ;;
    --outdir)
      OUTDIR_BASE="${2:-}"; shift 2 ;;
    --no-perf)
      ENABLE_PERF=0; shift ;;
    --perf-record)
      ENABLE_PERF_RECORD=1; shift ;;
    --perf-freq)
      PERF_FREQ_HZ="${2:-}"; shift 2 ;;
    --callgraph)
      CALLGRAPH_MODE="${2:-}"; shift 2 ;;
    -h|--help)
      usage; exit 0 ;;
    *)
      echo "Unknown arg: $1" >&2
      usage
      exit 2
      ;;
  esac
done

if [[ "$PERF_FREQ_HZ" != "" ]] && ! [[ "$PERF_FREQ_HZ" =~ ^[0-9]+$ ]]; then
  echo "--perf-freq must be an integer (Hz). Got: $PERF_FREQ_HZ" >&2
  exit 2
fi

if ! [[ "$INTERVAL_SEC" =~ ^[0-9]+([.][0-9]+)?$ ]]; then
  echo "--interval must be a number (seconds). Got: $INTERVAL_SEC" >&2
  exit 2
fi

require_cmd() {
  local cmd="$1"
  if ! command -v "$cmd" >/dev/null 2>&1; then
    echo "Missing dependency: $cmd" >&2
    return 1
  fi
}

if ! require_cmd pidstat; then
  echo "Install sysstat (pidstat) and try again." >&2
  exit 1
fi

if [[ -z "$PID" ]]; then
  # Use most recently started matching process.
  PID="$(pgrep -n -f "$PROCESS_REGEX" || true)"
fi

if [[ -z "$PID" ]]; then
  echo "Could not find dlt-viewer PID. Either start the app first or pass --pid." >&2
  echo "Tried process regex: $PROCESS_REGEX" >&2
  exit 1
fi

if ! [[ "$PID" =~ ^[0-9]+$ ]]; then
  echo "Invalid PID: $PID" >&2
  exit 2
fi

if [[ ! -d "/proc/$PID" ]]; then
  echo "PID $PID does not exist (/proc/$PID missing)." >&2
  exit 1
fi

TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
TAG_SAFE="${TAG//[^A-Za-z0-9._-]/_}"
RUN_NAME="search_${TIMESTAMP}"
if [[ -n "$TAG_SAFE" ]]; then
  RUN_NAME+="_${TAG_SAFE}"
fi

if [[ -z "$OUTDIR_BASE" ]]; then
  OUTDIR_BASE="$(pwd)/perf_runs"
fi

OUTDIR="$OUTDIR_BASE/$RUN_NAME"
mkdir -p "$OUTDIR"

META="$OUTDIR/meta.txt"
{
  echo "timestamp=$TIMESTAMP"
  echo "tag=$TAG"
  echo "pid=$PID"
  echo "process_regex=$PROCESS_REGEX"
  echo "interval_sec=$INTERVAL_SEC"
  echo "hostname=$(hostname)"
  echo "uname=$(uname -a)"
  if command -v git >/dev/null 2>&1 && git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "git_rev=$(git rev-parse HEAD 2>/dev/null || true)"
    echo "git_status_short=$(git status --porcelain 2>/dev/null | wc -l | tr -d ' ') files_dirty"
  fi
  echo "cmdline=$(tr '\0' ' ' < /proc/$PID/cmdline)"
} > "$META"

# Snapshots (before)
cp "/proc/$PID/status" "$OUTDIR/proc_status_before.txt" || true
cp "/proc/$PID/smaps_rollup" "$OUTDIR/proc_smaps_rollup_before.txt" || true

PIDSTAT_LOG="$OUTDIR/pidstat.txt"
PERF_STAT_LOG="$OUTDIR/perf_stat_interval.txt"

PIDSTAT_PID=""
PERF_PID=""
PERF_RECORD_PID=""

if [[ -z "$CALLGRAPH_MODE" ]]; then
  # Enable callgraph based on tag naming convention.
  # Examples: --tag baseline_callgraph, --tag improvement_cg
  shopt -s nocasematch
  if [[ "$TAG" == *callgraph* || "$TAG" == *cg* ]]; then
    CALLGRAPH_MODE="dwarf"
  fi
  shopt -u nocasematch
fi

if [[ -n "$CALLGRAPH_MODE" ]]; then
  case "$CALLGRAPH_MODE" in
    fp|dwarf) ;;
    *)
      echo "Invalid --callgraph value: $CALLGRAPH_MODE (expected: fp|dwarf)" >&2
      exit 2
      ;;
  esac
fi

cleanup() {
  set +e
  if [[ -n "$PIDSTAT_PID" ]] && kill -0 "$PIDSTAT_PID" >/dev/null 2>&1; then
    kill "$PIDSTAT_PID" >/dev/null 2>&1 || true
    wait "$PIDSTAT_PID" >/dev/null 2>&1 || true
  fi
  if [[ -n "$PERF_PID" ]] && kill -0 "$PERF_PID" >/dev/null 2>&1; then
    kill "$PERF_PID" >/dev/null 2>&1 || true
    wait "$PERF_PID" >/dev/null 2>&1 || true
  fi
  if [[ -n "$PERF_RECORD_PID" ]] && kill -0 "$PERF_RECORD_PID" >/dev/null 2>&1; then
    kill -INT "$PERF_RECORD_PID" >/dev/null 2>&1 || true
    wait "$PERF_RECORD_PID" >/dev/null 2>&1 || true
  fi

  # Snapshots (after)
  cp "/proc/$PID/status" "$OUTDIR/proc_status_after.txt" || true
  cp "/proc/$PID/smaps_rollup" "$OUTDIR/proc_smaps_rollup_after.txt" || true
}
trap cleanup EXIT INT TERM

echo "Recording search window for PID=$PID"
echo "Output directory: $OUTDIR"
echo "Start your two searches now (text + regex)."
echo "Press Enter here when done to stop recording."

# pidstat: CPU + memory + threads, 1 sample per INTERVAL_SEC
# -h avoids repeating headers.
(pidstat -h -r -u -d -t -p "$PID" "$INTERVAL_SEC" > "$PIDSTAT_LOG") &
PIDSTAT_PID=$!

if [[ "$ENABLE_PERF" -eq 1 ]]; then
  if require_cmd perf; then
    # perf stat writes to stderr by default.
    # Convert seconds to integer ms; supports fractional seconds via awk.
    PERF_INTERVAL_MS="$(awk -v s="$INTERVAL_SEC" 'BEGIN{ms=int(s*1000+0.5); if(ms<1) ms=1; print ms}')"

    # Try without sudo first; fall back to sudo if needed.
    set +e
    perf stat -I "$PERF_INTERVAL_MS" -p "$PID" \
      -e cycles,instructions,branches,branch-misses,cache-references,cache-misses,context-switches,cpu-migrations,page-faults \
      1>/dev/null 2>"$PERF_STAT_LOG" &
    PERF_PID=$!
    sleep 0.2
    if ! kill -0 "$PERF_PID" >/dev/null 2>&1; then
      # likely failed immediately; retry with sudo
      sudo perf stat -I "$PERF_INTERVAL_MS" -p "$PID" \
        -e cycles,instructions,branches,branch-misses,cache-references,cache-misses,context-switches,cpu-migrations,page-faults \
        1>/dev/null 2>"$PERF_STAT_LOG" \
        &
      PERF_PID=$!
    fi
    set -e
  else
    echo "perf not found; skipping perf stat. (Install linux-tools / perf)" | tee -a "$META" >/dev/null
  fi
fi

if [[ "$ENABLE_PERF_RECORD" -eq 1 ]]; then
  if require_cmd perf; then
    PERF_DATA="$OUTDIR/perf.data"
    PERF_RECORD_LOG="$OUTDIR/perf_record.txt"

    PERF_RECORD_ARGS=(record -F "$PERF_FREQ_HZ" -p "$PID" -o "$PERF_DATA")
    if [[ -n "$CALLGRAPH_MODE" ]]; then
      PERF_RECORD_ARGS+=(--call-graph "$CALLGRAPH_MODE")
    fi

    set +e
    perf "${PERF_RECORD_ARGS[@]}" 1>/dev/null 2>"$PERF_RECORD_LOG" &
    PERF_RECORD_PID=$!
    sleep 0.2
    if ! kill -0 "$PERF_RECORD_PID" >/dev/null 2>&1; then
      sudo perf "${PERF_RECORD_ARGS[@]}" 1>/dev/null 2>"$PERF_RECORD_LOG" &
      PERF_RECORD_PID=$!
    fi
    set -e
  else
    echo "perf not found; skipping perf record. (Install linux-tools / perf)" | tee -a "$META" >/dev/null
  fi
fi

read -r _

echo "Stopping... writing after-snapshots." 

# cleanup runs via trap

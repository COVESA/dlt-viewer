#!/usr/bin/env python3
"""Generate sample DLT files for testing dlt-viewer without real traces.

The payload of every message is one line of ``git log`` output of this (or any)
repository, so the generated file is human readable. The history is replayed
oldest commit first and each message carries the real commit date as its DLT
time, so the time and timestamp columns behave like a genuine trace. Messages
are spread over several application ids, context ids, ECUs, session ids and
log levels so that the filter and sort features of the viewer can be
exercised.

Examples:
    # 10 MB sample from the current repository's git history
    python3 scripts/generate_sample_dlt.py --size 10mb -o sample.dlt

    # 1 GB sample
    python3 scripts/generate_sample_dlt.py --size 1gb

    # use the git history of another repository
    python3 scripts/generate_sample_dlt.py --size 100kb --repo ~/src/linux

The file is written in the DLT v1 storage format understood by dlt-viewer:
storage header (little endian) + standard header (big endian) + extended
header + verbose payload (little endian, MSBF not set).
"""

import argparse
import os
import random
import struct
import subprocess
import sys
import time

# DLT protocol constants (see qdlt/dlt_protocol.h)
DLT_HTYP_UEH = 0x01   # use extended header
DLT_HTYP_WEID = 0x04  # with ECU id
DLT_HTYP_WSID = 0x08  # with session id
DLT_HTYP_WTMS = 0x10  # with timestamp
DLT_HTYP_VERS1 = 0x20  # protocol version 1

DLT_MSIN_VERB = 0x01
DLT_TYPE_LOG = 0x00

DLT_LOG_FATAL = 0x1
DLT_LOG_ERROR = 0x2
DLT_LOG_WARN = 0x3
DLT_LOG_INFO = 0x4
DLT_LOG_DEBUG = 0x5
DLT_LOG_VERBOSE = 0x6

DLT_TYPE_INFO_UINT = 0x00000040
DLT_TYPE_INFO_STRG = 0x00000200
DLT_TYLE_32BIT = 0x03
DLT_SCOD_UTF8 = 0x00008000

STORAGE_HEADER = struct.Struct("<4sIi4s")       # pattern, seconds, microseconds, ecu
STANDARD_HEADER = struct.Struct(">BBH")         # htyp, mcnt, len (big endian)
HEADER_EXTRA = struct.Struct(">4sII")           # ecu, session id, timestamp (big endian)
EXTENDED_HEADER = struct.Struct(">BB4s4s")      # msin, noar, apid, ctid
STRING_ARG_HEAD = struct.Struct("<IH")          # type info, string length (little endian)
UINT32_ARG = struct.Struct("<II")               # type info, value (little endian)

HTYP = DLT_HTYP_UEH | DLT_HTYP_WEID | DLT_HTYP_WSID | DLT_HTYP_WTMS | DLT_HTYP_VERS1

# (ecu, apid, list of ctids, session id) combinations used round-robin so the
# generated file contains plenty of material for filter/sort testing.
SOURCES = [
    (b"ECU1", b"GIT\x00", [b"CMT\x00", b"AUTH", b"DATE"], 1001),
    (b"ECU1", b"LOG\x00", [b"MSG\x00", b"META"], 1002),
    (b"ECU1", b"SYS\x00", [b"JOUR", b"PROC"], 1003),
    (b"ECU2", b"APP1", [b"CTX1", b"CTX2", b"CTX3"], 2001),
    (b"ECU2", b"APP2", [b"MAIN", b"NET\x00"], 2002),
    (b"ECU2", b"TEST", [b"FLT\x00", b"SORT"], 2003),
]

LOG_LEVELS = [
    (DLT_LOG_INFO, 40),
    (DLT_LOG_DEBUG, 25),
    (DLT_LOG_VERBOSE, 15),
    (DLT_LOG_WARN, 12),
    (DLT_LOG_ERROR, 6),
    (DLT_LOG_FATAL, 2),
]

FALLBACK_LINES = [
    "commit 0000000000000000000000000000000000000000",
    "Author: Example Contributor <contributor@example.com>",
    "Date:   Thu Jan 1 00:00:00 1970 +0000",
    "    Initial commit",
    "    Add sample payload because no git history was available",
]


def parse_size(text):
    """Parse a human readable size like '1gb', '100MB', '512kb' or '4096'."""
    text = text.strip().lower()
    units = {"kb": 1024, "mb": 1024 ** 2, "gb": 1024 ** 3, "k": 1024,
             "m": 1024 ** 2, "g": 1024 ** 3, "b": 1}
    for suffix in sorted(units, key=len, reverse=True):
        if text.endswith(suffix):
            number = text[: -len(suffix)]
            break
    else:
        number, suffix = text, "b"
    try:
        value = float(number)
    except ValueError:
        raise argparse.ArgumentTypeError(f"invalid size: '{text}'")
    size = int(value * units[suffix])
    if size <= 0:
        raise argparse.ArgumentTypeError("size must be positive")
    return size


def git_log_entries(repo):
    """Return the git history as a list of (commit unix time, log lines).

    The history is returned oldest commit first and the lines mimic the
    regular ``git log`` output (commit/Author/Date header plus the indented
    commit message).
    """
    pretty = ("%x1e%at%ncommit %H%nAuthor: %an <%ae>%nDate:   %ad%n%n"
              "%w(0,4,4)%B")
    fallback = [(int(time.time()), list(FALLBACK_LINES))]
    try:
        out = subprocess.run(
            ["git", "-C", repo, "--no-pager", "log", "--reverse",
             "--date=iso", f"--pretty=format:{pretty}"],
            capture_output=True, check=True,
        ).stdout.decode("utf-8", errors="replace")
    except (OSError, subprocess.CalledProcessError) as exc:
        print(f"warning: could not read git log from '{repo}' ({exc}); "
              "using built-in sample lines", file=sys.stderr)
        return fallback
    entries = []
    for record in out.split("\x1e"):
        lines = record.splitlines()
        if not lines:
            continue
        try:
            commit_time = int(lines[0])
        except ValueError:
            continue
        text = [line for line in lines[1:] if line.strip()]
        if text:
            entries.append((commit_time, text))
    if not entries:
        print("warning: git log was empty; using built-in sample lines",
              file=sys.stderr)
        return fallback
    # git log order is topological, not strictly chronological: sort by
    # commit date so the generated trace time is monotonic
    entries.sort(key=lambda entry: entry[0])
    return entries


def pick_level(rng):
    roll = rng.randrange(100)
    acc = 0
    for level, weight in LOG_LEVELS:
        acc += weight
        if roll < acc:
            return level
    return DLT_LOG_INFO


def build_message(line, index, seconds, microseconds, timestamp, counters, rng):
    """Build one complete storage-format DLT message as bytes."""
    ecu, apid, ctids, seid = SOURCES[index % len(SOURCES)]
    ctid = ctids[(index // len(SOURCES)) % len(ctids)]

    payload_text = line.encode("utf-8") + b"\x00"
    payload = STRING_ARG_HEAD.pack(DLT_TYPE_INFO_STRG | DLT_SCOD_UTF8,
                                   len(payload_text)) + payload_text
    noar = 1
    # every 8th message carries an additional uint32 argument (the message
    # index) so files also contain multi-argument messages
    if index % 8 == 0:
        payload += UINT32_ARG.pack(DLT_TYPE_INFO_UINT | DLT_TYLE_32BIT, index & 0xFFFFFFFF)
        noar = 2

    key = (apid, ctid)
    mcnt = counters.get(key, 0)
    counters[key] = (mcnt + 1) & 0xFF

    msin = DLT_MSIN_VERB | (DLT_TYPE_LOG << 1) | (pick_level(rng) << 4)
    length = (STANDARD_HEADER.size + HEADER_EXTRA.size + EXTENDED_HEADER.size
              + len(payload))

    return b"".join((
        STORAGE_HEADER.pack(b"DLT\x01", seconds, microseconds, ecu),
        STANDARD_HEADER.pack(HTYP, mcnt, length),
        HEADER_EXTRA.pack(ecu, seid, timestamp),
        EXTENDED_HEADER.pack(msin, noar, apid, ctid),
        payload,
    ))


def generate(path, target_size, entries, seed):
    rng = random.Random(seed)
    counters = {}
    first_time = entries[0][0]
    written = 0
    index = 0
    cycle = 0
    next_report = target_size // 10 or target_size
    chunk = bytearray()

    with open(path, "wb") as f:
        clock_us = 0  # running wall clock in microseconds since the epoch
        while written + len(chunk) < target_size:
            for commit_time, lines in entries:
                if cycle == 0:
                    # follow the real commit date, but never step backwards,
                    # e.g. for commits in the same second
                    clock_us = max(commit_time * 1_000_000, clock_us)
                # on later cycles the history is replayed again and the clock
                # simply keeps running past the newest commit, so time stays
                # ascending without overflowing the 32 bit storage time
                for line in lines:
                    seconds = clock_us // 1_000_000
                    microseconds = clock_us % 1_000_000
                    # DLT timestamp: 0.1 ms since the first commit (a 32 bit
                    # field, so it wraps about every 5 days of history)
                    timestamp = ((seconds - first_time) * 10000
                                 + microseconds // 100) & 0xFFFFFFFF
                    chunk += build_message(line, index, seconds, microseconds,
                                           timestamp, counters, rng)
                    index += 1

                    # spread the lines of one commit by 0.1 .. 2 ms
                    clock_us += rng.randrange(100, 2000)

                    if len(chunk) >= 4 * 1024 * 1024:
                        f.write(chunk)
                        written += len(chunk)
                        chunk.clear()
                        if written >= next_report:
                            pct = min(100, written * 100 // target_size)
                            print(f"\r{pct:3d}% ({written // (1024 * 1024)} MiB, "
                                  f"{index} messages)", end="", flush=True)
                            next_report += target_size // 10 or target_size
                if written + len(chunk) >= target_size:
                    break
            cycle += 1
        if chunk:
            f.write(chunk)
            written += len(chunk)
    print(f"\rdone: {path} ({written} bytes, {index} messages)")
    return index, written


def main():
    parser = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--size", type=parse_size, default="10mb",
                        help="target file size, e.g. 512kb, 100mb, 1gb")
    parser.add_argument("-o", "--output", default="sample.dlt",
                        help="output .dlt file")
    parser.add_argument("--repo", default=".",
                        help="git repository whose log is used as payload")
    parser.add_argument("--seed", type=int, default=42,
                        help="random seed for log levels and sub-second timing")
    args = parser.parse_args()

    entries = git_log_entries(args.repo)
    total_lines = sum(len(lines) for _, lines in entries)
    print(f"using {total_lines} git log lines from {len(entries)} commits "
          "as payload source")
    generate(args.output, args.size, entries, args.seed)


if __name__ == "__main__":
    main()

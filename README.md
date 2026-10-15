# dlt-to-csv

A high-performance forensic CLI utility, analytical query engine, and Python library designed to parse automotive **DLT (Diagnostic Log and Trace)** binary log files and export them to strictly-quoted CSV or high-throughput indexed **SQLite** databases.

---

## Authorship, Copyright & Licensing

* **Author & Maintainer:** Michael Male <michael.male@met.police.uk>
* **Copyright:** © 2026 Crown Copyright (Metropolitan Police Service)
* **License:** [Mozilla Public License, v. 2.0 (MPL-2.0)](LICENSE.txt)
* **Upstream Attribution:** Derived from the [COVESA Project DLT Viewer](https://github.com/COVESA/dlt-viewer) ([COVESA / Connected Vehicle Systems Alliance](http://www.covesa.global/)).
* **AI Attribution:** Developed with the assistance of an AI coding assistant (Google Antigravity).

---

## Key Features

* **Zero External Dependencies:** Built entirely with standard library Python (Python 3.8+). Runs out-of-the-box on restricted or air-gapped forensic workstations.
* **Standalone Windows Executable (`dist/dlt-to-csv.exe`):** Pre-compiled standalone binary included in `dist/`. No Python runtime or installation required.
* **Full AUTOSAR DLT Protocol Fidelity:**
  * **DLT v1 & DLT v2 Support**: Complete header decoding (Storage Header, Standard Header, Extended Header).
  * **Verbose Argument Parsing**: Decodes strings (ASCII/UTF-8), signed/unsigned integers (8–128 bit), floats (IEEE 754 32/64-bit), booleans, raw bytes, and variable metadata (names and engineering units).
  * **Non-Verbose Messages**: Extracts 32-bit Message IDs (`MsgID`) and raw payload frames.
  * **Control Messages**: Service IDs, daemon communication, and network trace data.
* **Fault-Tolerant Stream Carving & Header Resync:**
  * Automotive crash logs and ECU memory dumps often contain corrupted, truncated, or interleaved byte streams.
  * The scanner automatically carves corrupted frames, synchronizes on valid pattern headers (`DLT\x01`, `DLT\x02`, `DLS\x01`), logs corruption offsets, and resumes parsing without dropping remainder data.
* **Forensic Evidentiary Chain-of-Custody:**
  * Automatic **SHA-256** and **MD5** cryptographic hashing for every processed DLT file.
  * Records **SourceFile**, exact **FileOffset**, **MsgLength**, **MessageCounter**, and high-precision storage timestamps (`time_utc`, `epoch_seconds`, `microseconds`).
  * Generates structured JSON forensic audit reports (`--report`).
* **High-Throughput SQLite Database Engine (2M+ Rows):**
  * Direct bulk ingestion into indexed SQLite databases (`.db` / `.sqlite`) at **100,000–250,000 messages/sec** via Write-Ahead Logging (`WAL`), in-memory cache tuning, and batch transactions.
  * ISO-8601 timestamps (`time_utc`) enable native SQLite date and time filtering (`date()`, `time()`, `strftime()`).
* **Multi-Core In-Place Hex-to-ASCII Payload Decoding:**
  * Space-separated ECU hex sequences (e.g. `48 65 6c 6c 6f 20 57 6f 72 6c 64` $\rightarrow$ `"Hello World"`) are automatically decoded into readable text in the indexed `payload_ascii` column.
  * Multiprocessing pool scales across **75–80% of available CPU cores**, accelerating decoding to **over 313,000 rows/second** (>4x CPU speedup).
  * **Single-writer architecture** guarantees zero database lock contention (`database is locked` is impossible).
* **Robust Comma Handling & RFC 4180 Quoting:**
  * All CSV fields are strictly enclosed in quotation marks (`"..."`) by default, preventing column corruption from payloads containing commas.
  * Optional `--replace-commas` sanitizes payload commas for legacy downstream tools.

---

## Standalone Executable Usage (Windows)

A portable, pre-compiled Windows executable is provided in [`dist/dlt-to-csv.exe`](dist/dlt-to-csv.exe). You can copy this file directly to a forensic USB drive and run it from CMD or PowerShell without installing Python.

```cmd
# Process an evidence folder into individual CSV files:
dist\dlt-to-csv.exe C:\Evidence\DLT_Logs\ -o C:\Analysis\CSV_Output\

# Build SQLite database and decode hex payloads in one command:
dist\dlt-to-csv.exe C:\Evidence\DLT_Logs\ -r --sqlite evidence.db --no-csv --update-ascii --cpu-percent 80 --batch-size 50000
```

To build or update the standalone executable yourself:
```cmd
build_dlt_to_csv_exe.bat
```

---

## Command-Line Reference

```text
usage: dlt-to-csv [-h] [-o OUTPUT] [-r] [-c] [--forensic]
                  [--sqlite DB_PATH [DB_PATH ...]] [--batch-size N] [-j N]
                  [--cpu-percent PCT] [--ram-cache-mb MB] [--no-csv]
                  [--query QUERY] [--query-dates]
                  [--query-export QUERY_EXPORT] [--update-ascii]
                  [--columns COLUMNS] [-d DELIMITER] [--quote-all]
                  [--no-quote-all] [--replace-commas [CHAR]]
                  [--timezone {utc,local}] [--tz-offset TZ_OFFSET] [--ecu ECU]
                  [--apid APID] [--ctid CTID] [--log-level LOG_LEVEL]
                  [--grep GREP] [--case-sensitive] [--start-time START_TIME]
                  [--end-time END_TIME] [--no-hash] [--report REPORT]
                  [--stdout] [-q] [-v]
                  [inputs ...]
```

### Primary Arguments

| Argument / Flag | Description |
| :--- | :--- |
| `inputs` | One or more folders containing `.dlt` files, or individual `.dlt` file paths. |
| `-o, --output <path>` | Destination directory for individual CSVs, or file path if using `--combined`. |
| `-r, --recursive` | Recursively scan subdirectories for `.dlt` files. |
| `-c, --combined` | Merge messages from all discovered DLT files into a single master CSV. |
| `--forensic` | Include evidentiary columns (`SourceFile`, `FileOffset`, `MsgLength`, `MsgID`, `EpochSeconds`, `Microseconds`). |

### SQLite Database & Query Engine (Large Datasets, 2M+ Rows)

| Flag | Description |
| :--- | :--- |
| `--sqlite, --db <path...>` | Ingest messages into SQLite, or target SQLite database(s) / wildcards (`*.db`) for queries/updates. |
| `--batch-size <N>` | Transaction batch chunk size for SQLite inserts and updates (default: `20000`). |
| `-j, --workers <N>` | Number of parallel worker processes for hex decoding (default: 75% of CPU cores; `1` for single-process). |
| `--cpu-percent <PCT>` | Percentage of system CPU cores to allocate to worker processes (e.g. `80` for 80%, `50` for 50%). |
| `--ram-cache-mb <MB>` | SQLite RAM page cache size in MB (default: `128` MB). |
| `--no-csv` | Suppress CSV generation when exporting to SQLite. |
| `--update-ascii, --update-db`| Perform an in-place batch transaction on the SQLite DB to decode all hex payloads into the indexed `payload_ascii` column. |
| `--query "<SQL>"` | Execute an arbitrary SQL query against the SQLite database and display results in the terminal. |
| `--query-dates` | Display summary table of all log dates, message tallies, active ECUs, and error breakdown. |
| `--query-export <out.csv>` | Stream results of `--query` directly into a CSV file. |

### Forensic Filtering Options

| Flag | Description |
| :--- | :--- |
| `--ecu <ECU1,ECU2>` | Filter by ECU ID(s), comma-separated (e.g. `ENGN,BCM,HEADUNIT`). |
| `--apid <APID1,APID2>` | Filter by Application ID(s), comma-separated (e.g. `LOGD,NAVI,AUDI`). |
| `--ctid <CTID1,CTID2>` | Filter by Context ID(s), comma-separated (e.g. `MAIN,INIT,CONN`). |
| `--log-level <levels>` | Filter by log subtype(s), comma-separated (e.g. `fatal,error,warn`). |
| `--grep <regex>` | Regular expression pattern to search within message payloads. |
| `--case-sensitive` | Perform case-sensitive matching for `--grep`. |
| `--start-time <time>` | Filter messages on/after timestamp (ISO-8601 `'YYYY-MM-DD HH:MM:SS'` or UNIX epoch seconds). |
| `--end-time <time>` | Filter messages on/before timestamp (ISO-8601 `'YYYY-MM-DD HH:MM:SS'` or UNIX epoch seconds). |

### CSV Formatting & Audit Options

| Flag | Description |
| :--- | :--- |
| `--replace-commas [CHAR]` | Replace commas inside payload strings with `CHAR` (default: `;`) to prevent naive split errors. |
| `--quote-all` | Enclose all fields in quotes (RFC 4180 strict quoting, enabled by default). |
| `--no-quote-all` | Disable strict quoting; only quote fields containing delimiters. |
| `--columns <list>` | Custom comma-separated list of column headers to export. |
| `--timezone {utc,local}` | Timestamp formatting mode: `utc` (default, recommended for forensics) or `local`. |
| `--tz-offset <hours>` | Optional timezone offset in hours to apply to UTC timestamp (e.g. `1` or `-5`). |
| `--report <path.json>` | Generate a structured JSON forensic audit manifest with cryptographic hashes and metadata. |
| `--no-hash` | Disable SHA-256 and MD5 computation to maximize raw ingestion speed. |
| `-q, --quiet` | Suppress progress output on `stderr`. |

---

## Forensic Workflow Examples

### 1. Ingest DLT Logs into SQLite & Decode ASCII in One Command

Process an entire directory of DLT files recursively, build `evidence.db`, and decode all hex payloads across 80% of CPU cores:

```cmd
dist\dlt-to-csv.exe "C:\Evidence\Vehicle_Logs\" -r --sqlite "C:\Analysis\evidence.db" --no-csv --update-ascii --cpu-percent 80 --batch-size 50000
```

### 2. Multi-Database Wildcard Update

Run multi-core in-place ASCII payload decoding across all databases matching a wildcard pattern:

```cmd
dist\dlt-to-csv.exe --sqlite *.db --update-ascii --cpu-percent 80 --batch-size 50000
```

### 3. Summary of Messages Grouped by Date

Instantly inspect the log date breakdown across a multi-million row database:

```cmd
dist\dlt-to-csv.exe --sqlite evidence.db --query-dates
```

**Output:**
```text
====================================================================================================================
DLT Dataset Summary by Date: evidence.db
====================================================================================================================
Log Date     |   Total Msgs |   ECUs |   Fatal |   Error |    Warn | First Timestamp (UTC)      | Last Timestamp (UTC)      
--------------------------------------------------------------------------------------------------------------------
2026-10-12   |      842,109 |     14 |       0 |      42 |     180 | 2026-10-12 00:00:01.120401 | 2026-10-12 23:59:58.981042
2026-10-13   |    1,210,480 |     14 |       2 |     118 |     342 | 2026-10-13 00:00:00.040192 | 2026-10-13 23:59:59.510882
====================================================================================================================
```

### 4. Date-Bounded SQL Query Exported to CSV

Query critical messages during an incident window and stream results directly into a CSV spreadsheet:

```cmd
dist\dlt-to-csv.exe --sqlite evidence.db \
  --query "SELECT time_utc, ecu_id, apid, subtype, payload_ascii FROM messages WHERE time_utc BETWEEN '2026-10-13 14:00:00' AND '2026-10-13 15:30:00' AND subtype IN ('error', 'fatal')" \
  --query-export C:\Analysis\incident_faults.csv
```

### 5. Generate Master Combined Forensic CSV with Hashes

```cmd
dist\dlt-to-csv.exe C:\Evidence\DLT_Folder\ -r -c --forensic -o C:\Analysis\master_forensic.csv --report C:\Analysis\audit_report.json
```

---

## Multi-Core Hardware Acceleration Architecture

```
                       ┌──> Worker Process 1 (Core 1) ──> RAM Decoded Tuples ──┐
[ SQLite Input Batch ] ┼──> Worker Process 2 (Core 2) ──> RAM Decoded Tuples ──┼──> [ Main Process ] ──> Commit to SQLite
 (e.g. 50,000 rows)    ├──> Worker Process 3 (Core 3) ──> RAM Decoded Tuples ──┤    (Single Writer)      (No Locks)
                       └──> Worker Process N (Core N) ──> RAM Decoded Tuples ──┘
```

1. **Bypassing Python's GIL:** Python's Global Interpreter Lock restricts standard threads to ~7% CPU usage on a 14-core machine. By using `ProcessPoolExecutor`, worker processes run on dedicated CPU cores with full hardware utilization.
2. **Zero Database Locks:** Worker processes perform pure in-memory string decoding and **never** connect to SQLite. Only the single main process writes to the database, ensuring `database is locked` errors are physically impossible.
3. **Deferred Index Construction:** The engine drops `idx_messages_payload_ascii` during bulk updates and rebuilds it in a single sequential pass at the end, eliminating random I/O and B-tree rebalance latency.

---

## Running from Python Source

If you prefer using the Python scripts directly:

```bash
# Display help:
python dlt_to_csv.py --help

# Run the complete test suite:
python -m unittest tests/test_dlt_to_csv.py

# Query payload ASCII helper script:
python query_payload_ascii.py evidence.db --date 2026-10-13 --log-level error,fatal
```

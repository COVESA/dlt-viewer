# DLT to CSV Forensic CLI Tool

A forensic-grade Python CLI application and library designed to parse automotive **DLT (Diagnostic Log and Trace)** binary files and convert them into CSV format.

Based on the [COVESA/dlt-viewer](https://github.com/COVESA/dlt-viewer) architecture and the AUTOSAR DLT protocol specification, this tool operates with **zero external dependencies** (pure standard library Python 3.8+), making it ideal for restricted, air-gapped digital forensic workstations.

---

## Key Features for Forensic Practitioners

* **Folder & Batch Ingestion:** Point the program at a folder containing 1 or more `.dlt` files, multiple individual files, or perform recursive subfolder searches (`-r` / `--recursive`).
* **Forensic Integrity & Traceability:**
  * Computes **SHA-256** and **MD5** cryptographic hashes for every input `.dlt` evidence file.
  * Records **SourceFile**, **FileOffset** (byte offset in file), **MsgLength**, and **MessageCounter** (0–255) to detect dropped or tampered packets.
  * Generates an automated JSON forensic audit manifest (`--report`) logging file hashes, message statistics, corruption metrics, and time bounds.
* **Fault-Tolerant Stream Carving & Resync:**
  * Automotive crash logs and ECU memory dumps often contain corrupted or partial frames.
  * When bad bytes or corrupted framing occur, the scanner automatically scans ahead for synchronization headers (`DLT\x01`, `DLT\x02`, `DLS\x01`), logs the corruption offset and skipped bytes, and resumes parsing without dropping the remainder of the file.
* **Full Protocol Fidelity (DLT v1 & DLT v2):**
  * Storage Header v1 and v2 (timestamps, microsecond/nanosecond resolution, ECU IDs).
  * Standard Header with all flags (`UEH`, `MSBF`, `WEID`, `WSID`, `WTMS`).
  * Extended Header (`APID`, `CTID`, Session ID, Message Type, Subtype).
  * Verbose argument decoding for ASCII/UTF-8 strings, signed/unsigned integers (8–128 bit), floats (IEEE 754 float/double), booleans, raw bytes, and variable metadata (names and engineering units).
  * Non-verbose message decoding with 32-bit Message ID (`MsgID`) extraction.
  * Control message decoding (`service_id`, service name, return code, network/software info).
* **Multi-Criteria Filtering:**
  * Filter by ECU ID (`--ecu`)
  * Filter by Application ID (`--apid`)
  * Filter by Context ID (`--ctid`)
  * Filter by Log Level (`--log-level` e.g. `fatal,error,warn`)
  * Filter by Payload regex / keyword search (`--grep`)
  * Filter by Time Range (`--start-time`, `--end-time`)
* **Flexible Output Modes:**
  * One CSV per input DLT file (default).
  * Merged/combined CSV across all logs (`-c` / `--combined`).
  * Direct stdout streaming (`--stdout`) for terminal pipelines.

## Standalone Executable for Air-Gapped Environments

For forensic practitioners working on air-gapped forensic workstations (where Python cannot be installed or network access is restricted), the tool is available as a single, portable, self-contained Windows executable:

* **Executable Location:** `dist/dlt-to-csv.exe` (or build using `build_dlt_to_csv_exe.bat`)
* **Requirements:** None. No Python runtime, no DLL installations, and zero external dependencies.

You can simply copy `dlt-to-csv.exe` to a forensic USB drive and execute it directly on any Windows workstation:
```cmd
dlt-to-csv.exe D:\Vehicle_Evidence\ -o D:\Analysis_CSV\
```

---

## Quick Start

### 1. Basic Folder Ingestion (1..* DLT Files)
Process an entire folder of `.dlt` files into individual CSVs:
```bash
# Using Python:
python dlt_to_csv.py /path/to/dlt_folder/ -o /path/to/csv_output/

# Using Standalone Executable:
dlt-to-csv.exe C:\Evidence\DLT_Logs\ -o C:\Analysis\CSV_Output\
```

### 2. Combined CSV with Forensic Traceability
Merge all discovered DLT files into a single master forensic CSV, including exact byte offsets, lengths, and file provenance:
```bash
python dlt_to_csv.py /evidence/vehicle_logs/ -r -c --forensic -o combined_evidence.csv
```

### 3. Generate a Forensic Audit Report
Generate both the CSV export and an evidentiary JSON audit manifest containing SHA-256 hashes, byte counts, and message tallies:
```bash
python dlt_to_csv.py /evidence/vehicle_logs/ -o /analysis/ --report /analysis/audit_manifest.json
```

### 4. Filter for Critical Faults & Errors
Extract only `error` and `fatal` log entries for specific ECUs (e.g. Engine and BCM):
```bash
python dlt_to_csv.py /evidence/vehicle_logs/ --ecu ENGN,BCM --log-level error,fatal -c -o critical_faults.csv
```

### 5. Search for Specific Keywords (Regex)
Search for diagnostic trouble codes (DTCs), tamper events, or sensor faults:
```bash
python dlt_to_csv.py /evidence/vehicle_logs/ --grep "DTC|tamper|collision|airbag" -c -o incident_events.csv
```

### 6. Ingest Large Datasets (2M+ Rows) into Indexed SQLite Database
For massive datasets where you want to import dozens/hundreds of DLT files without creating millions of raw CSV rows:
```bash
# Using Python:
python dlt_to_csv.py /evidence/large_dlt_dataset/ -r --sqlite vehicle_telemetry.db

# Using Standalone Executable:
dlt-to-csv.exe C:\Evidence\Large_Logs\ -r --sqlite C:\Analysis\vehicle_telemetry.db
```

### 7. Inspect Date Breakdown Across Imported DLTs
Instantly view daily message counts, error rates, and time bounds:
```bash
dlt-to-csv.exe --sqlite vehicle_telemetry.db --query-dates
```

### 8. Run Date-Bounded Queries & Export to CSV
Execute SQL queries with date/time filters and export the slice directly to CSV:
```bash
dlt-to-csv.exe --sqlite vehicle_telemetry.db \
  --query "SELECT time_utc, ecu_id, apid, subtype, payload FROM messages WHERE time_utc BETWEEN '2026-10-14 08:00:00' AND '2026-10-14 18:00:00' AND subtype IN ('error', 'fatal')" \
  --query-export /analysis/oct14_faults.csv
```

---

## Command-Line Reference

```text
usage: dlt-to-csv [-h] [-o OUTPUT] [-r] [-c] [--forensic] [--sqlite SQLITE]
                  [--sqlite-batch-size SQLITE_BATCH_SIZE] [--no-csv]
                  [--query QUERY] [--query-dates]
                  [--query-export QUERY_EXPORT] [--columns COLUMNS]
                  [-d DELIMITER] [--quote-all] [--no-quote-all] [--quote-minimal]
                  [--replace-commas [CHAR]] [--timezone {utc,local}]
                  [--tz-offset TZ_OFFSET] [--ecu ECU] [--apid APID]
                  [--ctid CTID] [--log-level LOG_LEVEL] [--grep GREP]
                  [--case-sensitive] [--start-time START_TIME]
                  [--end-time END_TIME] [--no-hash] [--report REPORT]
                  [--stdout] [-q] [-v]
                  [inputs ...]
```

### Positional Arguments
| Argument | Description |
| :--- | :--- |
| `inputs` | One or more folders containing `.dlt` files, or individual `.dlt` file paths (optional when querying SQLite). |

### Output & CSV Options
| Option | Description |
| :--- | :--- |
| `-o, --output` | Destination output directory (for individual CSVs) or target CSV path (if `--combined`). |
| `-r, --recursive` | Recursively discover `.dlt` files in subdirectories. |
| `-c, --combined` | Merge messages from all DLT files into a single combined CSV. |
| `--forensic` | Enable forensic columns (`SourceFile`, `FileOffset`, `MsgLength`, `MsgID`, `EpochSeconds`, `Microseconds`). |
| `--columns` | Custom comma-separated list of columns to export (e.g. `Time,Ecuid,Apid,Payload`). |
| `-d, --delimiter` | Field delimiter (default: `,`). |
| `--quote-all` | Enclose all fields in quotes (RFC 4180 strict quoting, **enabled by default** in v2). |
| `--no-quote-all`, `--quote-minimal` | Disable strict quoting and only quote fields containing special characters. |
| `--replace-commas [CHAR]` | Replace commas inside the `Payload` column with `CHAR` (default: `;`) to prevent naive split errors in downstream tools. |
| `--timezone` | `utc` (default, recommended for forensics) or `local`. |
| `--tz-offset` | Hours offset added to UTC timestamp (e.g. `1` or `-5`). |
| `--report` | Path to save cryptographic JSON audit summary. |
| `--no-hash` | Disable SHA-256 / MD5 hash calculation for maximum ingestion speed. |
| `--stdout` | Stream CSV output to `stdout` for UNIX piping. |
| `-q, --quiet` | Suppress progress messages on `stderr`. |
| `-v, --version` | Display version and exit. |

### SQLite Database & Analytical Query Options (2M+ Rows)
| Option | Description |
| :--- | :--- |
| `--sqlite, --db <path>` | Ingest parsed DLT messages into an indexed SQLite database (`.db` / `.sqlite`). |
| `--sqlite-batch-size <N>` | Number of rows per SQLite transaction during bulk loading and in-place updates (default: `20000`). |
| `--no-csv` | Suppress CSV generation when exporting to SQLite. |
| `--query "<SQL>"` | Execute an SQL query against the SQLite database and display results in terminal. |
| `--query-dates` | Query and display a summary table of all dates, message counts, active ECUs, and error breakdown. |
| `--query-export <path.csv>` | Stream results of `--query` directly into a target CSV file. |
| `--update-ascii, --update-db` | Perform an in-place batch update transaction on the SQLite DB to decode hex payloads into the indexed `payload_ascii` column. |

---

## SQLite Database Export & Date-Based Queries (2M+ Rows)

When analyzing fleets or vehicle tests with millions of messages across dozens of DLT files, CSV spreadsheets can become sluggish or exceed Excel's 1,048,576 row ceiling.

`dlt_to_csv` includes a dedicated **high-throughput SQLite engine** optimized for bulk ingestion of 2,000,000+ messages:

* **Ingestion Throughput:** Achieves 100,000–250,000 messages/sec using SQLite Write-Ahead Logging (`WAL`), 64MB memory caching, 20,000-row batch transactions, and deferred B-tree index construction.
* **ISO-8601 Timestamps (`time_utc`):** Timestamps are stored in standard `YYYY-MM-DD HH:MM:SS.ffffff` format, enabling native SQLite date and time functions (`date()`, `time()`, `datetime()`, `strftime()`).
* **B-Tree Indexes:** Automatic indexing on `time_utc`, `epoch_seconds`, `ecu_id`, `apid`, `ctid`, and `subtype`.

### Database Schema

#### `dlt_files` (Audit & Chain-of-Custody Table)
| Column | Type | Description |
| :--- | :--- | :--- |
| `id` | INTEGER PRIMARY KEY | Unique file identifier. |
| `file_path` | TEXT | Absolute file path. |
| `file_name` | TEXT | Base filename. |
| `file_size_bytes` | INTEGER | File size in bytes. |
| `sha256`, `md5` | TEXT | Cryptographic hashes of evidence file. |
| `valid_messages` | INTEGER | Count of successfully parsed messages. |
| `corrupt_messages`| INTEGER | Count of corrupted frames carved. |
| `first_timestamp_utc`, `last_timestamp_utc` | TEXT | Time bounds of messages in file. |
| `unique_ecus`, `unique_apids` | TEXT | Distinct ECUs and Application IDs discovered. |

#### `messages` (Telemetry & Log Records Table)
| Column | Type | Description |
| :--- | :--- | :--- |
| `id` | INTEGER PRIMARY KEY | Autoincrement row ID. |
| `file_id` | INTEGER | Foreign key to `dlt_files(id)`. |
| `source_file` | TEXT | Source DLT filename. |
| `file_offset` | INTEGER | Byte offset in source file. |
| `msg_length` | INTEGER | Packet length in bytes. |
| `msg_index` | INTEGER | Sequential counter within file. |
| `time_utc` | TEXT | **ISO-8601:** `YYYY-MM-DD HH:MM:SS.ffffff` (indexed). |
| `time_dlt` | TEXT | DLT Viewer format: `YYYY/MM/DD HH:MM:SS.uuuuuu`. |
| `epoch_seconds`| INTEGER | UNIX epoch seconds (indexed). |
| `microseconds` | INTEGER | Microsecond fraction (0–999999). |
| `ecu_timestamp_s`| REAL | ECU uptime timer in seconds. |
| `counter` | INTEGER | Message counter (0–255). |
| `ecu_id` | TEXT | ECU identifier (e.g. `ENGN`, `BCM1`, `GATEWAY`). |
| `apid`, `ctid` | TEXT | Application ID and Context ID. |
| `session_id` | INTEGER | Session identifier. |
| `type`, `subtype`| TEXT | Message type and subtype (`info`, `warn`, `error`, `fatal`). |
| `mode` | TEXT | `verbose` or `non-verbose`. |
| `payload` | TEXT | Raw payload string as captured in DLT. |
| `payload_ascii` | TEXT | Decoded ASCII / UTF-8 string with hex bytes automatically converted (indexed). |
| `msg_id` | TEXT | Message ID for non-verbose payloads. |

### Pre-Built Analytical Views

1. **`v_daily_summary`**: Aggregates daily message volume, error counts, and active time spans grouped by date (`YYYY-MM-DD`) and ECU.
2. **`v_critical_faults`**: Instant view filtering all `fatal` and `error` messages across all imported files.

### Example SQL Date Queries

```sql
-- 1. Count messages per day across the entire dataset:
SELECT date(time_utc) AS log_date, COUNT(*) AS total_msgs
FROM messages
GROUP BY log_date
ORDER BY log_date ASC;

-- 2. Query messages between specific dates and times (utilizes idx_messages_time_utc):
SELECT time_utc, ecu_id, apid, subtype, payload
FROM messages
WHERE time_utc >= '2026-10-14 08:00:00' AND time_utc < '2026-10-14 17:30:00'
ORDER BY time_utc ASC;

-- 3. High-performance index range scan using epoch seconds:
SELECT * FROM messages
WHERE epoch_seconds BETWEEN 1776000000 AND 1776050000;

-- 4. Hourly error frequency distribution:
SELECT
    strftime('%Y-%m-%d %H:00', time_utc) AS hour_bucket,
    ecu_id,
    COUNT(*) AS error_count
FROM messages
WHERE subtype IN ('error', 'fatal')
GROUP BY hour_bucket, ecu_id
ORDER BY hour_bucket ASC;

-- 5. Search payload text on a specific date:
SELECT time_utc, ecu_id, payload_ascii
FROM messages
WHERE date(time_utc) = '2026-10-14' AND payload_ascii LIKE '%DTC%'
ORDER BY time_utc ASC;
```

---

## In-Place Hex-to-ASCII Payload Conversion (`--update-ascii`)

In many automotive ECUs, strings and diagnostics are logged as space-separated hex byte sequences (e.g. `48 65 6c 6c 6f 20 57 6f 72 6c 64` for `"Hello World"`).

The standalone `dlt-to-csv.exe` executable includes an in-place batch update engine that converts these hex payloads directly into clean ASCII / UTF-8 text and stores them in the indexed `payload_ascii` column.

### In-Place Database Update Command

Run the update transaction directly on any existing SQLite database:

```cmd
dist\dlt-to-csv.exe --sqlite evidence.db --update-ascii
```

* **Batch Transaction Safety:** Updates records in chunked transactions (default: 20,000 rows per transaction, configurable via `--sqlite-batch-size`) to ensure optimal performance and minimal memory usage on 2M+ row databases.
* **Schema Migration:** Automatically adds the `payload_ascii` column via `ALTER TABLE` if missing from an older database.
* **Automatic Indexing:** Builds the `idx_messages_payload_ascii` index and checkpoints WAL upon completion.
* **Built-in SQL Function:** Registers `hex_to_ascii(payload)` so interactive queries can convert raw strings on the fly:
  ```cmd
  dist\dlt-to-csv.exe --sqlite evidence.db --query "SELECT time_utc, ecu_id, hex_to_ascii(payload) FROM messages LIMIT 20"
  ```

---

## Handling Columns with Commas (v2)

DLT log messages frequently contain commas in the `Payload` column (e.g. `"Speed: 100, Temp: 90, Status: OK"`). To prevent these from corrupting column alignment:
1. **Strict RFC 4180 Quoting (Default):** Every field is strictly enclosed in quotation marks (`"..."`). Compliant CSV readers (Excel, pandas, databases) treat commas inside quotes as content, preserving column boundaries.
2. **Payload Comma Sanitization (`--replace-commas`):** For tools or scripts that perform naive comma splitting (without quote parsing), pass `--replace-commas` to automatically convert commas in the payload to semicolons `;` (or a custom character like space or pipe):
   ```bash
   python dlt_to_csv.py /evidence/logs/ --replace-commas -o /analysis/
   # Or with custom character:
   python dlt_to_csv.py /evidence/logs/ --replace-commas " " -o /analysis/
   ```

---

## CSV Column Specifications

### Standard Columns (`DEFAULT_COLUMNS`)
Matches standard COVESA/dlt-viewer export format:

| Column | Type | Example | Description |
| :--- | :--- | :--- | :--- |
| **Index** | Integer | `0` | Sequential message counter within file/stream. |
| **Time** | String | `2026/04/12 14:22:01.500123` | Wall-clock time formatted as `YYYY/MM/DD HH:MM:SS.uuuuuu`. |
| **Timestamp** | String | `12.3456` | ECU uptime timer (0.1 ms resolution, `tmsp / 10000`). |
| **Count** | Integer | `42` | DLT message counter (`0..255`), increases per context. |
| **Ecuid** | String | `ECU1` | Electronic Control Unit Identifier (4 chars). |
| **Apid** | String | `LOGD` | Application Identifier (4 chars). |
| **Ctid** | String | `RBUF` | Context Identifier (4 chars). |
| **SessionId** | Integer | `0` | Session identifier number. |
| **Type** | String | `log` | Message type (`log`, `app_trace`, `nw_trace`, `control`). |
| **Subtype** | String | `info` | Message subtype (`fatal`, `error`, `warn`, `info`, `debug`, `verbose`). |
| **Mode** | String | `verbose` | `verbose` or `non-verbose`. |
| **#Args** | Integer | `2` | Number of arguments in the payload. |
| **Payload** | String | `Start logging after ECU startup` | Decoded text payload with whitespace normalized. |

### Forensic Provenance Columns (`--forensic`)
Appends the following fields for evidentiary chain-of-custody:

| Column | Type | Example | Description |
| :--- | :--- | :--- | :--- |
| **SourceFile** | String | `drive_log_001.dlt` | Base filename of the source DLT container. |
| **FileOffset** | Integer | `16384` | Exact byte offset of the message in the source file. |
| **MsgLength** | Integer | `71` | Total length of the message in bytes. |
| **MsgID** | String | `0x00001020` | Message ID for non-verbose payloads. |
| **EpochSeconds**| Integer | `1776003721` | Raw UNIX epoch seconds from storage header. |
| **Microseconds**| Integer | `500123` | Raw microsecond offset from storage header. |

---

## Forensic Audit Report Schema (`--report`)

When `--report <path.json>` is specified, the tool outputs an audit report structured as follows:

```json
{
  "tool": "DLT-to-CSV Forensic Converter",
  "version": "2.0.0",
  "execution_start_utc": "2026-10-14 13:30:00Z",
  "execution_end_utc": "2026-10-14 13:30:02Z",
  "duration_seconds": 2.145,
  "total_files_processed": 10,
  "total_messages_parsed": 154230,
  "total_messages_exported": 154230,
  "output_csv_files": [
    "/analysis/combined_evidence.csv"
  ],
  "file_details": [
    {
      "file_path": "/evidence/log_01.dlt",
      "file_size_bytes": 1048576,
      "sha256": "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
      "md5": "d41d8cd98f00b204e9800998ecf8427e",
      "total_messages": 15423,
      "valid_messages": 15423,
      "corrupt_messages": 0,
      "skipped_bytes": 0,
      "first_timestamp_utc": "2026-10-14 10:00:00Z",
      "last_timestamp_utc": "2026-10-14 10:15:30Z",
      "unique_ecus": ["BODY", "ENGN", "GATEWAY"],
      "unique_apids": ["DIAG", "LOGD", "SYS"],
      "unique_ctids": ["MAIN", "RBUF"],
      "elapsed_seconds": 0.21,
      "exported_messages": 15423
    }
  ]
}
```

---

## Hex-to-ASCII Payload Decoding Script (`query_payload_ascii.py`)

In many automotive ECUs, raw byte dumps or telemetry are logged into the `Payload` column as space-separated hex byte strings (e.g. `48 65 6c 6c 6f 20 57 6f 72 6c 64`).

The dedicated [`query_payload_ascii.py`](file:///c:/Users/FS02260Admin/Documents/DLT_Viewer_Michael_Version/dlt-viewer/query_payload_ascii.py) script queries the SQLite database, converts hex payloads to readable ASCII/UTF-8 text, and provides flexible viewing, CSV export, or in-place database enrichment:

### 1. Query Messages by Date & Print Decoded ASCII
```bash
python query_payload_ascii.py vehicle_telemetry.db --date 2026-10-14
```

### 2. Export Date-Filtered Decoded Messages to CSV
```bash
python query_payload_ascii.py vehicle_telemetry.db \
  --from-date "2026-10-14 08:00:00" --to-date "2026-10-14 18:00:00" \
  --ecu ENGN --log-level error,fatal \
  -o oct14_decoded_errors.csv
```

### 3. Permanently Enrich SQLite Database (`--update-db`)
For massive 2M+ row databases, you can permanently add and populate an indexed `payload_ascii` column in high-speed batches (50,000+ rows/sec):
```bash
python query_payload_ascii.py vehicle_telemetry.db --update-db
```
Once updated, standard SQL queries can directly search decoded text across all dates:
```sql
SELECT time_utc, ecu_id, payload_ascii
FROM messages
WHERE payload_ascii LIKE '%DTC%' AND date(time_utc) = '2026-10-14';
```

---

## Running the Automated Test Suite

Run the unit and integration tests from the repository root:
```bash
python -m unittest tests/test_dlt_to_csv.py
```
All 26 tests validate binary parsing, resync behavior, filtering, CLI functionality, SQLite database ingestion, date querying, and hex-to-ASCII payload decoding.

---

## License

This tool is part of the COVESA Project Dlt Viewer and is licensed under the
[Mozilla Public License, v. 2.0 (MPL-2.0)](http://mozilla.org/MPL/2.0/).
See [LICENSE.txt](../LICENSE.txt) and [MPL.txt](../MPL.txt) for the full license text.


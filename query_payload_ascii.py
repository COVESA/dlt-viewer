#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Author: Michael Male <michael.male@met.police.uk>
# Copyright (c) 2026 Crown Copyright (Metropolitan Police Service)
#
# Produced using an AI coding assistant (Google Antigravity).
#
# This file is part of dlt-to-csv, derived from COVESA Project Dlt Viewer
# (https://github.com/COVESA/dlt-viewer).
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.global/.

"""Query an SQLite DLT database and apply hex-to-ASCII payload decoding.

Converts space-separated hex byte strings (e.g. '48 65 6c 6c 6f') in the
DLT Payload column into readable ASCII / UTF-8 text.

Supports:
  * Filtering by dates and timestamps (e.g. --date 2026-10-14, --from-date, --to-date).
  * Filtering by ECU, APID, CTID, and Log Level.
  * Streaming output to terminal table or CSV file.
  * In-place database update (--update-db) to store decoded ASCII in a new
    indexed 'payload_ascii' column for massive 2M+ row datasets.
"""

import argparse
import csv
import re
import sqlite3
import sys
import unicodedata
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional, Tuple, Union


def sanitize_ascii_payload(text: str) -> str:
    """Sanitize string by removing escape sequences, control characters, and null bytes.

    Ensures null bytes (which cause SQLite tools to display the column as BLOB)
    and non-printable C0/C1 control codes are removed and normalized to spaces,
    leaving clean, human-readable ASCII/UTF-8 text.
    """
    if not text:
        return ""

    # Strip ANSI escape codes (e.g. \x1b[31m)
    text = re.sub(r"\x1b\[[0-9;]*[a-zA-Z]", " ", text)
    # Strip literal backslash-escapes for null and control codes if present as text
    text = re.sub(r"\\x00|\\0|\\x[0-1][0-9a-fA-F]", " ", text)

    cleaned_chars: List[str] = []
    for ch in text:
        code = ord(ch)
        # NUL byte (crucial: prevents SQLite from marking the column as BLOB)
        if code == 0:
            cleaned_chars.append(" ")
            continue
        # Standard whitespace (tab, newline, carriage return)
        if code in (9, 10, 13):
            cleaned_chars.append(" ")
            continue
        # C0 control characters (0x01-0x1F) and DEL (0x7F)
        if code < 32 or code == 127:
            cleaned_chars.append(" ")
            continue
        # C1 control characters (0x80-0x9F, e.g. \x9d)
        if 128 <= code <= 159:
            cleaned_chars.append(" ")
            continue
        # Unicode control (Cc), surrogate (Cs), format (Cf) characters
        if unicodedata.category(ch) in ("Cc", "Cs", "Cf"):
            cleaned_chars.append(" ")
            continue
        # Unicode replacement character
        if ch == "\ufffd":
            cleaned_chars.append(" ")
            continue
        cleaned_chars.append(ch)

    return " ".join("".join(cleaned_chars).split())


def _decode_bytes_safe(data: bytes) -> str:
    """Safely decode raw bytes trying UTF-8 first, then Latin-1."""
    if not data:
        return ""
    try:
        return data.decode("utf-8")
    except UnicodeDecodeError:
        return data.decode("latin-1", errors="replace")


def decode_hex_payload(val: Any) -> str:
    """Convert hexadecimal payload data into readable ASCII text and sanitize.

    Applies a 3-step pipeline:
      1. Checks if a cell in 'payload' contains hexadecimal characters.
      2. If so, converts these hexadecimal characters to ASCII / UTF-8 text,
         handling spaces, colons, hyphens, and mixed/typo tokens (e.g. 'v2') gracefully.
      3. Sanity checks by removing null bytes, escape sequences, and control
         characters so SQLite and CSV viewers never display the column as a BLOB.
    """
    if val is None:
        return ""
    if not isinstance(val, str):
        val = str(val)
    val = val.strip()
    if not val:
        return ""

    # Handle literal backslash-x escapes if present in raw string (e.g. '\x48\x65')
    if "\\x" in val:
        val = re.sub(r"\\x([0-9a-fA-F]{2})", lambda m: chr(int(m.group(1), 16)), val)

    # 1. Checks if cell contains hexadecimal characters
    normalized = re.sub(r"[:\-]", " ", val) if (":" in val or "-" in val) and (" " not in val) else val
    tokens = normalized.split()
    hex_pattern = re.compile(r"^(?:0x)?[0-9a-fA-F]{2}$")
    hex_tokens = [t for t in tokens if hex_pattern.match(t)]

    # Case A: Space-separated tokens predominantly containing 2-char hex bytes
    if len(tokens) > 1 and len(hex_tokens) >= 2 and (
        (len(hex_tokens) / len(tokens) >= 0.5) or len(hex_tokens) >= 3
    ):
        chunks: List[str] = []
        curr_bytes = bytearray()
        for t in tokens:
            if hex_pattern.match(t):
                digits = t[2:] if t.startswith(("0x", "0X")) else t
                curr_bytes.append(int(digits, 16))
            else:
                if curr_bytes:
                    chunks.append(_decode_bytes_safe(bytes(curr_bytes)))
                    curr_bytes.clear()
                chunks.append(t)
        if curr_bytes:
            chunks.append(_decode_bytes_safe(bytes(curr_bytes)))
        decoded_text = " ".join(chunks)
        return sanitize_ascii_payload(decoded_text)

    # Case B: Continuous unspaced hex (e.g. '48656c6c6f20576f726c64' or '0x1140c2')
    has_0x = val.startswith(("0x", "0X"))
    unspaced = val[2:] if has_0x else val
    if (has_0x and len(unspaced) >= 2 and len(unspaced) % 2 == 0) or (len(unspaced) >= 4 and len(unspaced) % 2 == 0):
        if all(c in "0123456789abcdefABCDEF" for c in unspaced):
            try:
                raw_bytes = bytes.fromhex(unspaced)
                decoded_text = _decode_bytes_safe(raw_bytes)
                return sanitize_ascii_payload(decoded_text)
            except Exception:
                pass

    # Case C: Plain text -> sanitize control characters and null bytes
    return sanitize_ascii_payload(val)


def create_parser() -> argparse.ArgumentParser:
    """Construct command line argument parser."""
    parser = argparse.ArgumentParser(
        prog="query_payload_ascii",
        description="Query SQLite DLT database and convert hex payloads to decoded ASCII text.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Query messages on a specific date and print first 50 decoded payloads:
  python query_payload_ascii.py evidence.db --date 2026-10-14

  # Query messages within a date range and export decoded results to CSV:
  python query_payload_ascii.py evidence.db --from-date "2026-10-14 08:00:00" --to-date "2026-10-14 18:00:00" -o decoded.csv

  # Filter by ECU and error level with regex search on decoded ASCII:
  python query_payload_ascii.py evidence.db --ecu ENGN --log-level error,fatal --grep "temperature|sensor"

  # Permanently add and populate 'payload_ascii' column across all 2M+ rows in SQLite:
  python query_payload_ascii.py evidence.db --update-db

  # Execute custom SQL query utilizing the built-in hex_to_ascii() function:
  python query_payload_ascii.py evidence.db --query "SELECT time_utc, ecu_id, hex_to_ascii(payload) FROM messages WHERE date(time_utc) = '2026-10-14'"
        """,
    )

    parser.add_argument(
        "database",
        type=Path,
        help="Path to the SQLite database (.db / .sqlite) containing DLT messages.",
    )

    # Date filtering
    date_group = parser.add_argument_group("Date & Time Filtering")
    date_group.add_argument(
        "-d",
        "--date",
        type=str,
        default=None,
        metavar="YYYY-MM-DD",
        help="Filter messages for a specific calendar date (e.g. '2026-10-14').",
    )
    date_group.add_argument(
        "--from-date",
        type=str,
        default=None,
        metavar="TIMESTAMP",
        help="Filter messages starting at this timestamp (e.g. '2026-10-14 08:00:00').",
    )
    date_group.add_argument(
        "--to-date",
        type=str,
        default=None,
        metavar="TIMESTAMP",
        help="Filter messages occurring at or before this timestamp (e.g. '2026-10-14 18:00:00').",
    )

    # Telemetry filtering
    filter_group = parser.add_argument_group("Telemetry Filtering")
    filter_group.add_argument(
        "--ecu",
        type=str,
        default=None,
        help="Filter by ECU ID(s), comma-separated (e.g. 'ENGN,BODY,GATEWAY').",
    )
    filter_group.add_argument(
        "--apid",
        type=str,
        default=None,
        help="Filter by Application ID(s), comma-separated (e.g. 'LOGD,NAVI,DIAG').",
    )
    filter_group.add_argument(
        "--ctid",
        type=str,
        default=None,
        help="Filter by Context ID(s), comma-separated (e.g. 'MAIN,INIT,CONN').",
    )
    filter_group.add_argument(
        "--log-level",
        type=str,
        default=None,
        help="Filter by message subtype/level(s), comma-separated (e.g. 'fatal,error,warn').",
    )
    filter_group.add_argument(
        "--grep",
        type=str,
        default=None,
        help="Search for regular expression keyword within decoded ASCII payload.",
    )
    filter_group.add_argument(
        "--query",
        type=str,
        default=None,
        help="Execute an arbitrary SQL query (can use the hex_to_ascii(col) SQL function).",
    )

    # Output & Action options
    action_group = parser.add_argument_group("Actions & Output")
    action_group.add_argument(
        "-o",
        "--output",
        type=Path,
        default=None,
        help="Save query results with decoded ASCII payload to a CSV file.",
    )
    action_group.add_argument(
        "--update-db",
        action="store_true",
        help="Add and populate a 'payload_ascii' column directly into the SQLite messages table.",
    )
    action_group.add_argument(
        "--batch-size",
        type=int,
        default=20000,
        metavar="N",
        help="Batch size for bulk updates (default: 20000 rows).",
    )
    action_group.add_argument(
        "-j",
        "--workers",
        type=int,
        default=None,
        metavar="N",
        help="Number of parallel worker processes for hex decoding (default: 75% of CPU cores; 1 for single-process).",
    )
    action_group.add_argument(
        "--cpu-percent",
        type=float,
        default=None,
        metavar="PCT",
        help="Percentage of system CPU cores to allocate to worker processes (e.g. 50 for 50%, 80 for 80%).",
    )
    action_group.add_argument(
        "--ram-cache-mb",
        type=int,
        default=128,
        metavar="MB",
        help="SQLite RAM page cache size in MB (default: 128 MB).",
    )
    action_group.add_argument(
        "--limit",
        type=int,
        default=50,
        help="Maximum rows to display in terminal (default: 50, use 0 for unlimited).",
    )
    action_group.add_argument(
        "-q",
        "--quiet",
        action="store_true",
        help="Suppress informational progress messages on stderr.",
    )

    return parser


def build_query(args: argparse.Namespace) -> Tuple[str, List[Any]]:
    """Build parameterized SQL query based on command line filters."""
    if args.query:
        return args.query, []

    where_clauses: List[str] = []
    params: List[Any] = []

    if args.date:
        # Utilize date() function or prefix scan
        where_clauses.append("date(time_utc) = ?")
        params.append(args.date.strip())

    if args.from_date:
        where_clauses.append("time_utc >= ?")
        params.append(args.from_date.strip())

    if args.to_date:
        where_clauses.append("time_utc <= ?")
        params.append(args.to_date.strip())

    if args.ecu:
        ecus = [e.strip() for e in args.ecu.split(",") if e.strip()]
        placeholders = ", ".join("?" for _ in ecus)
        where_clauses.append(f"ecu_id IN ({placeholders})")
        params.extend(ecus)

    if args.apid:
        apids = [a.strip() for a in args.apid.split(",") if a.strip()]
        placeholders = ", ".join("?" for _ in apids)
        where_clauses.append(f"apid IN ({placeholders})")
        params.extend(apids)

    if args.ctid:
        ctids = [c.strip() for c in args.ctid.split(",") if c.strip()]
        placeholders = ", ".join("?" for _ in ctids)
        where_clauses.append(f"ctid IN ({placeholders})")
        params.extend(ctids)

    if args.log_level:
        subtypes = [s.strip().lower() for s in args.log_level.split(",") if s.strip()]
        placeholders = ", ".join("?" for _ in subtypes)
        where_clauses.append(f"lower(subtype) IN ({placeholders})")
        params.extend(subtypes)

    where_str = ""
    if where_clauses:
        where_str = "WHERE " + " AND ".join(where_clauses)

    sql = f"""
        SELECT
            id,
            time_utc,
            ecu_id,
            apid,
            ctid,
            subtype,
            payload
        FROM messages
        {where_str}
        ORDER BY time_utc ASC, id ASC;
    """
    return sql, params


def update_database_in_place(
    db_path: Path,
    batch_size: int = 20000,
    workers: Optional[int] = None,
    cpu_percent: Optional[float] = None,
    ram_cache_mb: int = 128,
    quiet: bool = False,
) -> int:
    """Add payload_ascii column and populate it in bulk across all messages using multi-core processing."""
    from dlt_to_csv.sqlite_exporter import SqliteExporter

    exporter = SqliteExporter(db_path=db_path, batch_size=batch_size)
    updated = exporter.update_payload_ascii_in_place(
        batch_size=batch_size,
        workers=workers,
        cpu_percent=cpu_percent,
        ram_cache_mb=ram_cache_mb,
        quiet=quiet,
    )
    exporter.close(build_indexes=False)
    return updated


def main(argv: Optional[List[str]] = None) -> int:
    """CLI entrypoint."""
    parser = create_parser()
    args = parser.parse_args(argv)

    if not args.database.exists():
        sys.stderr.write(f"[!] Error: Database file not found: {args.database}\n")
        return 1

    # In-place update mode
    if args.update_db:
        update_database_in_place(
            args.database,
            batch_size=args.batch_size,
            workers=args.workers,
            cpu_percent=args.cpu_percent,
            ram_cache_mb=args.ram_cache_mb,
            quiet=args.quiet,
        )
        return 0

    # Query mode
    conn = sqlite3.connect(str(args.database))
    conn.row_factory = sqlite3.Row
    # Register hex_to_ascii UDF so user queries can use it directly
    conn.create_function("hex_to_ascii", 1, decode_hex_payload)
    cur = conn.cursor()

    sql, params = build_query(args)
    if not args.quiet:
        sys.stderr.write(f"[*] Executing query on {args.database.name} ...\n")
    cur.execute(sql, tuple(params))

    grep_compiled = re.compile(args.grep, re.IGNORECASE) if args.grep else None

    # CSV Export Mode
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        col_names = [d[0] for d in cur.description]
        # Ensure payload_ascii is reflected in headers
        out_headers = [c if c != "payload" else "payload_ascii" for c in col_names]
        if "payload_ascii" not in out_headers and "payload" in col_names:
            out_headers.append("payload_ascii")

        exported_rows = 0
        with open(args.output, "w", encoding="utf-8-sig", newline="") as f:
            writer = csv.writer(f, delimiter=",", quoting=csv.QUOTE_ALL, lineterminator="\n")
            writer.writerow(out_headers)

            while True:
                chunk = cur.fetchmany(10000)
                if not chunk:
                    break

                for row in chunk:
                    r_dict = dict(row)
                    raw_payload = r_dict.get("payload", "")
                    decoded = decode_hex_payload(raw_payload)

                    if grep_compiled and not grep_compiled.search(decoded):
                        continue

                    # Replace or add decoded payload
                    row_vals = []
                    for h in col_names:
                        if h == "payload":
                            row_vals.append(decoded)
                        else:
                            row_vals.append(r_dict.get(h, ""))

                    writer.writerow(row_vals)
                    exported_rows += 1

        conn.close()
        sys.stderr.write(f"[+] Decoded {exported_rows:,d} message(s) exported to: {args.output}\n")
        return 0

    # Terminal Output Mode
    matched_rows = 0
    displayed_rows = 0
    limit = args.limit if args.limit > 0 else float("inf")

    col_names = [d[0] for d in cur.description]
    sys.stdout.write(f"\n{'=' * 120}\n")
    sys.stdout.write(f"{'Time (UTC)':<24} | {'ECU':<6} | {'APID':<6} | {'Level':<8} | {'Decoded ASCII Payload'}\n")
    sys.stdout.write(f"{'-' * 120}\n")

    while True:
        chunk = cur.fetchmany(5000)
        if not chunk:
            break

        for row in chunk:
            r_dict = dict(row)
            raw_payload = r_dict.get("payload", "")
            decoded = decode_hex_payload(raw_payload)

            if grep_compiled and not grep_compiled.search(decoded):
                continue

            matched_rows += 1
            if displayed_rows < limit:
                time_str = str(r_dict.get("time_utc", ""))
                ecu_str = str(r_dict.get("ecu_id", ""))
                apid_str = str(r_dict.get("apid", ""))
                sub_str = str(r_dict.get("subtype", ""))
                # Clean payload of non-printable characters for terminal
                safe_display = "".join(c if (c.isprintable() or c == " ") else f"\\x{ord(c):02x}" for c in decoded)
                sys.stdout.write(f"{time_str:<24} | {ecu_str:<6} | {apid_str:<6} | {sub_str:<8} | {safe_display}\n")
                displayed_rows += 1

    sys.stdout.write(f"{'-' * 120}\n")
    if matched_rows > displayed_rows:
        sys.stdout.write(
            f"... [Showing first {displayed_rows} of {matched_rows:,d} matching rows. Use -o <file.csv> or --limit 0 to view all]\n\n"
        )
    else:
        sys.stdout.write(f"[+] Total matched rows: {matched_rows:,d}\n\n")

    conn.close()
    return 0


if __name__ == "__main__":
    import multiprocessing
    multiprocessing.freeze_support()
    sys.exit(main())

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

"""Command Line Interface (CLI) for parsing DLT files to CSV.

Provides forensic-grade CLI options, folder batch scanning, filtering,
hash calculation, and audit report generation.
"""

import argparse
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import List, Optional, Union

from .constants import DEFAULT_COLUMNS, FORENSIC_COLUMNS
from .exporter import DltFilter, process_dlt_files
from .scanner import discover_dlt_files

__version__ = "2.0.0"


def parse_timestamp_arg(val: Optional[str]) -> Optional[int]:
    """Parse a date/time string (ISO 8601, 'YYYY/MM/DD HH:MM:SS', or epoch seconds) into epoch integer."""
    if not val:
        return None

    # Try integer epoch seconds
    try:
        return int(val)
    except ValueError:
        pass

    # Try ISO 8601 or standard datetime formats
    formats = [
        "%Y-%m-%dT%H:%M:%SZ",
        "%Y-%m-%dT%H:%M:%S",
        "%Y-%m-%d %H:%M:%S",
        "%Y/%m/%d %H:%M:%S",
        "%Y-%m-%d",
    ]
    for fmt in formats:
        try:
            dt = datetime.strptime(val, fmt)
            return int(dt.replace(tzinfo=timezone.utc).timestamp())
        except ValueError:
            continue

    raise argparse.ArgumentTypeError(
        f"Invalid date/time format: '{val}'. Supported formats include 'YYYY-MM-DD HH:MM:SS' or UNIX epoch seconds."
    )


def expand_db_paths(db_specs: Union[str, Path, List[Union[str, Path]]]) -> List[Path]:
    """Expand string specs, lists, wildcards (*.db), and directories into resolved SQLite database paths."""
    if not isinstance(db_specs, list):
        db_specs = [db_specs]

    found: List[Path] = []
    seen = set()

    for item in db_specs:
        raw = str(item).strip()
        if not raw:
            continue
        p = Path(raw)

        # Check for glob wildcard characters (*, ?, [])
        if any(char in raw for char in ("*", "?", "[")):
            parent_dir = p.parent if str(p.parent) not in ("", ".") else Path(".")
            pattern = p.name
            matches = sorted(parent_dir.glob(pattern))
            for m in matches:
                if m.is_file() and m.resolve() not in seen:
                    seen.add(m.resolve())
                    found.append(m)
        elif p.is_dir():
            for ext in ("*.db", "*.sqlite", "*.sqlite3"):
                for m in sorted(p.glob(ext)):
                    if m.is_file() and m.resolve() not in seen:
                        seen.add(m.resolve())
                        found.append(m)
        elif p.is_file():
            if p.resolve() not in seen:
                seen.add(p.resolve())
                found.append(p)
        else:
            # File might not exist yet if specified as export target
            found.append(p)

    return found


def create_parser() -> argparse.ArgumentParser:
    """Construct argument parser with forensic and batch conversion options."""
    parser = argparse.ArgumentParser(
        prog="dlt-to-csv",
        description="Forensic CLI tool to parse automotive DLT (Diagnostic Log and Trace) files and export to CSV.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Parse all DLT files in a folder into individual CSV files:
  python dlt_to_csv.py C:\\Evidence\\Vehicle_DLT\\ -o C:\\Analysis\\CSV_Output\\

  # Recursively process a folder and merge into a single combined CSV:
  python dlt_to_csv.py /path/to/dlt_logs/ -r -c -o combined_forensic_export.csv

  # Extract only ERROR and FATAL messages with forensic traceability columns:
  python dlt_to_csv.py /evidence/dlt/ --forensic --log-level error,fatal -o /analysis/

  # Ingest 2M+ row dataset into an indexed SQLite database:
  dlt-to-csv.exe C:\Evidence\DLT\ --sqlite evidence.db --no-csv

  # Perform an in-place batch update transaction to decode all hex payloads to ASCII:
  dlt-to-csv.exe --sqlite evidence.db --update-ascii

  # Parallel bulk decode across multiple databases using 80% CPU and 50k batch size:
  dlt-to-csv.exe --sqlite *.db --update-ascii --cpu-percent 80 --batch-size 50000

  # Query messages on a specific date or inspect daily breakdown:
  dlt-to-csv.exe --sqlite evidence.db --query-dates
  dlt-to-csv.exe --sqlite evidence.db --query "SELECT time_utc, ecu_id, payload_ascii FROM messages WHERE date(time_utc) = '2026-10-14'"
        """,
    )

    parser.add_argument(
        "inputs",
        nargs="*",
        type=Path,
        default=[],
        help="Path to folder(s) containing 1..* DLT files, or individual .dlt file(s).",
    )

    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=None,
        help="Target output directory (for individual CSVs) or file path (if using --combined).",
    )

    parser.add_argument(
        "-r",
        "--recursive",
        action="store_true",
        help="Recursively search subdirectories for .dlt files.",
    )

    parser.add_argument(
        "-c",
        "--combined",
        action="store_true",
        help="Merge all parsed messages from all discovered DLT files into a single CSV file.",
    )

    parser.add_argument(
        "--forensic",
        action="store_true",
        help="Include forensic provenance columns: SourceFile, FileOffset, MsgLength, MsgID, EpochSeconds, Microseconds.",
    )

    # Database options for 2M+ row scalability
    db_group = parser.add_argument_group("SQLite Database & Query Options (Large Datasets, 2M+ rows)")
    db_group.add_argument(
        "--sqlite",
        "--db",
        nargs="+",
        default=None,
        dest="sqlite",
        metavar="DB_PATH",
        help="Export parsed DLT messages into SQLite, or target SQLite database(s)/wildcards (*.db) for queries/updates.",
    )
    db_group.add_argument(
        "--batch-size",
        "--sqlite-batch-size",
        type=int,
        default=20000,
        dest="sqlite_batch_size",
        metavar="N",
        help="Batch size for bulk SQLite transactions and updates (default: 20000 rows).",
    )
    db_group.add_argument(
        "-j",
        "--workers",
        type=int,
        default=None,
        metavar="N",
        help="Number of parallel worker processes for hex decoding (default: 75%% of CPU cores; 1 for single-process).",
    )
    db_group.add_argument(
        "--cpu-percent",
        type=float,
        default=None,
        metavar="PCT",
        help="Percentage of system CPU cores to allocate to worker processes (e.g. 50 for 50%%, 80 for 80%%).",
    )
    db_group.add_argument(
        "--ram-cache-mb",
        type=int,
        default=128,
        metavar="MB",
        help="SQLite RAM page cache size in MB (default: 128 MB).",
    )
    db_group.add_argument(
        "--no-csv",
        action="store_true",
        help="Suppress CSV file generation when exporting to SQLite.",
    )
    db_group.add_argument(
        "--query",
        type=str,
        default=None,
        help="Execute an SQL query against the SQLite database (e.g. 'SELECT * FROM messages WHERE date(time_utc) = ...').",
    )
    db_group.add_argument(
        "--query-dates",
        action="store_true",
        help="Query and display daily message counts, active ECUs, and error breakdown across all log dates.",
    )
    db_group.add_argument(
        "--query-export",
        type=Path,
        default=None,
        help="Export results of --query directly into a CSV file path.",
    )
    db_group.add_argument(
        "--update-ascii",
        "--update-db",
        action="store_true",
        dest="update_ascii",
        help="Perform an in-place batch transaction on the SQLite DB to decode all hex payloads into the indexed 'payload_ascii' column.",
    )

    parser.add_argument(
        "--columns",
        type=str,
        default=None,
        help="Custom comma-separated list of column headers to export (e.g. 'Time,Ecuid,Apid,Payload').",
    )

    parser.add_argument(
        "-d",
        "--delimiter",
        type=str,
        default=",",
        help="Field delimiter character for CSV output (default: ',').",
    )

    parser.add_argument(
        "--quote-all",
        action="store_true",
        default=True,
        help="Enclose all CSV fields in quotation marks (RFC 4180 strict quoting, default: enabled).",
    )

    parser.add_argument(
        "--no-quote-all",
        "--quote-minimal",
        action="store_false",
        dest="quote_all",
        help="Disable strict quoting and only quote fields containing special characters.",
    )

    parser.add_argument(
        "--replace-commas",
        "--sanitize-commas",
        nargs="?",
        const=";",
        default=None,
        metavar="CHAR",
        help="Replace commas within the Payload column with CHAR (default: ';') to prevent downstream naive split errors.",
    )

    # Timezone options
    parser.add_argument(
        "--timezone",
        type=str,
        default="utc",
        choices=["utc", "local"],
        help="Timestamp formatting mode: 'utc' (default, recommended for forensics) or 'local'.",
    )

    parser.add_argument(
        "--tz-offset",
        type=int,
        default=0,
        help="Optional timezone offset in hours to apply to UTC timestamp (e.g. 1 or -5).",
    )

    # Filtering options
    filter_group = parser.add_argument_group("Forensic Filtering Options")
    filter_group.add_argument(
        "--ecu",
        type=str,
        default=None,
        help="Filter by ECU ID(s), comma-separated (e.g. 'ECU1,BODY,HEADUNIT').",
    )
    filter_group.add_argument(
        "--apid",
        type=str,
        default=None,
        help="Filter by Application ID(s), comma-separated (e.g. 'LOGD,NAVI,AUDI').",
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
        help="Filter by log level/subtype(s), comma-separated (e.g. 'fatal,error,warn').",
    )
    filter_group.add_argument(
        "--grep",
        type=str,
        default=None,
        help="Regular expression pattern to search within payload text.",
    )
    filter_group.add_argument(
        "--case-sensitive",
        action="store_true",
        help="Perform case-sensitive matching when using --grep.",
    )
    filter_group.add_argument(
        "--start-time",
        type=parse_timestamp_arg,
        default=None,
        help="Filter messages starting at/after this timestamp (ISO 8601 or epoch seconds).",
    )
    filter_group.add_argument(
        "--end-time",
        type=parse_timestamp_arg,
        default=None,
        help="Filter messages occurring at/before this timestamp (ISO 8601 or epoch seconds).",
    )

    # Integrity & Auditing options
    audit_group = parser.add_argument_group("Forensic Integrity & Audit Options")
    audit_group.add_argument(
        "--no-hash",
        action="store_true",
        help="Disable SHA-256/MD5 cryptographic file hash computation.",
    )
    audit_group.add_argument(
        "--report",
        type=Path,
        default=None,
        help="Save a JSON forensic audit summary report to the specified file path.",
    )

    # Output control
    parser.add_argument(
        "--stdout",
        action="store_true",
        help="Stream CSV output directly to stdout (useful for piping into head, grep, etc.).",
    )
    parser.add_argument(
        "-q",
        "--quiet",
        action="store_true",
        help="Suppress informational output on stderr.",
    )
    parser.add_argument(
        "-v",
        "--version",
        action="version",
        version=f"%(prog)s {__version__}",
    )

    return parser


def run_sqlite_query_cli(args: argparse.Namespace) -> int:
    """Execute interactive or automated SQL queries / in-place updates against SQLite database(s)."""
    from .sqlite_exporter import SqliteExporter

    db_specs = args.sqlite or []
    if isinstance(db_specs, (str, Path)):
        db_specs = [db_specs]

    db_paths = expand_db_paths(db_specs)
    existing_paths = [p for p in db_paths if p.exists() and p.is_file()]
    if not existing_paths:
        sys.stderr.write(f"[!] Error: No SQLite database files found matching: {args.sqlite}\n")
        return 1

    exit_code = 0
    total_dbs = len(existing_paths)

    for idx, db_path in enumerate(existing_paths, 1):
        if total_dbs > 1 and not args.quiet:
            sys.stderr.write(f"\n[{idx}/{total_dbs}] Target Database: {db_path.name}\n")

        exporter = SqliteExporter(
            db_path=db_path,
            batch_size=args.sqlite_batch_size,
            use_utc=(args.timezone == "utc"),
            tz_offset_seconds=args.tz_offset * 3600,
        )

        if getattr(args, "update_ascii", False):
            exporter.update_payload_ascii_in_place(
                batch_size=args.sqlite_batch_size,
                workers=getattr(args, "workers", None),
                cpu_percent=getattr(args, "cpu_percent", None),
                ram_cache_mb=getattr(args, "ram_cache_mb", 128),
                quiet=args.quiet,
            )

        if args.query_dates:
            summary_rows = exporter.get_date_summary()
            if not summary_rows:
                sys.stdout.write(f"[*] No timestamped messages found in {db_path.name}.\n")
            else:
                sys.stdout.write(f"\n{'=' * 116}\n")
                sys.stdout.write(f"DLT Dataset Summary by Date: {db_path.name}\n")
                sys.stdout.write(f"{'=' * 116}\n")
                sys.stdout.write(
                    f"{'Log Date':<12} | {'Total Msgs':>12} | {'ECUs':>6} | {'Fatal':>7} | {'Error':>7} | {'Warn':>7} | {'First Timestamp (UTC)':<26} | {'Last Timestamp (UTC)':<26}\n"
                )
                sys.stdout.write(f"{'-' * 116}\n")
                for r in summary_rows:
                    sys.stdout.write(
                        f"{r.get('log_date', ''):<12} | "
                        f"{r.get('total_messages', 0):>12,d} | "
                        f"{r.get('distinct_ecus', 0):>6d} | "
                        f"{r.get('fatal_count', 0):>7,d} | "
                        f"{r.get('error_count', 0):>7,d} | "
                        f"{r.get('warn_count', 0):>7,d} | "
                        f"{str(r.get('start_time', '')):<26} | "
                        f"{str(r.get('end_time', '')):<26}\n"
                    )
                sys.stdout.write(f"{'-' * 116}\n\n")

        if args.query:
            sql = args.query.strip()
            if args.query_export:
                count = exporter.export_query_to_csv(
                    sql=sql,
                    output_path=args.query_export,
                    delimiter=args.delimiter,
                    quote_all=args.quote_all,
                )
                sys.stderr.write(f"[+] Query returned {count:,d} row(s) exported to: {args.query_export}\n")
            else:
                rows = exporter.query(sql)
                if not rows:
                    sys.stdout.write(f"[*] Query returned 0 rows for {db_path.name}.\n")
                else:
                    keys = list(rows[0].keys())
                    header_str = " | ".join(keys)
                    sys.stdout.write(f"\n{header_str}\n")
                    sys.stdout.write(f"{'-' * len(header_str)}\n")
                    for r in rows[:100]:
                        sys.stdout.write(" | ".join(str(r.get(k, "")) for k in keys) + "\n")
                    if len(rows) > 100:
                        sys.stdout.write(
                            f"... [Truncated. Total rows: {len(rows):,d}. Use --query-export <file.csv> to save full results]\n"
                        )
                    sys.stdout.write("\n")

        exporter.close(build_indexes=False)

    return exit_code


def main(argv: Optional[List[str]] = None) -> int:
    """CLI entrypoint."""
    parser = create_parser()
    args = parser.parse_args(argv)

    # Check for direct SQLite query or in-place update on existing database without new inputs
    is_sqlite_op = bool(args.query or args.query_dates or getattr(args, "update_ascii", False))
    if is_sqlite_op:
        if not args.inputs:
            if args.sqlite:
                return run_sqlite_query_cli(args)
        else:
            # Check if user passed .db / .sqlite files as positional inputs instead of DLT files
            resolved_dbs = expand_db_paths([str(p) for p in args.inputs])
            existing_dbs = [
                p for p in resolved_dbs if p.exists() and p.is_file() and p.suffix.lower() in (".db", ".sqlite", ".sqlite3")
            ]
            if existing_dbs and not args.sqlite:
                args.sqlite = existing_dbs
                return run_sqlite_query_cli(args)

    if not args.inputs:
        parser.error("the following arguments are required: inputs")

    # Discover target files
    files = discover_dlt_files(args.inputs, recursive=args.recursive)
    if not files:
        sys.stderr.write("[!] Error: No .dlt files found in the specified target(s).\n")
        return 1

    if not args.quiet:
        sys.stderr.write(f"[*] Discovered {len(files)} DLT file(s) to process.\n")

    # Column configuration
    columns = None
    if args.columns:
        columns = [c.strip() for c in args.columns.split(",") if c.strip()]
    elif args.forensic:
        columns = FORENSIC_COLUMNS
    else:
        columns = DEFAULT_COLUMNS

    # Build filter if requested
    ecus = args.ecu.split(",") if args.ecu else None
    apids = args.apid.split(",") if args.apid else None
    ctids = args.ctid.split(",") if args.ctid else None
    subtypes = args.log_level.split(",") if args.log_level else None

    msg_filter = None
    if any([ecus, apids, ctids, subtypes, args.grep, args.start_time, args.end_time]):
        msg_filter = DltFilter(
            ecus=ecus,
            apids=apids,
            ctids=ctids,
            subtypes=subtypes,
            grep=args.grep,
            case_sensitive=args.case_sensitive,
            start_time_epoch=args.start_time,
            end_time_epoch=args.end_time,
        )

    # Resolve destination SQLite database if specified
    sqlite_dest = None
    if args.sqlite:
        expanded = expand_db_paths(args.sqlite)
        sqlite_dest = expanded[0] if expanded else Path(str(args.sqlite[0]))

    # Determine CSV suppression when exporting to SQLite
    no_csv = args.no_csv or (sqlite_dest is not None and args.output is None and not args.stdout and not args.combined)

    # Handle output target default if CSV is being generated
    output_target = args.output
    if not no_csv and output_target is None and not args.stdout:
        first_input = args.inputs[0]
        output_target = first_input if first_input.is_dir() else first_input.parent

    try:
        report = process_dlt_files(
            files=files,
            output_target=output_target,
            combined=args.combined,
            columns=columns,
            delimiter=args.delimiter,
            use_utc=(args.timezone == "utc"),
            tz_offset_seconds=args.tz_offset * 3600,
            quote_all=args.quote_all,
            replace_commas=args.replace_commas,
            msg_filter=msg_filter,
            compute_hashes=not args.no_hash,
            report_path=args.report,
            stdout=args.stdout,
            quiet=args.quiet,
            sqlite_path=sqlite_dest,
            sqlite_batch_size=args.sqlite_batch_size,
            no_csv=no_csv,
        )

        if not args.quiet and not args.stdout:
            sys.stderr.write(
                f"\n[+] Conversion complete in {report['duration_seconds']}s.\n"
                f"    Total messages parsed:   {report['total_messages_parsed']:,d}\n"
                f"    Total messages exported: {report['total_messages_exported']:,d}\n"
            )
            if report.get("sqlite_database"):
                sys.stderr.write(
                    f"    SQLite database:         {report['sqlite_database']} "
                    f"({report.get('sqlite_messages_inserted', 0):,d} rows inserted)\n"
                )
            if report.get("output_csv_files"):
                sys.stderr.write(f"    CSV files written:       {len(report['output_csv_files'])}\n")
            if args.report:
                sys.stderr.write(f"    Forensic audit report:   {args.report}\n")

        # If user also requested queries or database updates immediately after import
        if sqlite_dest and (args.query or args.query_dates or getattr(args, "update_ascii", False)):
            args.sqlite = [sqlite_dest]
            run_sqlite_query_cli(args)

        return 0

    except Exception as exc:
        sys.stderr.write(f"[!] Fatal error during DLT parsing: {exc}\n")
        return 2


if __name__ == "__main__":
    sys.exit(main())

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

"""CSV Exporter and filtering engine for DLT messages.

Provides high-throughput streaming CSV generation, customizable column
selection, forensic auditing metadata, and flexible multi-criteria filtering.
"""

import csv
import json
import re
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional, Pattern, Set, TextIO, Union

from .constants import DEFAULT_COLUMNS, FORENSIC_COLUMNS
from .models import DltMessage, FileScanResult
from .scanner import DltFileScanner


class DltFilter:
    """Predicate filter for DLT messages based on ECU, App ID, Log Level, and Payload."""

    def __init__(
        self,
        ecus: Optional[Iterable[str]] = None,
        apids: Optional[Iterable[str]] = None,
        ctids: Optional[Iterable[str]] = None,
        subtypes: Optional[Iterable[str]] = None,
        grep: Optional[str] = None,
        case_sensitive: bool = False,
        start_time_epoch: Optional[int] = None,
        end_time_epoch: Optional[int] = None,
    ):
        self.ecus = {e.strip().upper() for e in ecus} if ecus else None
        self.apids = {a.strip().upper() for a in apids} if apids else None
        self.ctids = {c.strip().upper() for c in ctids} if ctids else None
        self.subtypes = {s.strip().lower() for s in subtypes} if subtypes else None

        self.grep_pattern: Optional[Pattern] = None
        if grep:
            flags = 0 if case_sensitive else re.IGNORECASE
            self.grep_pattern = re.compile(grep, flags)

        self.start_time_epoch = start_time_epoch
        self.end_time_epoch = end_time_epoch

    def matches(self, msg: DltMessage) -> bool:
        """Return True if the message passes all configured filter criteria."""
        if self.ecus and msg.ecu_id.upper() not in self.ecus:
            return False

        if self.apids and msg.apid.upper() not in self.apids:
            return False

        if self.ctids and msg.ctid.upper() not in self.ctids:
            return False

        if self.subtypes and msg.subtype.lower() not in self.subtypes:
            return False

        if self.start_time_epoch is not None and msg.time_seconds < self.start_time_epoch:
            return False

        if self.end_time_epoch is not None and msg.time_seconds > self.end_time_epoch:
            return False

        if self.grep_pattern:
            # Search both payload text and argument values
            if not self.grep_pattern.search(msg.payload_text):
                return False

        return True


class CsvExporter:
    """Streams DLT messages to CSV format with RFC 4180 compliance."""

    def __init__(
        self,
        columns: Optional[List[str]] = None,
        delimiter: str = ",",
        use_utc: bool = True,
        tz_offset_seconds: int = 0,
        quote_all: bool = True,
        replace_commas: Optional[str] = None,
    ):
        self.columns = columns or DEFAULT_COLUMNS
        self.delimiter = delimiter
        self.use_utc = use_utc
        self.tz_offset_seconds = tz_offset_seconds
        self.quoting = csv.QUOTE_ALL if quote_all else csv.QUOTE_MINIMAL
        self.replace_commas = replace_commas

    def write_header(self, output: TextIO) -> None:
        """Write the CSV header line."""
        writer = csv.writer(
            output,
            delimiter=self.delimiter,
            quoting=self.quoting,
            lineterminator="\n",
        )
        writer.writerow(self.columns)

    def write_message(self, output: TextIO, msg: DltMessage) -> None:
        """Write a single DltMessage as a CSV row."""
        writer = csv.writer(
            output,
            delimiter=self.delimiter,
            quoting=self.quoting,
            lineterminator="\n",
        )
        row_dict = msg.to_dict(
            columns=self.columns,
            use_utc=self.use_utc,
            tz_offset_seconds=self.tz_offset_seconds,
            replace_commas=self.replace_commas,
        )
        row = [row_dict.get(col, "") for col in self.columns]
        writer.writerow(row)

    def export_stream(
        self,
        messages: Iterable[DltMessage],
        output: TextIO,
        msg_filter: Optional[DltFilter] = None,
        write_header: bool = True,
    ) -> int:
        """Export an iterable of DltMessages to output stream, returning count of exported rows."""
        writer = csv.writer(
            output,
            delimiter=self.delimiter,
            quoting=self.quoting,
            lineterminator="\n",
        )
        if write_header:
            writer.writerow(self.columns)

        count = 0
        for msg in messages:
            if msg_filter is None or msg_filter.matches(msg):
                row_dict = msg.to_dict(
                    columns=self.columns,
                    use_utc=self.use_utc,
                    tz_offset_seconds=self.tz_offset_seconds,
                    replace_commas=self.replace_commas,
                )
                row = [row_dict.get(col, "") for col in self.columns]
                writer.writerow(row)
                count += 1

        return count


def process_dlt_files(
    files: List[Path],
    output_target: Optional[Path] = None,
    combined: bool = False,
    columns: Optional[List[str]] = None,
    delimiter: str = ",",
    use_utc: bool = True,
    tz_offset_seconds: int = 0,
    quote_all: bool = True,
    replace_commas: Optional[str] = None,
    msg_filter: Optional[DltFilter] = None,
    compute_hashes: bool = True,
    report_path: Optional[Path] = None,
    stdout: bool = False,
    quiet: bool = False,
    sqlite_path: Optional[Path] = None,
    sqlite_batch_size: int = 20000,
    no_csv: bool = False,
) -> Dict[str, Any]:
    """Execute the conversion pipeline across 1..* DLT files.

    Args:
        files: List of .dlt file paths to process.
        output_target: Output directory or combined output file path.
        combined: If True, writes all messages into a single combined CSV.
        columns: List of column names to include in CSV output.
        delimiter: Field delimiter character (default ',').
        use_utc: If True, formats time in UTC.
        tz_offset_seconds: Additional offset in seconds to add to timestamps.
        quote_all: Enforce RFC 4180 strict quoting on all fields (default True).
        replace_commas: Optional string to replace commas in payload (e.g. ';').
        msg_filter: Optional DltFilter instance for selective export.
        compute_hashes: Compute cryptographic SHA-256/MD5 hashes.
        report_path: Optional file path to write JSON forensic audit summary.
        stdout: If True, write CSV output directly to sys.stdout.
        quiet: Suppress progress and status messages.
        sqlite_path: Optional path to export messages into an indexed SQLite database.
        sqlite_batch_size: Bulk insertion batch size for SQLite (default: 20000).
        no_csv: If True, disables CSV generation (useful when only exporting to SQLite).

    Returns:
        Forensic report dictionary summarizing the run.
    """
    start_run = datetime.now(timezone.utc)
    exporter = CsvExporter(
        columns=columns,
        delimiter=delimiter,
        use_utc=use_utc,
        tz_offset_seconds=tz_offset_seconds,
        quote_all=quote_all,
        replace_commas=replace_commas,
    )

    sqlite_exporter = None
    if sqlite_path:
        from .sqlite_exporter import SqliteExporter
        sqlite_exporter = SqliteExporter(
            db_path=sqlite_path,
            batch_size=sqlite_batch_size,
            use_utc=use_utc,
            tz_offset_seconds=tz_offset_seconds,
            replace_commas=replace_commas,
        )

    per_file_results: List[Dict[str, Any]] = []
    total_messages_processed = 0
    total_messages_exported = 0
    generated_csv_files: List[str] = []

    if stdout and not no_csv:
        exporter.write_header(sys.stdout)

    combined_fp: Optional[TextIO] = None
    if combined and not stdout and not no_csv:
        if output_target is None:
            raise ValueError("Output target must be specified when using --combined mode.")
        if output_target.is_dir():
            combined_file_path = output_target / "combined_dlt_export.csv"
        else:
            combined_file_path = output_target
        combined_file_path.parent.mkdir(parents=True, exist_ok=True)
        combined_fp = open(combined_file_path, "w", encoding="utf-8-sig", newline="")
        exporter.write_header(combined_fp)
        generated_csv_files.append(str(combined_file_path))

    for file_path in files:
        if not quiet:
            sys.stderr.write(f"[*] Processing: {file_path.name} ...\n")
            sys.stderr.flush()

        file_id = None
        if sqlite_exporter:
            file_id = sqlite_exporter.record_file(file_path)

        scanner = DltFileScanner(file_path, compute_hashes=compute_hashes)
        messages_generator = scanner.scan()

        file_exported_count = 0
        csv_fp: Optional[TextIO] = None

        if not no_csv and not stdout and not combined:
            # Individual CSV mode
            if output_target:
                if output_target.suffix.lower() == ".csv" and len(files) == 1:
                    target_csv = output_target
                    target_csv.parent.mkdir(parents=True, exist_ok=True)
                elif output_target.is_dir() or output_target.suffix.lower() != ".csv":
                    output_target.mkdir(parents=True, exist_ok=True)
                    target_csv = output_target / f"{file_path.stem}.csv"
                else:
                    output_target.parent.mkdir(parents=True, exist_ok=True)
                    target_csv = output_target.parent / f"{file_path.stem}.csv"
            else:
                target_csv = file_path.parent / f"{file_path.stem}.csv"

            csv_fp = open(target_csv, "w", encoding="utf-8-sig", newline="")
            exporter.write_header(csv_fp)
            generated_csv_files.append(str(target_csv))

        for msg in messages_generator:
            if msg_filter is None or msg_filter.matches(msg):
                if sqlite_exporter and file_id is not None:
                    sqlite_exporter.insert_message(msg, file_id)

                if not no_csv:
                    if stdout:
                        exporter.write_message(sys.stdout, msg)
                    elif combined:
                        assert combined_fp is not None
                        exporter.write_message(combined_fp, msg)
                    elif csv_fp:
                        exporter.write_message(csv_fp, msg)

                file_exported_count += 1

        if csv_fp is not None:
            csv_fp.close()

        if sqlite_exporter and file_id is not None:
            sqlite_exporter.record_file(file_path, scanner.result)

        scan_summary = scanner.result.to_dict()
        scan_summary["exported_messages"] = file_exported_count
        per_file_results.append(scan_summary)

        total_messages_processed += scanner.result.valid_messages
        total_messages_exported += file_exported_count

        if not quiet:
            sys.stderr.write(
                f"    [+] {file_path.name}: {scanner.result.valid_messages} msgs parsed, "
                f"{file_exported_count} exported, {scanner.result.skipped_bytes} corrupt bytes.\n"
            )
            sys.stderr.flush()

    if combined_fp is not None:
        combined_fp.close()

    total_sqlite_rows = 0
    if sqlite_exporter:
        if not quiet:
            sys.stderr.write("[*] Building SQLite indexes and analytical views ...\n")
            sys.stderr.flush()
        total_sqlite_rows = sqlite_exporter.close(build_indexes=True)

    end_run = datetime.now(timezone.utc)
    duration = (end_run - start_run).total_seconds()

    report = {
        "tool": "DLT-to-CSV Forensic Converter",
        "version": "2.0.0",
        "execution_start_utc": start_run.strftime("%Y-%m-%d %H:%M:%SZ"),
        "execution_end_utc": end_run.strftime("%Y-%m-%d %H:%M:%SZ"),
        "duration_seconds": round(duration, 4),
        "total_files_processed": len(files),
        "total_messages_parsed": total_messages_processed,
        "total_messages_exported": total_messages_exported,
        "output_csv_files": generated_csv_files,
        "sqlite_database": str(sqlite_path) if sqlite_path else None,
        "sqlite_messages_inserted": total_sqlite_rows if sqlite_path else 0,
        "file_details": per_file_results,
    }

    if report_path:
        report_path = Path(report_path)
        report_path.parent.mkdir(parents=True, exist_ok=True)
        with open(report_path, "w", encoding="utf-8") as rf:
            json.dump(report, rf, indent=2)

    return report

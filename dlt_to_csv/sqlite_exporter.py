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

"""SQLite Database Exporter and analytical query engine for DLT messages.

Designed for high-throughput batch ingestion of large forensic log datasets
(2,000,000+ rows) with indexed date and time queries.
"""

import csv
import json
import os
import re
import sqlite3
import unicodedata
from concurrent.futures import ProcessPoolExecutor
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional, Tuple, Union

from .models import DltMessage, FileScanResult
from .exporter import DltFilter


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
    # Support space, colon, or hyphen separated byte tokens (e.g. '11 40 c2 v2 00...')
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


def _worker_decode_batch(items: List[Tuple[int, Any]]) -> List[Tuple[str, int, bool]]:
    """Worker function to decode a slice of (row_id, payload) in parallel."""
    results: List[Tuple[str, int, bool]] = []
    for row_id, payload in items:
        decoded = decode_hex_payload(payload)
        results.append((decoded, row_id, decoded != payload))
    return results


def resolve_workers(workers: Optional[int] = None, cpu_percent: Optional[float] = None) -> int:
    """Determine optimal worker process count based on CPU percentage or explicit count.

    Defaults to 75% of available CPU cores (e.g. 10 cores on 14-core CPU) to maximize
    throughput while leaving the system responsive. Use workers=1 for single-threaded.
    """
    total_cpus = os.cpu_count() or 4
    if workers is not None and workers > 0:
        return min(workers, total_cpus * 2)
    if cpu_percent is not None and cpu_percent > 0:
        ratio = min(max(cpu_percent, 5.0), 100.0) / 100.0
        return max(1, int(round(total_cpus * ratio)))
    # Default: 75% of available CPU cores
    return max(1, int(round(total_cpus * 0.75)))


class SqliteExporter:
    """High-throughput SQLite database exporter and query engine for DLT logs.

    Features:
      * Bulk ingestion optimized for 2M+ rows via WAL mode, memory caching,
        and configurable batch transactions.
      * Standard ISO-8601 timestamps ('YYYY-MM-DD HH:MM:SS.ffffff') enabling
        native SQLite date/time functions (date(), time(), datetime(), strftime()).
      * B-Tree indexes on time_utc, epoch_seconds, ecu_id, apid, and log level.
      * Evidentiary dlt_files audit tracking table and analytical views.
      * Built-in hex_to_ascii() SQL function for payload decoding.
    """

    DEFAULT_BATCH_SIZE = 20000

    def __init__(
        self,
        db_path: Union[str, Path],
        batch_size: int = DEFAULT_BATCH_SIZE,
        use_utc: bool = True,
        tz_offset_seconds: int = 0,
        replace_commas: Optional[str] = None,
    ):
        self.db_path = Path(db_path)
        self.batch_size = max(100, batch_size)
        self.use_utc = use_utc
        self.tz_offset_seconds = tz_offset_seconds
        self.replace_commas = replace_commas

        self.db_path.parent.mkdir(parents=True, exist_ok=True)
        self.conn = sqlite3.connect(str(self.db_path))
        self.conn.row_factory = sqlite3.Row
        self.conn.create_function("hex_to_ascii", 1, decode_hex_payload)
        self.cursor = self.conn.cursor()

        self._buffer: List[Tuple[Any, ...]] = []
        self._total_rows_inserted = 0
        self._in_transaction = False

        self._configure_pragmas()
        self._create_schema()

    def _configure_pragmas(self) -> None:
        """Configure SQLite engine pragmas for maximum bulk insertion throughput."""
        pragmas = [
            "PRAGMA journal_mode = WAL;",
            "PRAGMA synchronous = OFF;",
            "PRAGMA cache_size = -64000;",      # 64 MB memory cache
            "PRAGMA temp_store = MEMORY;",
            "PRAGMA foreign_keys = ON;",
        ]
        for pragma in pragmas:
            self.cursor.execute(pragma)

    def _create_schema(self) -> None:
        """Create the dlt_files and messages tables."""
        self.cursor.execute("""
            CREATE TABLE IF NOT EXISTS dlt_files (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                file_path TEXT NOT NULL UNIQUE,
                file_name TEXT NOT NULL,
                file_size_bytes INTEGER NOT NULL,
                sha256 TEXT,
                md5 TEXT,
                total_messages INTEGER DEFAULT 0,
                valid_messages INTEGER DEFAULT 0,
                corrupt_messages INTEGER DEFAULT 0,
                skipped_bytes INTEGER DEFAULT 0,
                first_time_epoch INTEGER,
                last_time_epoch INTEGER,
                first_timestamp_utc TEXT,
                last_timestamp_utc TEXT,
                unique_ecus TEXT,
                unique_apids TEXT,
                unique_ctids TEXT,
                import_time_utc TEXT NOT NULL
            );
        """)

        self.cursor.execute("""
            CREATE TABLE IF NOT EXISTS messages (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                file_id INTEGER,
                source_file TEXT NOT NULL,
                file_offset INTEGER NOT NULL,
                msg_length INTEGER NOT NULL,
                msg_index INTEGER NOT NULL,
                time_utc TEXT,
                time_dlt TEXT,
                epoch_seconds INTEGER,
                microseconds INTEGER,
                ecu_timestamp_s REAL,
                counter INTEGER,
                ecu_id TEXT,
                apid TEXT,
                ctid TEXT,
                session_id INTEGER,
                type TEXT,
                subtype TEXT,
                mode TEXT,
                num_args INTEGER,
                payload TEXT,
                payload_ascii TEXT,
                msg_id TEXT,
                FOREIGN KEY(file_id) REFERENCES dlt_files(id)
            );
        """)
        self.conn.commit()

    def create_indexes_and_views(self) -> None:
        """Build indexes and analytical views (invoked after bulk data insertion)."""
        indexes = [
            "CREATE INDEX IF NOT EXISTS idx_messages_time_utc ON messages(time_utc);",
            "CREATE INDEX IF NOT EXISTS idx_messages_epoch ON messages(epoch_seconds);",
            "CREATE INDEX IF NOT EXISTS idx_messages_date_lookup ON messages(epoch_seconds, ecu_id, subtype);",
            "CREATE INDEX IF NOT EXISTS idx_messages_ecu ON messages(ecu_id);",
            "CREATE INDEX IF NOT EXISTS idx_messages_apid ON messages(apid);",
            "CREATE INDEX IF NOT EXISTS idx_messages_ctid ON messages(ctid);",
            "CREATE INDEX IF NOT EXISTS idx_messages_subtype ON messages(subtype);",
            "CREATE INDEX IF NOT EXISTS idx_messages_source ON messages(source_file);",
            "CREATE INDEX IF NOT EXISTS idx_messages_file_id ON messages(file_id);",
            "CREATE INDEX IF NOT EXISTS idx_messages_payload_ascii ON messages(payload_ascii);",
        ]
        for idx in indexes:
            self.cursor.execute(idx)

        views = [
            """
            CREATE VIEW IF NOT EXISTS v_daily_summary AS
            SELECT
                substr(time_utc, 1, 10) AS log_date,
                ecu_id,
                COUNT(*) AS total_messages,
                SUM(CASE WHEN subtype = 'fatal' THEN 1 ELSE 0 END) AS fatal_count,
                SUM(CASE WHEN subtype = 'error' THEN 1 ELSE 0 END) AS error_count,
                SUM(CASE WHEN subtype = 'warn' THEN 1 ELSE 0 END) AS warn_count,
                MIN(time_utc) AS first_log_time,
                MAX(time_utc) AS last_log_time
            FROM messages
            WHERE time_utc IS NOT NULL AND time_utc != ''
            GROUP BY substr(time_utc, 1, 10), ecu_id;
            """,
            """
            CREATE VIEW IF NOT EXISTS v_critical_faults AS
            SELECT
                id,
                time_utc,
                ecu_id,
                apid,
                ctid,
                subtype,
                payload,
                source_file,
                file_offset
            FROM messages
            WHERE subtype IN ('fatal', 'error');
            """,
        ]
        for view in views:
            self.cursor.execute(view)

        self.conn.commit()

    def record_file(
        self,
        file_path: Path,
        scan_result: Optional[FileScanResult] = None,
    ) -> int:
        """Record or update a DLT file in the dlt_files audit table, returning file_id."""
        now_utc = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M:%SZ")
        str_path = str(file_path.resolve())

        # Check if already present
        self.cursor.execute("SELECT id FROM dlt_files WHERE file_path = ?;", (str_path,))
        row = self.cursor.fetchone()

        if scan_result:
            first_time_str = ""
            last_time_str = ""
            if scan_result.first_time_epoch:
                first_time_str = datetime.fromtimestamp(
                    scan_result.first_time_epoch, tz=timezone.utc
                ).strftime("%Y-%m-%d %H:%M:%SZ")
            if scan_result.last_time_epoch:
                last_time_str = datetime.fromtimestamp(
                    scan_result.last_time_epoch, tz=timezone.utc
                ).strftime("%Y-%m-%d %H:%M:%SZ")

            ecus_str = ",".join(sorted(list(scan_result.unique_ecus)))
            apids_str = ",".join(sorted(list(scan_result.unique_apids)))
            ctids_str = ",".join(sorted(list(scan_result.unique_ctids)))

            if row:
                file_id = row[0]
                self.cursor.execute(
                    """
                    UPDATE dlt_files SET
                        file_name = ?,
                        file_size_bytes = ?,
                        sha256 = ?,
                        md5 = ?,
                        total_messages = ?,
                        valid_messages = ?,
                        corrupt_messages = ?,
                        skipped_bytes = ?,
                        first_time_epoch = ?,
                        last_time_epoch = ?,
                        first_timestamp_utc = ?,
                        last_timestamp_utc = ?,
                        unique_ecus = ?,
                        unique_apids = ?,
                        unique_ctids = ?,
                        import_time_utc = ?
                    WHERE id = ?;
                    """,
                    (
                        file_path.name,
                        scan_result.file_size_bytes,
                        scan_result.sha256_hash,
                        scan_result.md5_hash,
                        scan_result.total_messages,
                        scan_result.valid_messages,
                        scan_result.corrupt_messages,
                        scan_result.skipped_bytes,
                        scan_result.first_time_epoch,
                        scan_result.last_time_epoch,
                        first_time_str,
                        last_time_str,
                        ecus_str,
                        apids_str,
                        ctids_str,
                        now_utc,
                        file_id,
                    ),
                )
            else:
                self.cursor.execute(
                    """
                    INSERT INTO dlt_files (
                        file_path, file_name, file_size_bytes, sha256, md5,
                        total_messages, valid_messages, corrupt_messages, skipped_bytes,
                        first_time_epoch, last_time_epoch, first_timestamp_utc, last_timestamp_utc,
                        unique_ecus, unique_apids, unique_ctids, import_time_utc
                    ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
                    """,
                    (
                        str_path,
                        file_path.name,
                        scan_result.file_size_bytes,
                        scan_result.sha256_hash,
                        scan_result.md5_hash,
                        scan_result.total_messages,
                        scan_result.valid_messages,
                        scan_result.corrupt_messages,
                        scan_result.skipped_bytes,
                        scan_result.first_time_epoch,
                        scan_result.last_time_epoch,
                        first_time_str,
                        last_time_str,
                        ecus_str,
                        apids_str,
                        ctids_str,
                        now_utc,
                    ),
                )
                file_id = self.cursor.lastrowid
        else:
            # File start placeholder
            file_size = file_path.stat().st_size if file_path.exists() else 0
            if row:
                file_id = row[0]
            else:
                self.cursor.execute(
                    """
                    INSERT INTO dlt_files (
                        file_path, file_name, file_size_bytes, import_time_utc
                    ) VALUES (?, ?, ?, ?);
                    """,
                    (str_path, file_path.name, file_size, now_utc),
                )
                file_id = self.cursor.lastrowid

        self.conn.commit()
        return file_id

    def insert_message(self, msg: DltMessage, file_id: int) -> None:
        """Buffer a single message for batch insertion."""
        iso_time = msg.get_iso_time_string(self.use_utc, self.tz_offset_seconds)
        dlt_time = msg.get_time_string(self.use_utc, self.tz_offset_seconds)
        clean_payload = msg.get_payload_clean(replace_commas=self.replace_commas)
        ascii_payload = decode_hex_payload(clean_payload)
        ecu_uptime = round(msg.timestamp / 10000.0, 4) if msg.timestamp else 0.0
        msg_id_str = f"0x{msg.message_id:08X}" if msg.message_id is not None else ""

        row_tuple = (
            file_id,
            msg.source_file,
            msg.file_offset,
            msg.msg_length,
            msg.index,
            iso_time,
            dlt_time,
            msg.time_seconds,
            msg.microseconds,
            ecu_uptime,
            msg.message_counter,
            msg.ecu_id,
            msg.apid,
            msg.ctid,
            msg.session_id,
            msg.type,
            msg.subtype,
            msg.mode,
            msg.number_of_arguments,
            clean_payload,
            ascii_payload,
            msg_id_str,
        )
        self._buffer.append(row_tuple)

        if len(self._buffer) >= self.batch_size:
            self.flush()

    def insert_messages_batch(
        self,
        messages: Iterable[DltMessage],
        file_id: int,
        msg_filter: Optional[DltFilter] = None,
    ) -> int:
        """Stream and batch insert an iterable of messages, returning inserted count."""
        count = 0
        for msg in messages:
            if msg_filter is None or msg_filter.matches(msg):
                self.insert_message(msg, file_id)
                count += 1
        return count

    def flush(self) -> None:
        """Execute batch insert of buffered rows and commit transaction."""
        if not self._buffer:
            return

        insert_sql = """
            INSERT INTO messages (
                file_id, source_file, file_offset, msg_length, msg_index,
                time_utc, time_dlt, epoch_seconds, microseconds, ecu_timestamp_s,
                counter, ecu_id, apid, ctid, session_id,
                type, subtype, mode, num_args, payload, payload_ascii, msg_id
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
        """
        self.cursor.executemany(insert_sql, self._buffer)
        self.conn.commit()
        self._total_rows_inserted += len(self._buffer)
        self._buffer.clear()

    def update_payload_ascii_in_place(
        self,
        batch_size: int = DEFAULT_BATCH_SIZE,
        workers: Optional[int] = None,
        cpu_percent: Optional[float] = None,
        ram_cache_mb: int = 128,
        quiet: bool = False,
    ) -> int:
        """Perform an in-place batch transaction on the database to decode all hex payloads.

        Decodes space-separated hex byte strings into ASCII/UTF-8 text and stores them
        directly in the indexed 'payload_ascii' column.

        Performance Optimizations:
          * Multi-core multiprocessing pool scaled to CPU percentage or worker count.
          * Configurable SQLite memory cache and 2GB memory-mapped I/O.
          * Drops B-tree index during updates and rebuilds sequentially in a single pass.
        """
        import sys
        self.flush()

        # Check if payload_ascii column already exists; add if missing
        self.cursor.execute("PRAGMA table_info(messages);")
        columns = [row[1] for row in self.cursor.fetchall()]
        if "payload_ascii" not in columns:
            if not quiet:
                sys.stderr.write("[*] Adding column 'payload_ascii' to messages table...\n")
            self.cursor.execute("ALTER TABLE messages ADD COLUMN payload_ascii TEXT;")
            self.conn.commit()

        # Drop index during bulk updates to eliminate B-tree rebalance overhead on every batch
        self.cursor.execute("DROP INDEX IF EXISTS idx_messages_payload_ascii;")
        self.conn.commit()

        # Configure high-performance PRAGMAs for in-place batch updates
        self.cursor.execute(f"PRAGMA cache_size = -{max(16, ram_cache_mb) * 1000};")
        self.cursor.execute("PRAGMA synchronous = OFF;")
        self.cursor.execute("PRAGMA temp_store = MEMORY;")
        try:
            self.cursor.execute("PRAGMA mmap_size = 2147483648;")
        except sqlite3.OperationalError:
            pass

        self.cursor.execute("SELECT COUNT(*) FROM messages;")
        total_count = self.cursor.fetchone()[0]

        num_workers = resolve_workers(workers=workers, cpu_percent=cpu_percent)
        effective_batch = max(500, batch_size)

        if not quiet:
            sys.stderr.write(
                f"[*] Processing {total_count:,d} messages in batches of {effective_batch:,d}\n"
                f"    Workers: {num_workers} processes | RAM cache: {ram_cache_mb}MB\n"
            )

        read_cur = self.conn.cursor()
        read_cur.execute("SELECT id, payload FROM messages;")

        updated_count = 0
        hex_converted_count = 0

        if num_workers > 1:
            with ProcessPoolExecutor(max_workers=num_workers) as executor:
                while True:
                    rows = read_cur.fetchmany(effective_batch)
                    if not rows:
                        break

                    # Convert sqlite3.Row objects to standard tuples for pickling across worker processes
                    plain_rows = [(r[0], r[1]) for r in rows]
                    chunk_size = max(250, len(plain_rows) // num_workers)
                    sub_chunks = [plain_rows[i : i + chunk_size] for i in range(0, len(plain_rows), chunk_size)]

                    update_tuples: List[Tuple[str, int]] = []
                    for worker_res in executor.map(_worker_decode_batch, sub_chunks):
                        for decoded, row_id, is_hex in worker_res:
                            if is_hex:
                                hex_converted_count += 1
                            update_tuples.append((decoded, row_id))

                    self.cursor.executemany("UPDATE messages SET payload_ascii = ? WHERE id = ?;", update_tuples)
                    self.conn.commit()
                    updated_count += len(update_tuples)

                    if not quiet and total_count > 0:
                        pct = (updated_count / total_count) * 100
                        sys.stderr.write(f"\r    [+] Updated {updated_count:,d} / {total_count:,d} rows ({pct:.1f}%)...")
                        sys.stderr.flush()
        else:
            # Single-process fallback (workers=1)
            batch: List[Tuple[str, int]] = []
            while True:
                rows = read_cur.fetchmany(effective_batch)
                if not rows:
                    break

                for row_id, payload in rows:
                    decoded = decode_hex_payload(payload)
                    if decoded != payload:
                        hex_converted_count += 1
                    batch.append((decoded, row_id))

                self.cursor.executemany("UPDATE messages SET payload_ascii = ? WHERE id = ?;", batch)
                self.conn.commit()
                updated_count += len(batch)
                batch.clear()

                if not quiet and total_count > 0:
                    pct = (updated_count / total_count) * 100
                    sys.stderr.write(f"\r    [+] Updated {updated_count:,d} / {total_count:,d} rows ({pct:.1f}%)...")
                    sys.stderr.flush()

        read_cur.close()

        if not quiet:
            sys.stderr.write("\n[*] Rebuilding index idx_messages_payload_ascii...\n")
            sys.stderr.flush()

        self.cursor.execute("CREATE INDEX IF NOT EXISTS idx_messages_payload_ascii ON messages(payload_ascii);")
        try:
            self.cursor.execute("PRAGMA optimize;")
            self.cursor.execute("PRAGMA wal_checkpoint(TRUNCATE);")
            self.conn.commit()
        except sqlite3.OperationalError:
            pass

        if not quiet:
            sys.stderr.write(
                f"[+] Database enrichment complete:\n"
                f"    Total rows updated:            {updated_count:,d}\n"
                f"    Hex payloads decoded to ASCII:  {hex_converted_count:,d}\n"
            )
        return updated_count

    def close(self, build_indexes: bool = True) -> int:
        """Flush remaining buffer, construct indexes and views, optimize, and close."""
        self.flush()
        if build_indexes:
            self.create_indexes_and_views()

        # Run SQLite cost-based query optimizer and checkpoint WAL
        try:
            self.cursor.execute("PRAGMA optimize;")
            self.cursor.execute("PRAGMA wal_checkpoint(TRUNCATE);")
            self.conn.commit()
        except sqlite3.OperationalError:
            pass

        self.cursor.close()
        self.conn.close()
        return self._total_rows_inserted

    # Query Helper Methods for Large-Scale Analysis

    def query(
        self,
        sql: str,
        params: Optional[Iterable[Any]] = None,
    ) -> List[Dict[str, Any]]:
        """Execute an arbitrary SQL query against the database and return dictionary rows."""
        self.flush()
        if params is None:
            self.cursor.execute(sql)
        else:
            self.cursor.execute(sql, tuple(params))

        rows = self.cursor.fetchall()
        return [dict(r) for r in rows]

    def get_date_summary(self) -> List[Dict[str, Any]]:
        """Query summary statistics grouped by date (YYYY-MM-DD) across all imported DLT messages."""
        sql = """
            SELECT
                substr(time_utc, 1, 10) AS log_date,
                COUNT(*) AS total_messages,
                COUNT(DISTINCT ecu_id) AS distinct_ecus,
                SUM(CASE WHEN subtype = 'fatal' THEN 1 ELSE 0 END) AS fatal_count,
                SUM(CASE WHEN subtype = 'error' THEN 1 ELSE 0 END) AS error_count,
                SUM(CASE WHEN subtype = 'warn' THEN 1 ELSE 0 END) AS warn_count,
                MIN(time_utc) AS start_time,
                MAX(time_utc) AS end_time
            FROM messages
            WHERE time_utc IS NOT NULL AND time_utc != ''
            GROUP BY substr(time_utc, 1, 10)
            ORDER BY log_date ASC;
        """
        return self.query(sql)

    def export_query_to_csv(
        self,
        sql: str,
        output_path: Union[str, Path],
        params: Optional[Iterable[Any]] = None,
        delimiter: str = ",",
        quote_all: bool = True,
    ) -> int:
        """Execute a query and stream results into a CSV file."""
        self.flush()
        output_file = Path(output_path)
        output_file.parent.mkdir(parents=True, exist_ok=True)

        if params is None:
            self.cursor.execute(sql)
        else:
            self.cursor.execute(sql, tuple(params))

        quoting = csv.QUOTE_ALL if quote_all else csv.QUOTE_MINIMAL
        row_count = 0

        with open(output_file, "w", encoding="utf-8-sig", newline="") as f:
            writer = csv.writer(f, delimiter=delimiter, quoting=quoting, lineterminator="\n")
            # Write column header names from cursor description
            headers = [desc[0] for desc in self.cursor.description]
            writer.writerow(headers)

            while True:
                chunk = self.cursor.fetchmany(10000)
                if not chunk:
                    break
                for r in chunk:
                    writer.writerow(list(r))
                    row_count += 1

        return row_count

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

"""Scanner and streaming reader for DLT files and directories.

Handles streaming file reads, chunk buffering, resynchronization upon
corrupted frames, cryptographic hashing (SHA-256, MD5) for forensic
traceability, and folder-level batch processing.
"""

import hashlib
import os
import re
import time
from pathlib import Path
from typing import Generator, Iterable, List, Optional, Tuple

from .constants import (
    DLT_STORAGE_HEADER_PATTERN_V1,
    DLT_STORAGE_HEADER_PATTERN_V2,
    DLT_SERIAL_HEADER_PATTERN,
)
from .models import DltMessage, FileScanResult
from .parser import parse_dlt_message


def natural_sort_key(s: str) -> list:
    """Key for natural alphanumeric sorting (e.g., 'file2' before 'file10')."""
    return [int(text) if text.isdigit() else text.lower() for text in re.split(r"(\d+)", str(s))]


def compute_file_hashes(file_path: Path, chunk_size: int = 1024 * 1024) -> Tuple[str, str, int]:
    """Compute SHA-256 and MD5 hashes and return (sha256_hex, md5_hex, file_size)."""
    sha256 = hashlib.sha256()
    md5 = hashlib.md5()
    total_size = 0

    with open(file_path, "rb") as f:
        while True:
            chunk = f.read(chunk_size)
            if not chunk:
                break
            sha256.update(chunk)
            md5.update(chunk)
            total_size += len(chunk)

    return sha256.hexdigest(), md5.hexdigest(), total_size


class DltFileScanner:
    """Streams and parses DLT messages from a file with fault-tolerant resynchronization."""

    CHUNK_SIZE = 256 * 1024  # 256 KB read chunks
    MAX_MESSAGE_SIZE = 1024 * 1024  # 1 MB maximum expected DLT message size

    def __init__(self, file_path: Path, compute_hashes: bool = True):
        self.file_path = Path(file_path)
        self.compute_hashes = compute_hashes
        self.result = FileScanResult(file_path=str(self.file_path))

    def scan(self) -> Generator[DltMessage, None, FileScanResult]:
        """Generator that yields valid DltMessage instances and returns FileScanResult."""
        start_time = time.time()

        if not self.file_path.exists():
            raise FileNotFoundError(f"File not found: {self.file_path}")

        if self.compute_hashes:
            sha256_val, md5_val, file_size = compute_file_hashes(self.file_path)
            self.result.sha256_hash = sha256_val
            self.result.md5_hash = md5_val
            self.result.file_size_bytes = file_size
        else:
            self.result.file_size_bytes = self.file_path.stat().st_size

        buffer = bytearray()
        stream_offset = 0  # Absolute file byte offset of buffer[0]
        msg_index = 0

        with open(self.file_path, "rb") as f:
            while True:
                # Read next chunk if buffer needs data
                if len(buffer) < self.MAX_MESSAGE_SIZE:
                    chunk = f.read(self.CHUNK_SIZE)
                    if chunk:
                        buffer.extend(chunk)

                if not buffer:
                    break  # Reached end of file

                # Try parsing message at buffer start (offset 0 in buffer)
                parse_result = parse_dlt_message(buffer, 0)

                if parse_result is not None:
                    msg, consumed = parse_result

                    # Populate message provenance metadata
                    msg.index = msg_index
                    msg.source_file = self.file_path.name
                    msg.file_offset = stream_offset

                    # Track file-level statistics
                    msg_index += 1
                    self.result.total_messages += 1
                    self.result.valid_messages += 1

                    if msg.time_seconds > 0:
                        if (
                            self.result.first_time_epoch is None
                            or msg.time_seconds < self.result.first_time_epoch
                        ):
                            self.result.first_time_epoch = msg.time_seconds
                        if (
                            self.result.last_time_epoch is None
                            or msg.time_seconds > self.result.last_time_epoch
                        ):
                            self.result.last_time_epoch = msg.time_seconds

                    if msg.ecu_id:
                        self.result.unique_ecus.add(msg.ecu_id)
                    if msg.apid:
                        self.result.unique_apids.add(msg.apid)
                    if msg.ctid:
                        self.result.unique_ctids.add(msg.ctid)

                    yield msg

                    # Advance buffer and stream offset
                    del buffer[:consumed]
                    stream_offset += consumed

                else:
                    # Parse failed or message is incomplete.
                    # Check if more data can be read from file before treating as corruption.
                    if len(buffer) < self.MAX_MESSAGE_SIZE and not f.closed:
                        more_data = f.read(self.CHUNK_SIZE)
                        if more_data:
                            buffer.extend(more_data)
                            # Retry parsing with expanded buffer
                            continue

                    # If at EOF or buffer cannot expand further, resynchronize to next header
                    resync_offset = self._find_next_sync_header(buffer, 1)

                    if resync_offset != -1:
                        # Found next sync header; skip corrupted region
                        skipped = resync_offset
                        self.result.skipped_bytes += skipped
                        self.result.corrupt_messages += 1
                        del buffer[:skipped]
                        stream_offset += skipped
                    else:
                        # No more sync headers found in current buffer
                        if len(buffer) >= 4:
                            # Keep last 3 bytes for possible boundary sync match in next chunk
                            tail_keep = min(3, len(buffer))
                            skipped = len(buffer) - tail_keep
                            self.result.skipped_bytes += skipped
                            self.result.corrupt_messages += 1
                            del buffer[:skipped]
                            stream_offset += skipped
                        else:
                            # Remainder at EOF is skipped
                            self.result.skipped_bytes += len(buffer)
                            buffer.clear()

        self.result.elapsed_seconds = time.time() - start_time
        return self.result

    @staticmethod
    def _find_next_sync_header(buf: bytearray, start_pos: int = 1) -> int:
        """Find the byte offset of the next DLT header pattern in the buffer."""
        patterns = (
            DLT_STORAGE_HEADER_PATTERN_V1,
            DLT_STORAGE_HEADER_PATTERN_V2,
            DLT_SERIAL_HEADER_PATTERN,
        )

        earliest = -1
        for p in patterns:
            pos = buf.find(p, start_pos)
            if pos != -1:
                if earliest == -1 or pos < earliest:
                    earliest = pos

        return earliest


def discover_dlt_files(
    targets: Iterable[Path], recursive: bool = False
) -> List[Path]:
    """Discover all .dlt files from a list of files and directories."""
    discovered: List[Path] = []

    for target in targets:
        p = Path(target)
        if not p.exists():
            continue

        if p.is_file():
            if p.suffix.lower() == ".dlt" or True:  # If explicitly given, include it
                discovered.append(p.resolve())
        elif p.is_dir():
            pattern = "**/*.dlt" if recursive else "*.dlt"
            # Case-insensitive matching for .dlt extension
            found = [f.resolve() for f in p.glob(pattern) if f.is_file()]
            discovered.extend(found)

    # De-duplicate while preserving natural sort order (by filename, then directory)
    unique_paths = list(dict.fromkeys(discovered))
    unique_paths.sort(key=lambda x: (natural_sort_key(x.name), natural_sort_key(str(x.parent))))
    return unique_paths

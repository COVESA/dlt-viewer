# -*- coding: utf-8 -*-
#
# This file is part of COVESA Project Dlt Viewer.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.global/.

"""Data models representing DLT messages, headers, arguments, and scan results.
"""

from dataclasses import dataclass, field
from datetime import datetime, timezone, timedelta
from typing import Any, Dict, List, Optional, Set
from pathlib import Path

from .constants import (
    DEFAULT_COLUMNS,
    FORENSIC_COLUMNS,
    DLT_MESSAGE_TYPE_STRINGS,
    DLT_LOG_SUBTYPE_STRINGS,
    DLT_TRACE_SUBTYPE_STRINGS,
    DLT_NW_TRACE_SUBTYPE_STRINGS,
    DLT_CONTROL_SUBTYPE_STRINGS,
    DLT_SERVICE_ID_STRINGS,
    DLT_CONTROL_RETURN_TYPE_STRINGS,
    DLT_TYPE_LOG,
    DLT_TYPE_APP_TRACE,
    DLT_TYPE_NW_TRACE,
    DLT_TYPE_CONTROL,
)


@dataclass
class DltArgument:
    """Represents a single decoded argument from a verbose DLT message payload."""

    type_info: int
    type_name: str
    name: str = ""
    unit: str = ""
    data: bytes = b""
    value: Any = None
    string_repr: str = ""

    def as_string(self) -> str:
        """Return the formatted string representation of this argument."""
        if self.string_repr:
            return self.string_repr
        if self.value is not None:
            if isinstance(self.value, bool):
                return "true" if self.value else "false"
            return str(self.value)
        return "?"


@dataclass
class DltStorageHeader:
    """Represents a DLT storage header prepended to messages in file storage."""

    version: int = 1
    seconds: int = 0
    microseconds: int = 0
    ecu: str = ""
    header_length: int = 16


@dataclass
class DltStandardHeader:
    """Represents the DLT standard header found in every message."""

    htyp: int = 0
    mcnt: int = 0
    length: int = 0  # Total length of message excluding storage header
    version: int = 1
    use_extended_header: bool = False
    is_big_endian: bool = False
    with_ecu_id: bool = False
    with_session_id: bool = False
    with_timestamp: bool = False
    ecu_id: str = ""
    session_id: int = 0
    timestamp: int = 0  # In 0.1 ms increments since ECU startup
    header_length: int = 4  # Includes extra fields


@dataclass
class DltExtendedHeader:
    """Represents the DLT extended header."""

    msin: int = 0
    is_verbose: bool = False
    message_type: int = 0
    message_subtype: int = 0
    number_of_arguments: int = 0
    apid: str = ""
    ctid: str = ""
    header_length: int = 10


@dataclass
class DltMessage:
    """Represents a fully or partially parsed DLT message with forensic metadata."""

    index: int = 0
    source_file: str = ""
    file_offset: int = 0
    msg_length: int = 0
    version: int = 1

    # Timestamps
    time_seconds: int = 0
    microseconds: int = 0
    timestamp: int = 0  # ECU uptime in 0.1 ms units (tmsp / 10000 = seconds)

    # Identifiers
    message_counter: int = 0
    ecu_id: str = ""
    apid: str = ""
    ctid: str = ""
    session_id: int = 0

    # Type & Mode
    type: str = ""        # log, app_trace, nw_trace, control
    subtype: str = ""     # info, warn, error, fatal, etc.
    mode: str = "verbose" # verbose, non-verbose
    number_of_arguments: int = 0

    # Payload
    arguments: List[DltArgument] = field(default_factory=list)
    payload_bytes: bytes = b""
    payload_text: str = ""
    message_id: Optional[int] = None

    # Control Message details
    ctrl_service_id: Optional[int] = None
    ctrl_service_name: str = ""
    ctrl_return_type: str = ""

    def get_time_string(
        self, use_utc: bool = True, tz_offset_seconds: int = 0
    ) -> str:
        """Format the storage timestamp as 'YYYY/MM/DD HH:MM:SS.uuuuuu'.

        Matches DLT Viewer format: '%Y/%m/%d %H:%M:%S.%06d'.
        """
        if self.time_seconds == 0 and self.microseconds == 0:
            return ""

        try:
            if use_utc:
                dt = datetime.fromtimestamp(self.time_seconds, tz=timezone.utc)
                if tz_offset_seconds != 0:
                    dt = dt + timedelta(seconds=tz_offset_seconds)
            else:
                dt = datetime.fromtimestamp(self.time_seconds)

            base_str = dt.strftime("%Y/%m/%d %H:%M:%S")
            return f"{base_str}.{self.microseconds:06d}"
        except (ValueError, OSError, OverflowError):
            return f"{self.time_seconds}.{self.microseconds:06d}"

    def get_iso_time_string(
        self, use_utc: bool = True, tz_offset_seconds: int = 0
    ) -> str:
        """Format the storage timestamp as ISO-8601 'YYYY-MM-DD HH:MM:SS.uuuuuu'.

        Enables native SQLite date(), time(), datetime(), and strftime() functions.
        """
        if self.time_seconds == 0 and self.microseconds == 0:
            return ""

        try:
            if use_utc:
                dt = datetime.fromtimestamp(self.time_seconds, tz=timezone.utc)
                if tz_offset_seconds != 0:
                    dt = dt + timedelta(seconds=tz_offset_seconds)
            else:
                dt = datetime.fromtimestamp(self.time_seconds)

            base_str = dt.strftime("%Y-%m-%d %H:%M:%S")
            return f"{base_str}.{self.microseconds:06d}"
        except (ValueError, OSError, OverflowError):
            return ""

    def get_timestamp_string(self) -> str:
        """Format the ECU uptime timestamp in 'seconds.fraction' (0.1 ms resolution).

        Matches DLT Viewer format: '%d.%04d' where uptime is divided by 10000.
        """
        if self.timestamp == 0:
            return "0.0000"
        secs = self.timestamp // 10000
        frac = self.timestamp % 10000
        return f"{secs}.{frac:04d}"

    def get_payload_clean(self, replace_commas: Optional[str] = None) -> str:
        """Return payload text with null bytes stripped, whitespace normalized, and optional comma replacement."""
        clean = self.payload_text.replace("\x00", "").strip()
        # Collapse multiple spaces into one space while preserving content
        res = " ".join(clean.split())
        if replace_commas is not None:
            res = res.replace(",", replace_commas)
        return res

    def to_dict(
        self,
        columns: Optional[List[str]] = None,
        use_utc: bool = True,
        tz_offset_seconds: int = 0,
        replace_commas: Optional[str] = None,
    ) -> Dict[str, Any]:
        """Convert message to dictionary mapped to requested CSV column headers."""
        cols = columns or DEFAULT_COLUMNS

        # Pre-computed formatted fields
        formatted_time = self.get_time_string(use_utc, tz_offset_seconds)
        formatted_timestamp = self.get_timestamp_string()
        clean_payload = self.get_payload_clean(replace_commas=replace_commas)

        full_map = {
            "Index": self.index,
            "Time": formatted_time,
            "Timestamp": formatted_timestamp,
            "Count": self.message_counter,
            "Ecuid": self.ecu_id,
            "Apid": self.apid,
            "Ctid": self.ctid,
            "SessionId": self.session_id,
            "Type": self.type,
            "Subtype": self.subtype,
            "Mode": self.mode,
            "#Args": self.number_of_arguments,
            "Payload": clean_payload,
            # Forensic metadata fields
            "SourceFile": self.source_file,
            "FileOffset": self.file_offset,
            "MsgLength": self.msg_length,
            "MsgID": f"0x{self.message_id:08X}" if self.message_id is not None else "",
            "EpochSeconds": self.time_seconds,
            "Microseconds": self.microseconds,
        }

        return {c: full_map.get(c, "") for c in cols}


@dataclass
class FileScanResult:
    """Forensic summary of scanning and parsing a single DLT file."""

    file_path: str = ""
    file_size_bytes: int = 0
    sha256_hash: str = ""
    md5_hash: str = ""
    total_messages: int = 0
    valid_messages: int = 0
    corrupt_messages: int = 0
    skipped_bytes: int = 0
    first_time_epoch: Optional[int] = None
    last_time_epoch: Optional[int] = None
    unique_ecus: Set[str] = field(default_factory=set)
    unique_apids: Set[str] = field(default_factory=set)
    unique_ctids: Set[str] = field(default_factory=set)
    elapsed_seconds: float = 0.0

    def to_dict(self) -> Dict[str, Any]:
        """Serialize scan metrics to dictionary for forensic reports."""
        first_time_str = ""
        last_time_str = ""
        if self.first_time_epoch:
            first_time_str = datetime.fromtimestamp(
                self.first_time_epoch, tz=timezone.utc
            ).strftime("%Y-%m-%d %H:%M:%SZ")
        if self.last_time_epoch:
            last_time_str = datetime.fromtimestamp(
                self.last_time_epoch, tz=timezone.utc
            ).strftime("%Y-%m-%d %H:%M:%SZ")

        return {
            "file_path": self.file_path,
            "file_size_bytes": self.file_size_bytes,
            "sha256": self.sha256_hash,
            "md5": self.md5_hash,
            "total_messages": self.total_messages,
            "valid_messages": self.valid_messages,
            "corrupt_messages": self.corrupt_messages,
            "skipped_bytes": self.skipped_bytes,
            "first_timestamp_utc": first_time_str,
            "last_timestamp_utc": last_time_str,
            "unique_ecus": sorted(list(self.unique_ecus)),
            "unique_apids": sorted(list(self.unique_apids)),
            "unique_ctids": sorted(list(self.unique_ctids)),
            "elapsed_seconds": round(self.elapsed_seconds, 4),
        }

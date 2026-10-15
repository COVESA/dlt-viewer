# -*- coding: utf-8 -*-
#
# This file is part of COVESA Project Dlt Viewer.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.global/.

"""DLT to CSV conversion package for forensic automotive log analysis.
"""

from .constants import DEFAULT_COLUMNS, FORENSIC_COLUMNS
from .exporter import CsvExporter, DltFilter, process_dlt_files
from .models import (
    DltArgument,
    DltExtendedHeader,
    DltMessage,
    DltStandardHeader,
    DltStorageHeader,
    FileScanResult,
)
from .parser import parse_dlt_message
from .scanner import DltFileScanner, discover_dlt_files
from .sqlite_exporter import SqliteExporter, decode_hex_payload

__version__ = "2.0.0"

__all__ = [
    "CsvExporter",
    "DltArgument",
    "DltExtendedHeader",
    "DltFileScanner",
    "DltFilter",
    "DltMessage",
    "DltStandardHeader",
    "DltStorageHeader",
    "FileScanResult",
    "SqliteExporter",
    "decode_hex_payload",
    "discover_dlt_files",
    "parse_dlt_message",
    "process_dlt_files",
    "DEFAULT_COLUMNS",
    "FORENSIC_COLUMNS",
]

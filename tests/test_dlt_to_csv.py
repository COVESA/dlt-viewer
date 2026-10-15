# -*- coding: utf-8 -*-
#
# This file is part of COVESA Project Dlt Viewer.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.global/.

"""Unit and integration test suite for dlt_to_csv forensic parser.
"""

import csv
import json
import os
import sqlite3
import struct
import tempfile
import unittest
from pathlib import Path
from typing import Optional

from dlt_to_csv.sqlite_exporter import SqliteExporter

from dlt_to_csv.cli import main, parse_timestamp_arg
from dlt_to_csv.constants import (
    DEFAULT_COLUMNS,
    FORENSIC_COLUMNS,
    DLT_HTYP_MSBF,
    DLT_HTYP_UEH,
    DLT_HTYP_WEID,
    DLT_HTYP_WSID,
    DLT_HTYP_WTMS,
    DLT_LOG_ERROR,
    DLT_LOG_INFO,
    DLT_LOG_WARN,
    DLT_SCOD_BIN,
    DLT_SCOD_HEX,
    DLT_SCOD_UTF8,
    DLT_STORAGE_HEADER_PATTERN_V1,
    DLT_TYPE_CONTROL,
    DLT_TYPE_INFO_BOOL,
    DLT_TYPE_INFO_FLOA,
    DLT_TYPE_INFO_RAWD,
    DLT_TYPE_INFO_SINT,
    DLT_TYPE_INFO_STRG,
    DLT_TYPE_INFO_UINT,
    DLT_TYPE_INFO_VARI,
    DLT_TYPE_LOG,
    DLT_TYLE_8BIT,
    DLT_TYLE_16BIT,
    DLT_TYLE_32BIT,
    DLT_TYLE_64BIT,
)
from dlt_to_csv.exporter import CsvExporter, DltFilter, process_dlt_files
from dlt_to_csv.models import DltMessage
from dlt_to_csv.parser import parse_dlt_message
from dlt_to_csv.scanner import DltFileScanner, discover_dlt_files


def create_dlt_v1_message_bytes(
    seconds: int = 1609459200,
    microseconds: int = 500000,
    storage_ecu: str = "ECU1",
    mcnt: int = 1,
    htyp_extra: int = DLT_HTYP_UEH | DLT_HTYP_WEID | DLT_HTYP_WSID | DLT_HTYP_WTMS,
    is_big_endian: bool = False,
    std_ecu: Optional[str] = None,
    session_id: int = 42,
    timestamp: int = 123456,
    is_verbose: bool = True,
    msg_type: int = DLT_TYPE_LOG,
    msg_subtype: int = DLT_LOG_INFO,
    apid: str = "APP1",
    ctid: str = "CTX1",
    payload_bytes: bytes = b"",
    noar: int = 1,
) -> bytes:
    """Helper to synthesize a valid binary DLT v1 message with storage header."""
    if std_ecu is None:
        std_ecu = storage_ecu
    # Storage Header: 16 bytes
    # pattern: 4B, seconds: 4B uint32 LE, microseconds: 4B int32 LE, ecu: 4B
    ecu_bytes = storage_ecu.encode("ascii").ljust(4, b"\x00")[:4]
    storage_hdr = struct.pack("<4sIi4s", DLT_STORAGE_HEADER_PATTERN_V1, seconds, microseconds, ecu_bytes)

    # Standard Header
    htyp = (1 << 5) | htyp_extra
    if is_big_endian:
        htyp |= DLT_HTYP_MSBF

    extra_bytes = b""
    if htyp & DLT_HTYP_WEID:
        extra_bytes += std_ecu.encode("ascii").ljust(4, b"\x00")[:4]
    if htyp & DLT_HTYP_WSID:
        extra_bytes += struct.pack(">I", session_id)
    if htyp & DLT_HTYP_WTMS:
        extra_bytes += struct.pack(">I", timestamp)

    # Extended Header
    ext_bytes = b""
    if htyp & DLT_HTYP_UEH:
        msin = 0
        if is_verbose:
            msin |= 0x01
        msin |= (msg_type & 0x07) << 1
        msin |= (msg_subtype & 0x0F) << 4

        apid_b = apid.encode("ascii").ljust(4, b"\x00")[:4]
        ctid_b = ctid.encode("ascii").ljust(4, b"\x00")[:4]
        ext_bytes = struct.pack(">BB4s4s", msin, noar, apid_b, ctid_b)

    # Total message length after standard header
    total_len = 4 + len(extra_bytes) + len(ext_bytes) + len(payload_bytes)
    std_hdr = struct.pack(">BBH", htyp, mcnt, total_len)

    return storage_hdr + std_hdr + extra_bytes + ext_bytes + payload_bytes


class TestDltParser(unittest.TestCase):
    """Test suite for DLT message parsing."""

    def test_parse_dlt_v2_sample_file(self):
        """Verify parsing of the bundled qdlt/tests/testfile.dlt (DLT v2 message)."""
        sample_path = Path("qdlt/tests/testfile.dlt")
        self.assertTrue(sample_path.exists(), "testfile.dlt must exist")

        with open(sample_path, "rb") as f:
            data = f.read()

        res = parse_dlt_message(data, 0)
        self.assertIsNotNone(res)
        msg, consumed = res
        self.assertEqual(consumed, 71)
        self.assertEqual(msg.version, 2)
        self.assertEqual(msg.ecu_id, "ECU1")
        self.assertEqual(msg.apid, "logd")
        self.assertEqual(msg.ctid, "RBUF")
        self.assertEqual(msg.type, "log")
        self.assertEqual(msg.subtype, "info")
        self.assertEqual(msg.mode, "verbose")
        self.assertEqual(msg.payload_text, "Start logging after ECU startup")

    def test_parse_v1_string_argument(self):
        """Verify parsing of DLT v1 verbose string argument (ASCII and UTF-8)."""
        # Build payload: Type info (uint32) + length (uint16) + string bytes
        # DLT_TYPE_INFO_STRG | DLT_SCOD_UTF8
        type_info = DLT_TYPE_INFO_STRG | DLT_SCOD_UTF8
        test_str = "Forensic Log Test Entry"
        str_bytes = test_str.encode("utf-8")
        payload = struct.pack("<IH", type_info, len(str_bytes)) + str_bytes

        raw_msg = create_dlt_v1_message_bytes(payload_bytes=payload, noar=1)
        res = parse_dlt_message(raw_msg, 0)
        self.assertIsNotNone(res)
        msg, consumed = res
        self.assertEqual(consumed, len(raw_msg))
        self.assertEqual(msg.version, 1)
        self.assertEqual(msg.ecu_id, "ECU1")
        self.assertEqual(msg.apid, "APP1")
        self.assertEqual(msg.ctid, "CTX1")
        self.assertEqual(msg.subtype, "info")
        self.assertEqual(msg.payload_text, test_str)
        self.assertEqual(len(msg.arguments), 1)
        self.assertEqual(msg.arguments[0].value, test_str)

    def test_parse_v1_numeric_arguments(self):
        """Verify parsing of signed and unsigned integers, floats, and booleans."""
        # Argument 1: Bool true
        arg1 = struct.pack("<IB", DLT_TYPE_INFO_BOOL, 1)

        # Argument 2: SINT 32-bit: -123456
        arg2 = struct.pack("<Ii", DLT_TYPE_INFO_SINT | DLT_TYLE_32BIT, -123456)

        # Argument 3: UINT 32-bit HEX: 0xDEADBEEF
        arg3 = struct.pack("<II", DLT_TYPE_INFO_UINT | DLT_TYLE_32BIT | DLT_SCOD_HEX, 0xDEADBEEF)

        # Argument 4: Float 64-bit: 3.14159265
        arg4 = struct.pack("<Id", DLT_TYPE_INFO_FLOA | DLT_TYLE_64BIT, 3.14159265)

        payload = arg1 + arg2 + arg3 + arg4
        raw_msg = create_dlt_v1_message_bytes(payload_bytes=payload, noar=4)

        res = parse_dlt_message(raw_msg, 0)
        self.assertIsNotNone(res)
        msg, _ = res
        self.assertEqual(len(msg.arguments), 4)
        self.assertEqual(msg.arguments[0].value, True)
        self.assertEqual(msg.arguments[1].value, -123456)
        self.assertEqual(msg.arguments[2].value, 0xDEADBEEF)
        self.assertEqual(msg.arguments[2].string_repr, "0xDEADBEEF")
        self.assertAlmostEqual(msg.arguments[3].value, 3.14159265, places=5)

    def test_parse_v1_variable_name_and_unit(self):
        """Verify parsing of arguments with VARI flag (name and unit)."""
        # VARI uint16 with name 'Speed' and unit 'km/h'
        type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_16BIT | DLT_TYPE_INFO_VARI
        name_bytes = b"Speed"
        unit_bytes = b"km/h"
        hdr_vari = struct.pack("<IHH", type_info, len(name_bytes), len(unit_bytes))
        data_vari = struct.pack("<H", 120)
        payload = hdr_vari + name_bytes + unit_bytes + data_vari

        raw_msg = create_dlt_v1_message_bytes(payload_bytes=payload, noar=1)
        res = parse_dlt_message(raw_msg, 0)
        self.assertIsNotNone(res)
        msg, _ = res
        self.assertEqual(len(msg.arguments), 1)
        self.assertEqual(msg.arguments[0].name, "Speed")
        self.assertEqual(msg.arguments[0].unit, "km/h")
        self.assertEqual(msg.arguments[0].value, 120)
        self.assertIn("Speed: 120 km/h", msg.payload_text)

    def test_parse_v1_non_verbose_message(self):
        """Verify non-verbose message parsing with message ID and raw hex payload."""
        # Non-verbose message: 4 bytes MsgID + raw bytes (in little endian)
        msg_id = 0x12345678
        raw_payload = struct.pack("<I", msg_id) + b"NON_VERBOSE_DATA"

        raw_msg = create_dlt_v1_message_bytes(
            is_verbose=False,
            payload_bytes=raw_payload,
            noar=0,
        )

        res = parse_dlt_message(raw_msg, 0)
        self.assertIsNotNone(res)
        msg, _ = res
        self.assertEqual(msg.mode, "non-verbose")
        self.assertEqual(msg.message_id, msg_id)
        self.assertIn(f"[{msg_id}]", msg.payload_text)


class TestDltScannerAndResync(unittest.TestCase):
    """Test suite for file scanning, cryptographic hashing, and fault-tolerant carving."""

    def test_corrupted_stream_resynchronization(self):
        """Verify that scanner recovers from corrupt bytes between valid messages."""
        msg1 = create_dlt_v1_message_bytes(mcnt=1, payload_bytes=b"\x00\x02\x00\x00\x04\x00MSG1", noar=1)
        msg2 = create_dlt_v1_message_bytes(mcnt=2, payload_bytes=b"\x00\x02\x00\x00\x04\x00MSG2", noar=1)

        # Corrupted garbage bytes between msg1 and msg2
        garbage = b"\xff\x00\xaa\x55\xde\xad\xbe\xef\x01\x02\x03\x04"
        test_stream = msg1 + garbage + msg2

        with tempfile.NamedTemporaryFile(suffix=".dlt", delete=False) as f:
            f.write(test_stream)
            tmp_path = Path(f.name)

        try:
            scanner = DltFileScanner(tmp_path, compute_hashes=True)
            messages = list(scanner.scan())
            self.assertEqual(len(messages), 2, "Scanner must recover both messages despite corruption")
            self.assertEqual(messages[0].message_counter, 1)
            self.assertEqual(messages[1].message_counter, 2)
            self.assertGreater(scanner.result.skipped_bytes, 0)
            self.assertGreater(scanner.result.corrupt_messages, 0)
            self.assertTrue(len(scanner.result.sha256_hash) == 64)
        finally:
            tmp_path.unlink()

    def test_discover_dlt_files(self):
        """Verify file discovery across directories and subdirectories."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            dir_path = Path(tmp_dir)
            sub_dir = dir_path / "sub"
            sub_dir.mkdir()

            f1 = dir_path / "log_1.dlt"
            f2 = dir_path / "log_10.dlt"
            f3 = sub_dir / "log_2.dlt"
            f4 = dir_path / "ignored.txt"

            for f in (f1, f2, f3, f4):
                f.write_bytes(b"TEST")

            # Non-recursive
            found_non_rec = discover_dlt_files([dir_path], recursive=False)
            self.assertEqual(len(found_non_rec), 2)
            self.assertEqual(found_non_rec[0].name, "log_1.dlt")
            self.assertEqual(found_non_rec[1].name, "log_10.dlt")

            # Recursive (with natural sorting: log_1, log_2, log_10)
            found_rec = discover_dlt_files([dir_path], recursive=True)
            self.assertEqual(len(found_rec), 3)
            names = [p.name for p in found_rec]
            self.assertEqual(names, ["log_1.dlt", "log_2.dlt", "log_10.dlt"])


class TestCsvExporterAndCli(unittest.TestCase):
    """Test suite for CSV export, filtering, and CLI invocation."""

    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.temp_path = Path(self.temp_dir.name)

        # Generate sample DLT files in temp directory
        self.file1 = self.temp_path / "car_log_1.dlt"
        self.file2 = self.temp_path / "car_log_2.dlt"

        msg1 = create_dlt_v1_message_bytes(
            seconds=1609459200,
            storage_ecu="BODY",
            apid="BCM1",
            msg_subtype=DLT_LOG_INFO,
            payload_bytes=b"\x00\x02\x00\x00\x0b\x00Doors locked",
            noar=1,
        )
        msg2 = create_dlt_v1_message_bytes(
            seconds=1609459210,
            storage_ecu="ENGN",
            apid="ECU1",
            msg_subtype=DLT_LOG_ERROR,
            payload_bytes=b"\x00\x02\x00\x00\x11\x00Overheat detected",
            noar=1,
        )

        self.file1.write_bytes(msg1 + msg2)

        msg3 = create_dlt_v1_message_bytes(
            seconds=1609459220,
            storage_ecu="ENGN",
            apid="ECU1",
            msg_subtype=DLT_LOG_WARN,
            payload_bytes=b"\x00\x02\x00\x00\x11\x00RPM limit reached",
            noar=1,
        )
        self.file2.write_bytes(msg3)

    def tearDown(self):
        try:
            self.temp_dir.cleanup()
        except Exception:
            pass

    def test_individual_csv_export(self):
        """Verify processing a folder produces individual CSV files."""
        out_dir = self.temp_path / "csv_out"
        out_dir.mkdir()

        code = main([str(self.temp_path), "-o", str(out_dir), "-q"])
        self.assertEqual(code, 0)

        csv1 = out_dir / "car_log_1.csv"
        csv2 = out_dir / "car_log_2.csv"
        self.assertTrue(csv1.exists(), "car_log_1.csv must be created")
        self.assertTrue(csv2.exists(), "car_log_2.csv must be created")

        with open(csv1, "r", encoding="utf-8-sig") as f:
            reader = list(csv.reader(f))
            self.assertEqual(reader[0], DEFAULT_COLUMNS)
            self.assertEqual(len(reader), 3)  # Header + 2 messages
            self.assertEqual(reader[1][4], "BODY")
            self.assertEqual(reader[2][4], "ENGN")

    def test_combined_csv_export(self):
        """Verify --combined merges all DLT messages into a single CSV."""
        combined_file = self.temp_path / "merged.csv"

        code = main([str(self.temp_path), "-c", "-o", str(combined_file), "-q"])
        self.assertEqual(code, 0)
        self.assertTrue(combined_file.exists())

        with open(combined_file, "r", encoding="utf-8-sig") as f:
            reader = list(csv.reader(f))
            self.assertEqual(reader[0], DEFAULT_COLUMNS)
            self.assertEqual(len(reader), 4)  # Header + 3 messages total across both files

    def test_forensic_columns_mode(self):
        """Verify --forensic flag adds file offset, source file, and hash traceability."""
        combined_file = self.temp_path / "forensic.csv"

        code = main([str(self.temp_path), "-c", "--forensic", "-o", str(combined_file), "-q"])
        self.assertEqual(code, 0)

        with open(combined_file, "r", encoding="utf-8-sig") as f:
            reader = list(csv.reader(f))
            self.assertEqual(reader[0], FORENSIC_COLUMNS)
            self.assertIn("SourceFile", reader[0])
            self.assertIn("FileOffset", reader[0])
            self.assertEqual(reader[1][13], "car_log_1.dlt")  # SourceFile

    def test_filtering_by_ecu_and_log_level(self):
        """Verify filtering by ECU ID and log level."""
        combined_file = self.temp_path / "filtered.csv"

        # Filter only ENGN and ERROR
        code = main([
            str(self.temp_path),
            "-c",
            "--ecu", "ENGN",
            "--log-level", "error",
            "-o", str(combined_file),
            "-q",
        ])
        self.assertEqual(code, 0)

        with open(combined_file, "r", encoding="utf-8-sig") as f:
            reader = list(csv.reader(f))
            self.assertEqual(len(reader), 2)  # Header + 1 matched message
            self.assertEqual(reader[1][4], "ENGN")
            self.assertEqual(reader[1][9], "error")
            self.assertIn("Overheat", reader[1][12])

    def test_audit_report_generation(self):
        """Verify generation of the cryptographic audit JSON manifest."""
        report_file = self.temp_path / "audit_report.json"

        code = main([
            str(self.temp_path),
            "--report", str(report_file),
            "-q",
        ])
        self.assertEqual(code, 0)
        self.assertTrue(report_file.exists())

        with open(report_file, "r", encoding="utf-8") as f:
            data = json.load(f)
            self.assertIn("tool", data)
            self.assertEqual(data["total_files_processed"], 2)
            self.assertEqual(data["total_messages_parsed"], 3)
            self.assertEqual(len(data["file_details"]), 2)
            self.assertTrue(len(data["file_details"][0]["sha256"]) == 64)

    def test_control_messages(self):
        """Verify parsing of control messages (service IDs, responses)."""
        # Service ID 0x13: get_software_version response (return code 0: ok)
        service_id = 0x13
        return_code = 0x00
        sw_version_str = "SW_VER_4.12.0_BUILD99"
        payload = struct.pack("<IB", service_id, return_code) + sw_version_str.encode("utf-8")

        raw_msg = create_dlt_v1_message_bytes(
            msg_type=DLT_TYPE_CONTROL,
            msg_subtype=0x02,  # DLT_CONTROL_RESPONSE
            payload_bytes=payload,
            noar=0,
        )

        res = parse_dlt_message(raw_msg, 0)
        self.assertIsNotNone(res)
        msg, _ = res
        self.assertEqual(msg.type, "control")
        self.assertEqual(msg.subtype, "response")
        self.assertEqual(msg.ctrl_service_id, 0x13)
        self.assertEqual(msg.ctrl_service_name, "get_software_version")
        self.assertEqual(msg.ctrl_return_type, "ok")
        self.assertIn("SW_VER_4.12.0_BUILD99", msg.payload_text)

    def test_big_endian_payload_parsing(self):
        """Verify parsing when MSBF (Big Endian) is set in standard header."""
        type_info = DLT_TYPE_INFO_UINT | DLT_TYLE_32BIT
        # Big-endian 0x12345678
        payload = struct.pack(">II", type_info, 0x12345678)

        raw_msg = create_dlt_v1_message_bytes(
            is_big_endian=True,
            payload_bytes=payload,
            noar=1,
        )

        res = parse_dlt_message(raw_msg, 0)
        self.assertIsNotNone(res)
        msg, _ = res
        self.assertEqual(len(msg.arguments), 1)
        self.assertEqual(msg.arguments[0].value, 0x12345678)

    def test_unicode_and_csv_escaping(self):
        """Verify that multiline, comma-separated, and Unicode text is correctly escaped in CSV."""
        complex_text = 'Line 1, "quoted value"\nLine 2: 日本語 / Ümlaut / 🚗'
        text_bytes = complex_text.encode("utf-8")
        type_info = DLT_TYPE_INFO_STRG | DLT_SCOD_UTF8
        payload = struct.pack("<IH", type_info, len(text_bytes)) + text_bytes

        raw_msg = create_dlt_v1_message_bytes(payload_bytes=payload, noar=1)
        test_file = self.temp_path / "unicode_test.dlt"
        test_file.write_bytes(raw_msg)

        out_csv = self.temp_path / "unicode_out.csv"
        code = main([str(test_file), "-o", str(out_csv), "-q"])
        self.assertEqual(code, 0)

        with open(out_csv, "r", encoding="utf-8-sig") as f:
            reader = list(csv.reader(f))
            self.assertEqual(len(reader), 2)
            exported_payload = reader[1][12]
            # Since get_payload_clean() normalizes whitespace:
            self.assertIn("日本語", exported_payload)
            self.assertIn("Ümlaut", exported_payload)
            self.assertIn("🚗", exported_payload)
            self.assertIn('"quoted value"', exported_payload)

    def test_custom_columns(self):
        """Verify custom column selection via --columns."""
        out_csv = self.temp_path / "custom_cols.csv"
        code = main([
            str(self.file1),
            "--columns", "Time,Ecuid,Apid,Payload",
            "-o", str(out_csv),
            "-q",
        ])
        self.assertEqual(code, 0)

        with open(out_csv, "r", encoding="utf-8-sig") as f:
            reader = list(csv.reader(f))
            self.assertEqual(reader[0], ["Time", "Ecuid", "Apid", "Payload"])
            self.assertEqual(len(reader[1]), 4)

    def test_strict_rfc4180_quoting_by_default(self):
        """Verify that all CSV fields are strictly quoted by default to protect columns with commas."""
        payload_with_commas = 'Item 1, Item 2, Item 3, extra "quoted" detail'
        text_bytes = payload_with_commas.encode("utf-8")
        type_info = DLT_TYPE_INFO_STRG | DLT_SCOD_UTF8
        payload = struct.pack("<IH", type_info, len(text_bytes)) + text_bytes

        raw_msg = create_dlt_v1_message_bytes(payload_bytes=payload, noar=1)
        test_file = self.temp_path / "commas_test.dlt"
        test_file.write_bytes(raw_msg)

        out_csv = self.temp_path / "commas_default_strict.csv"
        code = main([str(test_file), "-o", str(out_csv), "-q"])
        self.assertEqual(code, 0)

        raw_lines = out_csv.read_text(encoding="utf-8-sig").splitlines()
        # Verify strict RFC 4180 quoting on header and rows
        self.assertTrue(raw_lines[0].startswith('"Index","Time"'))
        self.assertTrue(raw_lines[1].startswith('"0",'))

        with open(out_csv, "r", encoding="utf-8-sig") as f:
            reader = list(csv.reader(f))
            self.assertEqual(len(reader), 2)
            row = reader[1]
            # Standard row must have exactly 13 columns - commas in payload must NOT split the row
            self.assertEqual(len(row), 13)
            self.assertEqual(row[12], 'Item 1, Item 2, Item 3, extra "quoted" detail')

    def test_quote_minimal_opt_out(self):
        """Verify --quote-minimal disables strict quoting for plain fields."""
        out_csv = self.temp_path / "quote_minimal.csv"
        code = main([str(self.file1), "--quote-minimal", "-o", str(out_csv), "-q"])
        self.assertEqual(code, 0)

        raw_lines = out_csv.read_text(encoding="utf-8-sig").splitlines()
        # With quote-minimal, header is Index,Time,... without surrounding quotes
        self.assertTrue(raw_lines[0].startswith("Index,Time"))

    def test_replace_commas_in_payload(self):
        """Verify --replace-commas replaces commas with semicolon or custom character."""
        payload_with_commas = "Speed: 120 km/h, Temp: 85 C, Status: OK"
        text_bytes = payload_with_commas.encode("utf-8")
        type_info = DLT_TYPE_INFO_STRG | DLT_SCOD_UTF8
        payload = struct.pack("<IH", type_info, len(text_bytes)) + text_bytes

        raw_msg = create_dlt_v1_message_bytes(payload_bytes=payload, noar=1)
        test_file = self.temp_path / "replace_commas_test.dlt"
        test_file.write_bytes(raw_msg)

        # 1. Default replacement should be ';' when --replace-commas is given without args
        out_csv_default = self.temp_path / "replace_default.csv"
        code = main([str(test_file), "--replace-commas", "-o", str(out_csv_default), "-q"])
        self.assertEqual(code, 0)

        with open(out_csv_default, "r", encoding="utf-8-sig") as f:
            reader = list(csv.reader(f))
            payload_out = reader[1][12]
            self.assertEqual(payload_out, "Speed: 120 km/h; Temp: 85 C; Status: OK")
            self.assertNotIn(",", payload_out)

        # 2. Custom replacement with pipe '|'
        out_csv_pipe = self.temp_path / "replace_pipe.csv"
        code = main([str(test_file), "--replace-commas", "|", "-o", str(out_csv_pipe), "-q"])
        self.assertEqual(code, 0)

        with open(out_csv_pipe, "r", encoding="utf-8-sig") as f:
            reader = list(csv.reader(f))
            payload_out = reader[1][12]
            self.assertEqual(payload_out, "Speed: 120 km/h| Temp: 85 C| Status: OK")
            self.assertNotIn(",", payload_out)

        # 3. Custom replacement with space ' '
        out_csv_space = self.temp_path / "replace_space.csv"
        code = main([str(test_file), "--replace-commas", " ", "-o", str(out_csv_space), "-q"])
        self.assertEqual(code, 0)

        with open(out_csv_space, "r", encoding="utf-8-sig") as f:
            reader = list(csv.reader(f))
            payload_out = reader[1][12]
            self.assertEqual(payload_out, "Speed: 120 km/h  Temp: 85 C  Status: OK")
            self.assertNotIn(",", payload_out)

    def test_sqlite_exporter_direct(self):
        """Verify SqliteExporter schema creation, batch inserts, indexes, views, and date queries."""
        db_path = self.temp_path / "test_direct.db"
        exporter = SqliteExporter(db_path=db_path, batch_size=2)

        file_id = exporter.record_file(self.file1)
        self.assertEqual(file_id, 1)

        scanner = DltFileScanner(self.file1)
        messages = list(scanner.scan())
        self.assertEqual(len(messages), 2)

        count = exporter.insert_messages_batch(messages, file_id=file_id)
        self.assertEqual(count, 2)
        exporter.record_file(self.file1, scanner.result)
        total_rows = exporter.close(build_indexes=True)
        self.assertEqual(total_rows, 2)

        # Inspect SQLite DB with sqlite3
        conn = sqlite3.connect(str(db_path))
        conn.row_factory = sqlite3.Row
        cur = conn.cursor()

        # Check dlt_files table
        cur.execute("SELECT * FROM dlt_files WHERE id = 1;")
        f_row = cur.fetchone()
        self.assertIsNotNone(f_row)
        self.assertEqual(f_row["file_name"], "car_log_1.dlt")
        self.assertEqual(f_row["valid_messages"], 2)

        # Check messages table
        cur.execute("SELECT * FROM messages ORDER BY id ASC;")
        rows = cur.fetchall()
        self.assertEqual(len(rows), 2)
        # Verify ISO-8601 formatting for time_utc
        self.assertTrue(rows[0]["time_utc"].startswith("2021-01-01"))
        self.assertEqual(rows[0]["ecu_id"], "BODY")
        self.assertEqual(rows[0]["apid"], "BCM1")

        # Test date-based SQL queries
        cur.execute("SELECT COUNT(*) AS c FROM messages WHERE date(time_utc) = '2021-01-01';")
        self.assertEqual(cur.fetchone()["c"], 2)

        # Test analytical views
        cur.execute("SELECT * FROM v_daily_summary ORDER BY ecu_id ASC;")
        v_rows = cur.fetchall()
        self.assertEqual(len(v_rows), 2)  # 1 for BODY, 1 for ENGN
        self.assertEqual(v_rows[0]["log_date"], "2021-01-01")
        self.assertEqual(sum(r["total_messages"] for r in v_rows), 2)

        cur.close()
        conn.close()

    def test_sqlite_export_via_cli(self):
        """Verify CLI exports directly into SQLite with --sqlite and --no-csv."""
        db_path = self.temp_path / "cli_exported.db"
        code = main([
            str(self.temp_path),
            "--sqlite", str(db_path),
            "--no-csv",
            "-q",
        ])
        self.assertEqual(code, 0)
        self.assertTrue(db_path.exists())

        conn = sqlite3.connect(str(db_path))
        cur = conn.cursor()
        cur.execute("SELECT COUNT(*) FROM messages;")
        total_msgs = cur.fetchone()[0]
        # file1 has 2 messages, file2 has 1 message -> total 3 messages
        self.assertEqual(total_msgs, 3)

        cur.execute("SELECT COUNT(*) FROM dlt_files;")
        total_files = cur.fetchone()[0]
        self.assertEqual(total_files, 2)
        cur.close()
        conn.close()

    def test_sqlite_query_and_dates_cli(self):
        """Verify CLI --query, --query-dates, and --query-export on SQLite database."""
        db_path = self.temp_path / "query_test.db"
        # First import all files into DB
        code = main([str(self.temp_path), "--sqlite", str(db_path), "--no-csv", "-q"])
        self.assertEqual(code, 0)

        # 1. Run --query-dates without inputs
        code_dates = main(["--sqlite", str(db_path), "--query-dates", "-q"])
        self.assertEqual(code_dates, 0)

        # 2. Run --query with export to CSV
        query_csv = self.temp_path / "query_output.csv"
        sql = "SELECT time_utc, ecu_id, subtype, payload FROM messages WHERE subtype='error'"
        code_query = main([
            "--sqlite", str(db_path),
            "--query", sql,
            "--query-export", str(query_csv),
            "-q",
        ])
        self.assertEqual(code_query, 0)
        self.assertTrue(query_csv.exists())

        with open(query_csv, "r", encoding="utf-8-sig") as f:
            reader = list(csv.reader(f))
            self.assertEqual(reader[0], ["time_utc", "ecu_id", "subtype", "payload"])
            self.assertEqual(len(reader), 2)  # Header + 1 error row
            self.assertEqual(reader[1][1], "ENGN")
            self.assertEqual(reader[1][2], "error")
            self.assertIn("Overheat", reader[1][3])

    def test_sqlite_date_range_queries(self):
        """Verify date-range filtering, daily grouping, and index behavior across distinct dates."""
        db_path = self.temp_path / "date_ranges.db"
        exporter = SqliteExporter(db_path=db_path)

        # Synthesize messages spanning 3 different days
        # Day 1: 2026-05-01 10:00:00 (epoch 1777629600)
        # Day 2: 2026-05-02 12:00:00 (epoch 1777723200)
        # Day 3: 2026-05-03 14:00:00 (epoch 1777816800)
        epochs = [1777629600, 1777723200, 1777816800]
        test_file = self.temp_path / "multi_day.dlt"
        raw_bytes = b"".join(
            create_dlt_v1_message_bytes(seconds=ep, storage_ecu=f"ECU{i}", payload_bytes=b"\x00\x02\x00\x00\x04\x00Test")
            for i, ep in enumerate(epochs)
        )
        test_file.write_bytes(raw_bytes)

        scanner = DltFileScanner(test_file)
        msgs = list(scanner.scan())
        fid = exporter.record_file(test_file)
        exporter.insert_messages_batch(msgs, file_id=fid)
        exporter.record_file(test_file, scanner.result)
        exporter.close(build_indexes=True)

        conn = sqlite3.connect(str(db_path))
        conn.row_factory = sqlite3.Row
        cur = conn.cursor()

        # 1. Filter by specific date
        cur.execute("SELECT COUNT(*) AS c FROM messages WHERE date(time_utc) = '2026-05-02';")
        self.assertEqual(cur.fetchone()["c"], 1)

        # 2. Filter by date range (BETWEEN)
        cur.execute("SELECT COUNT(*) AS c FROM messages WHERE time_utc BETWEEN '2026-05-01 00:00:00' AND '2026-05-02 23:59:59';")
        self.assertEqual(cur.fetchone()["c"], 2)

        # 3. Filter by epoch seconds
        cur.execute("SELECT COUNT(*) AS c FROM messages WHERE epoch_seconds >= 1777723200;")
        self.assertEqual(cur.fetchone()["c"], 2)

        # 4. Check get_date_summary() via fresh exporter
        query_exp = SqliteExporter(db_path=db_path)
        summary = query_exp.get_date_summary()
        self.assertEqual(len(summary), 3)
        self.assertEqual(summary[0]["log_date"], "2026-05-01")
        self.assertEqual(summary[1]["log_date"], "2026-05-02")
        self.assertEqual(summary[2]["log_date"], "2026-05-03")
        query_exp.close(build_indexes=False)

        cur.close()
        conn.close()

    def test_hex_to_ascii_payload_conversion(self):
        """Verify hex-to-ASCII payload conversion logic matching user requirements."""
        from dlt_to_csv.sqlite_exporter import decode_hex_payload, sanitize_ascii_payload

        # 1. Standard space-separated hex byte sequence
        self.assertEqual(decode_hex_payload("48 65 6c 6c 6f 20 57 6f 72 6c 64"), "Hello World")
        self.assertEqual(decode_hex_payload("48 65 6C 6C 6F"), "Hello")
        self.assertEqual(decode_hex_payload("30 31 32 33 2c 20 4f 4b"), "0123, OK")

        # 2. User's exact case with typo/mixed token 'v2', null bytes, and control characters
        user_input = "11 40 c2 v2 00 c2 b2 00 00 00 c2 9d c2 aa 01 00"
        user_output = decode_hex_payload(user_input)
        # Verify '@' (0x40) and 'v2' are present
        self.assertIn("@", user_output)
        self.assertIn("v2", user_output)
        # Sanity check: verify ZERO null bytes or weird control characters remain
        self.assertNotIn("\x00", user_output)
        self.assertNotIn("\x11", user_output)
        self.assertNotIn("\x01", user_output)
        self.assertNotIn("\x9d", user_output)

        # 3. Hex strings with null bytes and control chars inside
        self.assertEqual(decode_hex_payload("48 65 6c 6c 6f 00 00 57 6f 72 6c 64"), "Hello World")
        self.assertEqual(decode_hex_payload("00 00 00 00"), "")

        # 4. Plain text should be gracefully preserved while removing rogue control codes/null bytes
        self.assertEqual(decode_hex_payload("Doors locked"), "Doors locked")
        self.assertEqual(decode_hex_payload("Overheat detected"), "Overheat detected")
        self.assertEqual(decode_hex_payload("Engine started\x00\x01\x11"), "Engine started")

        # 5. Continuous unspaced hex & colon-separated hex
        self.assertEqual(decode_hex_payload("48656c6c6f20576f726c64"), "Hello World")
        self.assertEqual(decode_hex_payload("48:65:6c:6c:6f"), "Hello")

        # 6. Empty or None values
        self.assertEqual(decode_hex_payload(None), "")
        self.assertEqual(decode_hex_payload(""), "")

    def test_query_payload_ascii_script(self):
        """Verify query_payload_ascii script querying, CSV export, and in-place DB update."""
        import query_payload_ascii

        db_path = self.temp_path / "ascii_test.db"

        # Create DLT message with space-separated hex payload
        hex_payload_text = "45 6e 67 69 6e 65 20 53 74 61 72 74 65 64"  # "Engine Started"
        text_bytes = hex_payload_text.encode("utf-8")
        type_info = DLT_TYPE_INFO_STRG | DLT_SCOD_UTF8
        payload = struct.pack("<IH", type_info, len(text_bytes)) + text_bytes

        raw_msg = create_dlt_v1_message_bytes(payload_bytes=payload, noar=1)
        test_file = self.temp_path / "hex_payload.dlt"
        test_file.write_bytes(raw_msg)

        # Ingest into SQLite
        code = main([str(test_file), "--sqlite", str(db_path), "--no-csv", "-q"])
        self.assertEqual(code, 0)

        # 1. Test terminal query via query_payload_ascii
        exit_code = query_payload_ascii.main([str(db_path), "-q"])
        self.assertEqual(exit_code, 0)

        # 2. Test CSV export with decoded ASCII
        out_csv = self.temp_path / "ascii_out.csv"
        exit_code_csv = query_payload_ascii.main([str(db_path), "-o", str(out_csv), "-q"])
        self.assertEqual(exit_code_csv, 0)
        self.assertTrue(out_csv.exists())

        with open(out_csv, "r", encoding="utf-8-sig") as f:
            reader = list(csv.reader(f))
            self.assertEqual(len(reader), 2)  # Header + 1 row
            # Payload column should have the decoded text
            decoded_val = reader[1][6]
            self.assertEqual(decoded_val, "Engine Started")

        # 3. Test in-place database update (--update-db)
        exit_code_upd = query_payload_ascii.main([str(db_path), "--update-db", "-q"])
        self.assertEqual(exit_code_upd, 0)

        # Verify payload_ascii column in SQLite database
        conn = sqlite3.connect(str(db_path))
        conn.row_factory = sqlite3.Row
        cur = conn.cursor()
        cur.execute("SELECT payload, payload_ascii FROM messages LIMIT 1;")
        row = cur.fetchone()
        self.assertEqual(row["payload"], hex_payload_text)
        self.assertEqual(row["payload_ascii"], "Engine Started")
        cur.close()
        conn.close()

    def test_cli_update_ascii_in_place(self):
        """Verify dlt-to-csv CLI --sqlite <db> --update-ascii performs in-place transactions."""
        db_path = self.temp_path / "cli_inplace_ascii.db"

        # Manually create a minimal SQLite database without payload_ascii column to test schema migration
        conn = sqlite3.connect(str(db_path))
        conn.execute("""
            CREATE TABLE messages (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                payload TEXT
            );
        """)
        conn.execute("INSERT INTO messages (payload) VALUES ('48 65 6c 6c 6f 20 57 6f 72 6c 64');")
        conn.execute("INSERT INTO messages (payload) VALUES ('Plain text log');")
        conn.commit()
        conn.close()

        # Run CLI in-place update
        exit_code = main(["--sqlite", str(db_path), "--update-ascii", "-q"])
        self.assertEqual(exit_code, 0)

        # Inspect DB
        conn = sqlite3.connect(str(db_path))
        conn.row_factory = sqlite3.Row
        cur = conn.cursor()
        cur.execute("SELECT id, payload, payload_ascii FROM messages ORDER BY id ASC;")
        rows = cur.fetchall()
        self.assertEqual(len(rows), 2)
        self.assertEqual(rows[0]["payload"], "48 65 6c 6c 6f 20 57 6f 72 6c 64")
        self.assertEqual(rows[0]["payload_ascii"], "Hello World")
        self.assertEqual(rows[1]["payload"], "Plain text log")
        self.assertEqual(rows[1]["payload_ascii"], "Plain text log")

        # Verify index was created
        cur.execute("PRAGMA index_list('messages');")
        indexes = [idx["name"] for idx in cur.fetchall()]
        self.assertIn("idx_messages_payload_ascii", indexes)

        cur.close()
        conn.close()


if __name__ == "__main__":
    unittest.main()





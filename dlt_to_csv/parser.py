# -*- coding: utf-8 -*-
#
# This file is part of COVESA Project Dlt Viewer.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.global/.

"""Binary parser for DLT (Diagnostic Log and Trace) messages.

Implements byte-accurate parsing for DLT v1 and v2, storage headers,
standard headers, extended headers, and verbose/non-verbose payloads.
Matches COVESA/dlt-viewer C++ decoding logic.
"""

import struct
from typing import Optional, Tuple, List

from .constants import (
    DLT_STORAGE_HEADER_PATTERN_V1,
    DLT_STORAGE_HEADER_PATTERN_V2,
    DLT_SERIAL_HEADER_PATTERN,
    DLT_HTYP_UEH,
    DLT_HTYP_MSBF,
    DLT_HTYP_WEID,
    DLT_HTYP_WSID,
    DLT_HTYP_WTMS,
    DLT_HTYP_VERS,
    DLT_MSIN_VERB,
    DLT_MSIN_MSTP,
    DLT_MSIN_MTIN,
    DLT_MSIN_MSTP_SHIFT,
    DLT_MSIN_MTIN_SHIFT,
    DLT_MESSAGE_TYPE_STRINGS,
    DLT_LOG_SUBTYPE_STRINGS,
    DLT_TRACE_SUBTYPE_STRINGS,
    DLT_NW_TRACE_SUBTYPE_STRINGS,
    DLT_CONTROL_SUBTYPE_STRINGS,
    DLT_TYPE_LOG,
    DLT_TYPE_APP_TRACE,
    DLT_TYPE_NW_TRACE,
    DLT_TYPE_CONTROL,
    DLT_CONTROL_RESPONSE,
    DLT_TYPE_INFO_TYLE,
    DLT_TYPE_INFO_BOOL,
    DLT_TYPE_INFO_SINT,
    DLT_TYPE_INFO_UINT,
    DLT_TYPE_INFO_FLOA,
    DLT_TYPE_INFO_ARAY,
    DLT_TYPE_INFO_STRG,
    DLT_TYPE_INFO_RAWD,
    DLT_TYPE_INFO_VARI,
    DLT_TYPE_INFO_FIXP,
    DLT_TYPE_INFO_TRAI,
    DLT_TYPE_INFO_STRU,
    DLT_TYPE_INFO_SCOD,
    DLT_TYLE_8BIT,
    DLT_TYLE_16BIT,
    DLT_TYLE_32BIT,
    DLT_TYLE_64BIT,
    DLT_TYLE_128BIT,
    DLT_SCOD_ASCII,
    DLT_SCOD_UTF8,
    DLT_SCOD_HEX,
    DLT_SCOD_BIN,
    DLT_SERVICE_ID_STRINGS,
    DLT_CONTROL_RETURN_TYPE_STRINGS,
    DLT_SERVICE_ID_MARKER,
    DLT_SERVICE_ID_GET_SOFTWARE_VERSION,
    DLT_SERVICE_ID_CONNECTION_INFO,
    DLT_SERVICE_ID_TIMEZONE,
)
from .models import (
    DltArgument,
    DltExtendedHeader,
    DltMessage,
    DltStandardHeader,
    DltStorageHeader,
)


def clean_id_string(raw: bytes) -> str:
    """Decode a 4-byte DLT ID (ECU, APID, CTID) to a sanitized ASCII/UTF-8 string."""
    try:
        # Strip trailing null bytes and whitespace
        decoded = raw.decode("latin1", errors="replace").replace("\x00", "").strip()
        # Ensure printable characters
        return "".join(c for c in decoded if c.isprintable())
    except Exception:
        return ""


def parse_storage_header(buf: bytes, offset: int = 0) -> Optional[DltStorageHeader]:
    """Parse DLT storage header if present at the given offset.

    Returns DltStorageHeader if valid, or None if pattern does not match.
    """
    buf_len = len(buf) - offset
    if buf_len < 16:
        return None

    pattern = buf[offset : offset + 4]
    if pattern == DLT_STORAGE_HEADER_PATTERN_V1:
        # DLT v1 Storage Header: pattern (4B), seconds (4B LE uint32), microseconds (4B LE int32), ecu (4B)
        seconds, microseconds = struct.unpack_from("<Ii", buf, offset + 4)
        ecu = clean_id_string(buf[offset + 12 : offset + 16])
        return DltStorageHeader(
            version=1,
            seconds=seconds,
            microseconds=microseconds,
            ecu=ecu,
            header_length=16,
        )
    elif pattern == DLT_STORAGE_HEADER_PATTERN_V2:
        # DLT v2 Storage Header: pattern (4B), nanoseconds (4B), seconds (4B/5B), ecuidlen (1B), ecu
        if buf_len < 14:
            return None
        nanoseconds = struct.unpack_from("<I", buf, offset + 4)[0]
        # In DLT v2: seconds is stored in bytes 8..12
        sec_bytes = buf[offset + 8 : offset + 13]
        seconds = int.from_bytes(sec_bytes, byteorder="little")
        ecu_len = buf[offset + 13]
        total_len = 14 + ecu_len
        if buf_len < total_len:
            return None
        ecu = clean_id_string(buf[offset + 14 : offset + total_len])
        microseconds = nanoseconds // 1000
        return DltStorageHeader(
            version=2,
            seconds=seconds,
            microseconds=microseconds,
            ecu=ecu,
            header_length=total_len,
        )

    return None


def parse_standard_header(
    buf: bytes, offset: int = 0
) -> Optional[Tuple[DltStandardHeader, int]]:
    """Parse DLT standard header and extra parameters starting at offset.

    Returns tuple (DltStandardHeader, bytes_consumed) or None on error.
    """
    buf_len = len(buf) - offset
    if buf_len < 4:
        return None

    htyp, mcnt, length = struct.unpack_from(">BBH", buf, offset)
    version = (htyp & DLT_HTYP_VERS) >> 5
    use_ueh = bool(htyp & DLT_HTYP_UEH)
    is_big_endian = bool(htyp & DLT_HTYP_MSBF)
    with_weid = bool(htyp & DLT_HTYP_WEID)
    with_wsid = bool(htyp & DLT_HTYP_WSID)
    with_wtms = bool(htyp & DLT_HTYP_WTMS)

    consumed = 4
    ecu_id = ""
    session_id = 0
    timestamp = 0

    # Calculate extra fields size
    extra_size = (4 if with_weid else 0) + (4 if with_wsid else 0) + (4 if with_wtms else 0)
    if buf_len < consumed + extra_size:
        return None

    if with_weid:
        ecu_id = clean_id_string(buf[offset + consumed : offset + consumed + 4])
        consumed += 4

    if with_wsid:
        session_id = struct.unpack_from(">I", buf, offset + consumed)[0]
        consumed += 4

    if with_wtms:
        timestamp = struct.unpack_from(">I", buf, offset + consumed)[0]
        consumed += 4

    hdr = DltStandardHeader(
        htyp=htyp,
        mcnt=mcnt,
        length=length,
        version=version,
        use_extended_header=use_ueh,
        is_big_endian=is_big_endian,
        with_ecu_id=with_weid,
        with_session_id=with_wsid,
        with_timestamp=with_wtms,
        ecu_id=ecu_id,
        session_id=session_id,
        timestamp=timestamp,
        header_length=consumed,
    )
    return hdr, consumed


def parse_extended_header(
    buf: bytes, offset: int = 0
) -> Optional[Tuple[DltExtendedHeader, int]]:
    """Parse DLT extended header starting at offset.

    Returns tuple (DltExtendedHeader, bytes_consumed) or None on error.
    """
    buf_len = len(buf) - offset
    if buf_len < 10:
        return None

    msin, noar = struct.unpack_from("BB", buf, offset)
    apid = clean_id_string(buf[offset + 2 : offset + 6])
    ctid = clean_id_string(buf[offset + 6 : offset + 10])

    is_verbose = bool(msin & DLT_MSIN_VERB)
    msg_type = (msin & DLT_MSIN_MSTP) >> DLT_MSIN_MSTP_SHIFT
    msg_subtype = (msin & DLT_MSIN_MTIN) >> DLT_MSIN_MTIN_SHIFT

    hdr = DltExtendedHeader(
        msin=msin,
        is_verbose=is_verbose,
        message_type=msg_type,
        message_subtype=msg_subtype,
        number_of_arguments=noar,
        apid=apid,
        ctid=ctid,
        header_length=10,
    )
    return hdr, 10


def parse_argument(
    payload: bytes, offset: int, is_big_endian: bool
) -> Optional[Tuple[DltArgument, int]]:
    """Parse a single verbose payload argument starting at offset.

    Returns tuple (DltArgument, new_offset) or None on failure.
    """
    payload_len = len(payload)
    if payload_len < offset + 4:
        return None

    endian_str = ">" if is_big_endian else "<"
    dlt_type = struct.unpack_from(f"{endian_str}I", payload, offset)[0]
    curr = offset + 4

    arg_type_name = "Unknown"
    is_string = False
    is_bool = False
    is_sint = False
    is_uint = False
    is_float = False
    is_raw = False
    is_trace = False

    if dlt_type & DLT_TYPE_INFO_STRG:
        is_string = True
        arg_type_name = (
            "Utf8String"
            if (dlt_type & DLT_TYPE_INFO_SCOD) == DLT_SCOD_UTF8
            else "String"
        )
    elif dlt_type & DLT_TYPE_INFO_BOOL:
        is_bool = True
        arg_type_name = "Bool"
    elif dlt_type & DLT_TYPE_INFO_SINT:
        is_sint = True
        arg_type_name = "SignedInteger"
    elif dlt_type & DLT_TYPE_INFO_UINT:
        is_uint = True
        arg_type_name = "UnsignedInteger"
    elif dlt_type & DLT_TYPE_INFO_FLOA:
        is_float = True
        arg_type_name = "Float"
    elif dlt_type & DLT_TYPE_INFO_RAWD:
        is_raw = True
        arg_type_name = "RawData"
    elif dlt_type & DLT_TYPE_INFO_TRAI:
        is_trace = True
        arg_type_name = "TraceInfo"
    else:
        # Unsupported argument type
        return None

    # Length field for variable length types
    length = 0
    if is_string or is_raw or is_trace:
        if payload_len < curr + 2:
            return None
        length = struct.unpack_from(f"{endian_str}H", payload, curr)[0]
        curr += 2

    # Variable info (name and optional unit)
    name = ""
    unit = ""
    if dlt_type & DLT_TYPE_INFO_VARI:
        if payload_len < curr + 2:
            return None
        name_len = struct.unpack_from(f"{endian_str}H", payload, curr)[0]
        curr += 2

        unit_len = 0
        if is_sint or is_uint or is_float:
            if payload_len < curr + 2:
                return None
            unit_len = struct.unpack_from(f"{endian_str}H", payload, curr)[0]
            curr += 2

        if payload_len < curr + name_len:
            return None
        name = payload[curr : curr + name_len].decode("utf-8", errors="replace").replace("\x00", "")
        curr += name_len

        if unit_len > 0:
            if payload_len < curr + unit_len:
                return None
            unit = payload[curr : curr + unit_len].decode("utf-8", errors="replace").replace("\x00", "")
            curr += unit_len

    # Extract argument value
    data = b""
    val = None
    string_repr = ""

    if is_string or is_raw or is_trace:
        if payload_len < curr + length:
            return None
        data = payload[curr : curr + length]
        curr += length

        if is_string:
            # Decode string; DLT strings may include a null terminator
            text = data.decode("utf-8", errors="replace")
            if text.endswith("\x00"):
                text = text[:-1]
            val = text
            string_repr = text
        elif is_raw:
            val = data
            string_repr = " ".join(f"{b:02x}" for b in data)
        else:
            val = data
            string_repr = "?"

    elif is_bool:
        if payload_len < curr + 1:
            return None
        data = payload[curr : curr + 1]
        curr += 1
        b_val = bool(data[0])
        val = b_val
        string_repr = "true" if b_val else "false"

    elif is_sint or is_uint:
        tyle = dlt_type & DLT_TYPE_INFO_TYLE
        size_map = {
            DLT_TYLE_8BIT: 1,
            DLT_TYLE_16BIT: 2,
            DLT_TYLE_32BIT: 4,
            DLT_TYLE_64BIT: 8,
            DLT_TYLE_128BIT: 16,
        }
        size = size_map.get(tyle, 0)
        if size == 0 or payload_len < curr + size:
            return None

        data = payload[curr : curr + size]
        curr += size

        # Format unpacked number
        if size == 1:
            fmt = "b" if is_sint else "B"
            val = struct.unpack(fmt, data)[0]
        elif size == 2:
            fmt = f"{endian_str}h" if is_sint else f"{endian_str}H"
            val = struct.unpack(fmt, data)[0]
        elif size == 4:
            fmt = f"{endian_str}i" if is_sint else f"{endian_str}I"
            val = struct.unpack(fmt, data)[0]
        elif size == 8:
            fmt = f"{endian_str}q" if is_sint else f"{endian_str}Q"
            val = struct.unpack(fmt, data)[0]
        elif size == 16:
            # 128-bit integer
            val = int.from_bytes(data, byteorder="big" if is_big_endian else "little", signed=is_sint)

        scod = dlt_type & DLT_TYPE_INFO_SCOD
        if is_uint and scod == DLT_SCOD_HEX:
            string_repr = f"0x{val:X}"
        elif is_uint and scod == DLT_SCOD_BIN:
            string_repr = bin(val)
        else:
            string_repr = str(val)

    elif is_float:
        tyle = dlt_type & DLT_TYPE_INFO_TYLE
        if tyle == DLT_TYLE_32BIT:
            if payload_len < curr + 4:
                return None
            data = payload[curr : curr + 4]
            curr += 4
            val = struct.unpack(f"{endian_str}f", data)[0]
            string_repr = f"{val:.8f}".rstrip("0").rstrip(".") if "." in f"{val:.8f}" else str(val)
        elif tyle == DLT_TYLE_64BIT:
            if payload_len < curr + 8:
                return None
            data = payload[curr : curr + 8]
            curr += 8
            val = struct.unpack(f"{endian_str}d", data)[0]
            string_repr = f"{val:.8f}".rstrip("0").rstrip(".") if "." in f"{val:.8f}" else str(val)
        else:
            return None

    # Append variable name and unit if available
    if name:
        if unit:
            string_repr = f"{name}: {string_repr} {unit}"
        else:
            string_repr = f"{name}: {string_repr}"

    arg = DltArgument(
        type_info=dlt_type,
        type_name=arg_type_name,
        name=name,
        unit=unit,
        data=data,
        value=val,
        string_repr=string_repr,
    )
    return arg, curr


def format_hex_ascii_table(data: bytes, max_len: int = 256) -> str:
    """Format non-verbose payload data as hex and printable ASCII matching DLT Viewer."""
    if not data:
        return ""
    chunk = data[:max_len]
    hex_str = " ".join(f"{b:02x}" for b in chunk)
    ascii_chars = [chr(b) if 32 <= b <= 126 else "-" for b in chunk]
    ascii_str = "".join(ascii_chars)
    return f"{hex_str} | {ascii_str}"


def parse_control_message(
    payload: bytes, is_big_endian: bool, is_response: bool
) -> Tuple[int, str, str, str]:
    """Parse control message payload.

    Returns (service_id, service_name, return_type_str, description).
    """
    if len(payload) < 4:
        return 0, "", "", ""

    endian_str = ">" if is_big_endian else "<"
    service_id = struct.unpack_from(f"{endian_str}I", payload, 0)[0]
    service_name = DLT_SERVICE_ID_STRINGS.get(service_id, f"service_0x{service_id:X}")

    return_type_str = ""
    offset = 4
    if is_response and len(payload) >= 5:
        return_type = payload[4]
        return_type_str = DLT_CONTROL_RETURN_TYPE_STRINGS.get(return_type, str(return_type))
        offset = 5

    desc = ""
    if service_id == DLT_SERVICE_ID_MARKER:
        desc = "MARKER"
    elif service_id == DLT_SERVICE_ID_GET_SOFTWARE_VERSION and is_response:
        # String starting from offset
        desc = payload[offset:].decode("utf-8", errors="replace").replace("\x00", "").strip()
    elif service_id == DLT_SERVICE_ID_CONNECTION_INFO:
        if len(payload) >= offset + 5:
            state = payload[offset]
            state_str = "connected" if state == 2 else "disconnected" if state == 1 else "unknown"
            comid = clean_id_string(payload[offset + 1 : offset + 5])
            desc = f"{state_str} {comid}"
        else:
            desc = payload[offset:].hex()
    elif service_id == DLT_SERVICE_ID_TIMEZONE:
        if len(payload) >= offset + 5:
            tz_sec = struct.unpack_from(f"{endian_str}i", payload, offset)[0]
            is_dst = bool(payload[offset + 4])
            desc = f"{tz_sec} s {'DST' if is_dst else ''}".strip()
        else:
            desc = payload[offset:].hex()
    else:
        # Generic payload hex representation
        rest = payload[offset:]
        if rest:
            desc = " ".join(f"{b:02x}" for b in rest[:64])

    return service_id, service_name, return_type_str, desc


def parse_dlt_v2_message(
    buf: bytes, offset: int, storage_header: Optional[DltStorageHeader]
) -> Optional[Tuple[DltMessage, int]]:
    """Parse a DLT v2 message starting at offset.

    Returns tuple (DltMessage, total_consumed_bytes) or None on failure.
    """
    buf_len = len(buf) - offset
    if buf_len < 7:
        return None

    htyp2 = struct.unpack_from("<I", buf, offset)[0]
    version = (htyp2 & 0xE0) >> 5
    if version != 2:
        return None

    # DLT v2 bitfield flags
    with_session_id = bool(htyp2 & 0x10)
    with_app_context_id = bool(htyp2 & 0x08)
    with_ecu_id = bool(htyp2 & 0x04)
    content_info = htyp2 & 0x03  # 0: verbose, 1: non-verbose, 2: control

    is_verbose = (content_info == 0)
    with_hf_msg_info = (content_info in (0, 2))
    with_hf_noar = (content_info in (0, 2))
    with_hf_timestamp = (content_info in (0, 1))
    with_hf_msg_id = (content_info == 1)

    mcnt = buf[offset + 4]
    msg_len = struct.unpack_from(">H", buf, offset + 5)[0]

    if buf_len < msg_len:
        return None

    curr = offset + 7
    msg_type_idx = 0
    msg_subtype_idx = 0
    if with_hf_msg_info:
        if curr >= offset + msg_len:
            return None
        minfo = buf[curr]
        curr += 1
        msg_type_idx = (minfo & 0x0E) >> 1
        msg_subtype_idx = (minfo & 0xF0) >> 4

    noar = 0
    if with_hf_noar:
        if curr >= offset + msg_len:
            return None
        noar = buf[curr]
        curr += 1

    timestamp = 0
    time_seconds = storage_header.seconds if storage_header else 0
    microseconds = storage_header.microseconds if storage_header else 0

    if with_hf_timestamp:
        if curr + 9 > offset + msg_len:
            return None
        nanosecs = struct.unpack_from(">I", buf, curr)[0]
        # 5-byte seconds
        sec_bytes = buf[curr + 4 : curr + 9]
        secs = int.from_bytes(sec_bytes, byteorder="big")
        curr += 9
        # Timestamp calculation matching qdltmsg.cpp
        timestamp = (secs * 10000) + (nanosecs // 100000)
        if not (nanosecs & 0x80000000):
            time_seconds = secs
            microseconds = (nanosecs & 0x7FFFFFFF) // 1000

    msg_id = None
    if with_hf_msg_id:
        if curr + 4 > offset + msg_len:
            return None
        msg_id = struct.unpack_from(">I", buf, curr)[0]
        curr += 4

    ecu_id = storage_header.ecu if storage_header else ""
    if with_ecu_id:
        if curr >= offset + msg_len:
            return None
        ecu_len = buf[curr]
        curr += 1
        if curr + ecu_len > offset + msg_len:
            return None
        ecu_id = clean_id_string(buf[curr : curr + ecu_len])
        curr += ecu_len

    apid = ""
    ctid = ""
    if with_app_context_id:
        if curr >= offset + msg_len:
            return None
        apid_len = buf[curr]
        curr += 1
        if curr + apid_len > offset + msg_len:
            return None
        apid = clean_id_string(buf[curr : curr + apid_len])
        curr += apid_len

        if curr >= offset + msg_len:
            return None
        ctid_len = buf[curr]
        curr += 1
        if curr + ctid_len > offset + msg_len:
            return None
        ctid = clean_id_string(buf[curr : curr + ctid_len])
        curr += ctid_len

    session_id = 0
    if with_session_id:
        if curr + 4 > offset + msg_len:
            return None
        session_id = struct.unpack_from(">I", buf, curr)[0]
        curr += 4

    # Payload slice
    payload_end = offset + msg_len
    payload = buf[curr:payload_end]

    type_str = DLT_MESSAGE_TYPE_STRINGS.get(msg_type_idx, "")
    subtype_str = ""
    if msg_type_idx == DLT_TYPE_LOG:
        subtype_str = DLT_LOG_SUBTYPE_STRINGS.get(msg_subtype_idx, "")
    elif msg_type_idx == DLT_TYPE_APP_TRACE:
        subtype_str = DLT_TRACE_SUBTYPE_STRINGS.get(msg_subtype_idx, "")
    elif msg_type_idx == DLT_TYPE_NW_TRACE:
        subtype_str = DLT_NW_TRACE_SUBTYPE_STRINGS.get(msg_subtype_idx, "")
    elif msg_type_idx == DLT_TYPE_CONTROL:
        subtype_str = DLT_CONTROL_SUBTYPE_STRINGS.get(msg_subtype_idx, "")

    mode_str = "verbose" if is_verbose else "non-verbose"

    # Decode arguments or non-verbose payload
    args: List[DltArgument] = []
    payload_text = ""

    if is_verbose:
        arg_offset = 0
        for _ in range(noar):
            res = parse_argument(payload, arg_offset, is_big_endian=False)
            if not res:
                break
            arg, arg_offset = res
            args.append(arg)
        payload_text = " ".join(a.as_string() for a in args)
    else:
        if msg_id is not None:
            payload_text = f"[{msg_id}] " + format_hex_ascii_table(payload)
        else:
            payload_text = format_hex_ascii_table(payload)

    msg = DltMessage(
        version=2,
        time_seconds=time_seconds,
        microseconds=microseconds,
        timestamp=timestamp,
        message_counter=mcnt,
        ecu_id=ecu_id,
        apid=apid,
        ctid=ctid,
        session_id=session_id,
        type=type_str,
        subtype=subtype_str,
        mode=mode_str,
        number_of_arguments=noar,
        arguments=args,
        payload_bytes=payload,
        payload_text=payload_text,
        message_id=msg_id,
    )
    return msg, msg_len


def parse_dlt_message(
    buf: bytes, offset: int = 0
) -> Optional[Tuple[DltMessage, int]]:
    """Parse a single DLT message (v1 or v2, with or without storage header).

    Returns tuple (DltMessage, total_consumed_bytes) or None if buffer does
    not contain a complete valid message at the offset.
    """
    buf_len = len(buf) - offset
    if buf_len < 4:
        return None

    curr = offset
    storage_hdr = parse_storage_header(buf, curr)
    if storage_hdr:
        curr += storage_hdr.header_length

    # Check for DLT version at curr
    if len(buf) - curr < 4:
        return None

    # Version is stored in bits 5-7 of byte 0
    first_byte = buf[curr]
    version = (first_byte & DLT_HTYP_VERS) >> 5

    if version == 2:
        res = parse_dlt_v2_message(buf, curr, storage_hdr)
        if not res:
            return None
        msg, v2_consumed = res
        total_consumed = (curr - offset) + v2_consumed
        msg.msg_length = total_consumed
        return msg, total_consumed

    # Otherwise DLT v1
    std_res = parse_standard_header(buf, curr)
    if not std_res:
        return None
    std_hdr, std_consumed = std_res
    curr += std_consumed

    # std_hdr.length is the total message length excluding the 16-byte storage header
    # Thus, message body after standard header must be:
    # remaining_len = std_hdr.length - std_consumed
    msg_total_expected = (curr - offset - (storage_hdr.header_length if storage_hdr else 0)) + (std_hdr.length - std_consumed)
    storage_hdr_len = storage_hdr.header_length if storage_hdr else 0
    if len(buf) - offset < storage_hdr_len + std_hdr.length:
        return None  # Incomplete message in buffer

    ext_hdr: Optional[DltExtendedHeader] = None
    if std_hdr.use_extended_header:
        ext_res = parse_extended_header(buf, curr)
        if not ext_res:
            return None
        ext_hdr, ext_consumed = ext_res
        curr += ext_consumed

    # Calculate payload slice
    payload_end = offset + storage_hdr_len + std_hdr.length
    payload = buf[curr:payload_end]
    total_consumed = storage_hdr_len + std_hdr.length

    # Determine ECU ID, APID, CTID
    ecu_id = std_hdr.ecu_id or (storage_hdr.ecu if storage_hdr else "")
    apid = ext_hdr.apid if ext_hdr else ""
    ctid = ext_hdr.ctid if ext_hdr else ""

    # Message Type & Subtype
    type_str = ""
    subtype_str = ""
    is_verbose = False
    noar = 0

    if ext_hdr:
        is_verbose = ext_hdr.is_verbose
        noar = ext_hdr.number_of_arguments
        type_str = DLT_MESSAGE_TYPE_STRINGS.get(ext_hdr.message_type, "")

        if ext_hdr.message_type == DLT_TYPE_LOG:
            subtype_str = DLT_LOG_SUBTYPE_STRINGS.get(ext_hdr.message_subtype, "")
        elif ext_hdr.message_type == DLT_TYPE_APP_TRACE:
            subtype_str = DLT_TRACE_SUBTYPE_STRINGS.get(ext_hdr.message_subtype, "")
        elif ext_hdr.message_type == DLT_TYPE_NW_TRACE:
            subtype_str = DLT_NW_TRACE_SUBTYPE_STRINGS.get(ext_hdr.message_subtype, "")
        elif ext_hdr.message_type == DLT_TYPE_CONTROL:
            subtype_str = DLT_CONTROL_SUBTYPE_STRINGS.get(ext_hdr.message_subtype, "")

    mode_str = "verbose" if is_verbose else "non-verbose"

    # Decode Payload
    args: List[DltArgument] = []
    payload_text = ""
    msg_id: Optional[int] = None
    ctrl_service_id: Optional[int] = None
    ctrl_service_name = ""
    ctrl_return_type = ""

    if ext_hdr and ext_hdr.message_type == DLT_TYPE_CONTROL:
        is_response = (ext_hdr.message_subtype == DLT_CONTROL_RESPONSE)
        ctrl_service_id, ctrl_service_name, ctrl_return_type, ctrl_desc = parse_control_message(
            payload, std_hdr.is_big_endian, is_response
        )
        payload_text = f"[{ctrl_service_name}"
        if ctrl_return_type:
            payload_text += f" {ctrl_return_type}"
        payload_text += f"] {ctrl_desc}".rstrip()

    elif is_verbose:
        arg_offset = 0
        for _ in range(noar):
            arg_res = parse_argument(payload, arg_offset, std_hdr.is_big_endian)
            if not arg_res:
                break
            arg, arg_offset = arg_res
            args.append(arg)
        payload_text = " ".join(a.as_string() for a in args)

    else:
        # Non-verbose message: starts with 4-byte message ID
        if len(payload) >= 4:
            endian_str = ">" if std_hdr.is_big_endian else "<"
            msg_id = struct.unpack_from(f"{endian_str}I", payload, 0)[0]
            rest_data = payload[4:]
            payload_text = f"[{msg_id}] " + format_hex_ascii_table(rest_data)
        else:
            payload_text = format_hex_ascii_table(payload)

    msg = DltMessage(
        version=1,
        time_seconds=storage_hdr.seconds if storage_hdr else 0,
        microseconds=storage_hdr.microseconds if storage_hdr else 0,
        timestamp=std_hdr.timestamp,
        message_counter=std_hdr.mcnt,
        ecu_id=ecu_id,
        apid=apid,
        ctid=ctid,
        session_id=std_hdr.session_id,
        type=type_str,
        subtype=subtype_str,
        mode=mode_str,
        number_of_arguments=noar,
        arguments=args,
        payload_bytes=payload,
        payload_text=payload_text,
        message_id=msg_id,
        ctrl_service_id=ctrl_service_id,
        ctrl_service_name=ctrl_service_name,
        ctrl_return_type=ctrl_return_type,
        msg_length=total_consumed,
    )
    return msg, total_consumed

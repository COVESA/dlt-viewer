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

"""Constants and definitions for DLT (Diagnostic Log and Trace) protocol parsing.

Based on COVESA/dlt-viewer specifications and AUTOSAR DLT protocol standards.
"""

# Storage Header patterns
DLT_STORAGE_HEADER_PATTERN_V1 = b"DLT\x01"
DLT_STORAGE_HEADER_PATTERN_V2 = b"DLT\x02"
DLT_SERIAL_HEADER_PATTERN = b"DLS\x01"

# Standard Header: Header Type (htyp) parameter bit definitions
DLT_HTYP_UEH = 0x01   # Use Extended Header
DLT_HTYP_MSBF = 0x02  # Most Significant Byte First (Big Endian)
DLT_HTYP_WEID = 0x04  # With ECU ID
DLT_HTYP_WSID = 0x08  # With Session ID
DLT_HTYP_WTMS = 0x10  # With Timestamp
DLT_HTYP_VERS = 0xE0  # Version number mask (version = (htyp & 0xE0) >> 5)

# Extended Header: Message Info (msin) parameter bit definitions
DLT_MSIN_VERB = 0x01        # Verbose mode
DLT_MSIN_MSTP = 0x0E        # Message type mask
DLT_MSIN_MTIN = 0xF0        # Message type info (subtype) mask
DLT_MSIN_MSTP_SHIFT = 1
DLT_MSIN_MTIN_SHIFT = 4

# Message Types (mstp)
DLT_TYPE_LOG = 0x00         # Log message
DLT_TYPE_APP_TRACE = 0x01   # Application trace message
DLT_TYPE_NW_TRACE = 0x02    # Network trace message
DLT_TYPE_CONTROL = 0x03     # Control message

# Message Type Strings (matching DLT Viewer qDltMessageType)
DLT_MESSAGE_TYPE_STRINGS = {
    DLT_TYPE_LOG: "log",
    DLT_TYPE_APP_TRACE: "app_trace",
    DLT_TYPE_NW_TRACE: "nw_trace",
    DLT_TYPE_CONTROL: "control",
}

# Subtype definitions for Log Messages (DLT_TYPE_LOG)
DLT_LOG_FATAL = 0x01
DLT_LOG_ERROR = 0x02
DLT_LOG_WARN = 0x03
DLT_LOG_INFO = 0x04
DLT_LOG_DEBUG = 0x05
DLT_LOG_VERBOSE = 0x06

DLT_LOG_SUBTYPE_STRINGS = {
    DLT_LOG_FATAL: "fatal",
    DLT_LOG_ERROR: "error",
    DLT_LOG_WARN: "warn",
    DLT_LOG_INFO: "info",
    DLT_LOG_DEBUG: "debug",
    DLT_LOG_VERBOSE: "verbose",
}

# Subtype definitions for Application Trace Messages (DLT_TYPE_APP_TRACE)
DLT_TRACE_VARIABLE = 0x01
DLT_TRACE_FUNCTION_IN = 0x02
DLT_TRACE_FUNCTION_OUT = 0x03
DLT_TRACE_STATE = 0x04
DLT_TRACE_VFB = 0x05

DLT_TRACE_SUBTYPE_STRINGS = {
    DLT_TRACE_VARIABLE: "variable",
    DLT_TRACE_FUNCTION_IN: "func_in",
    DLT_TRACE_FUNCTION_OUT: "func_out",
    DLT_TRACE_STATE: "state",
    DLT_TRACE_VFB: "vfb",
}

# Subtype definitions for Network Trace Messages (DLT_TYPE_NW_TRACE)
DLT_NW_TRACE_IPC = 0x01
DLT_NW_TRACE_CAN = 0x02
DLT_NW_TRACE_FLEXRAY = 0x03
DLT_NW_TRACE_MOST = 0x04
DLT_NW_TRACE_VFB = 0x05

DLT_NW_TRACE_SUBTYPE_STRINGS = {
    DLT_NW_TRACE_IPC: "ipc",
    DLT_NW_TRACE_CAN: "can",
    DLT_NW_TRACE_FLEXRAY: "flexray",
    DLT_NW_TRACE_MOST: "most",
    DLT_NW_TRACE_VFB: "vfb",
}

# Subtype definitions for Control Messages (DLT_TYPE_CONTROL)
DLT_CONTROL_REQUEST = 0x01
DLT_CONTROL_RESPONSE = 0x02
DLT_CONTROL_TIME = 0x03

DLT_CONTROL_SUBTYPE_STRINGS = {
    DLT_CONTROL_REQUEST: "request",
    DLT_CONTROL_RESPONSE: "response",
    DLT_CONTROL_TIME: "time",
}

# Type definitions of arguments in verbose payload
DLT_TYPE_INFO_TYLE = 0x0000000F  # Length of standard data
DLT_TYPE_INFO_BOOL = 0x00000010  # Boolean data
DLT_TYPE_INFO_SINT = 0x00000020  # Signed integer data
DLT_TYPE_INFO_UINT = 0x00000040  # Unsigned integer data
DLT_TYPE_INFO_FLOA = 0x00000080  # Float data
DLT_TYPE_INFO_ARAY = 0x00000100  # Array of standard types
DLT_TYPE_INFO_STRG = 0x00000200  # String
DLT_TYPE_INFO_RAWD = 0x00000400  # Raw data
DLT_TYPE_INFO_VARI = 0x00000800  # Variable name and optional unit present
DLT_TYPE_INFO_FIXP = 0x00001000  # Fixed point
DLT_TYPE_INFO_TRAI = 0x00002000  # Trace info
DLT_TYPE_INFO_STRU = 0x00004000  # Struct
DLT_TYPE_INFO_SCOD = 0x00038000  # String coding mask

# Type Length (TYLE) values
DLT_TYLE_8BIT = 0x00000001   # 1 byte
DLT_TYLE_16BIT = 0x00000002  # 2 bytes
DLT_TYLE_32BIT = 0x00000003  # 4 bytes
DLT_TYLE_64BIT = 0x00000004  # 8 bytes
DLT_TYLE_128BIT = 0x00000005 # 16 bytes

# String Coding (SCOD) values
DLT_SCOD_ASCII = 0x00000000
DLT_SCOD_UTF8 = 0x00008000
DLT_SCOD_HEX = 0x00010000
DLT_SCOD_BIN = 0x00018000

# Control Service IDs
DLT_SERVICE_ID_SET_LOG_LEVEL = 0x01
DLT_SERVICE_ID_SET_TRACE_STATUS = 0x02
DLT_SERVICE_ID_GET_LOG_INFO = 0x03
DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL = 0x04
DLT_SERVICE_ID_STORE_CONFIG = 0x05
DLT_SERVICE_ID_RESET_TO_FACTORY_DEFAULT = 0x06
DLT_SERVICE_ID_SET_COM_INTERFACE_STATUS = 0x07
DLT_SERVICE_ID_SET_COM_INTERFACE_MAX_BANDWIDTH = 0x08
DLT_SERVICE_ID_SET_VERBOSE_MODE = 0x09
DLT_SERVICE_ID_SET_MESSAGE_FILTERING = 0x0A
DLT_SERVICE_ID_SET_TIMING_PACKETS = 0x0B
DLT_SERVICE_ID_GET_LOCAL_TIME = 0x0C
DLT_SERVICE_ID_USE_ECU_ID = 0x0D
DLT_SERVICE_ID_USE_SESSION_ID = 0x0E
DLT_SERVICE_ID_USE_TIMESTAMP = 0x0F
DLT_SERVICE_ID_USE_EXTENDED_HEADER = 0x10
DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL = 0x11
DLT_SERVICE_ID_SET_DEFAULT_TRACE_STATUS = 0x12
DLT_SERVICE_ID_GET_SOFTWARE_VERSION = 0x13
DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW = 0x14
DLT_SERVICE_ID_UNREGISTER_CONTEXT = 0xF01
DLT_SERVICE_ID_CONNECTION_INFO = 0xF02
DLT_SERVICE_ID_TIMEZONE = 0xF03
DLT_SERVICE_ID_MARKER = 0xF04

DLT_SERVICE_ID_STRINGS = {
    DLT_SERVICE_ID_SET_LOG_LEVEL: "set_log_level",
    DLT_SERVICE_ID_SET_TRACE_STATUS: "set_trace_status",
    DLT_SERVICE_ID_GET_LOG_INFO: "get_log_info",
    DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL: "get_default_log_level",
    DLT_SERVICE_ID_STORE_CONFIG: "store_config",
    DLT_SERVICE_ID_RESET_TO_FACTORY_DEFAULT: "reset_to_factory_default",
    DLT_SERVICE_ID_SET_COM_INTERFACE_STATUS: "set_com_interface_status",
    DLT_SERVICE_ID_SET_COM_INTERFACE_MAX_BANDWIDTH: "set_com_interface_max_bandwidth",
    DLT_SERVICE_ID_SET_VERBOSE_MODE: "set_verbose_mode",
    DLT_SERVICE_ID_SET_MESSAGE_FILTERING: "set_message_filtering",
    DLT_SERVICE_ID_SET_TIMING_PACKETS: "set_timing_packets",
    DLT_SERVICE_ID_GET_LOCAL_TIME: "get_local_time",
    DLT_SERVICE_ID_USE_ECU_ID: "use_ecu_id",
    DLT_SERVICE_ID_USE_SESSION_ID: "use_session_id",
    DLT_SERVICE_ID_USE_TIMESTAMP: "use_timestamp",
    DLT_SERVICE_ID_USE_EXTENDED_HEADER: "use_extended_header",
    DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL: "set_default_log_level",
    DLT_SERVICE_ID_SET_DEFAULT_TRACE_STATUS: "set_default_trace_status",
    DLT_SERVICE_ID_GET_SOFTWARE_VERSION: "get_software_version",
    DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW: "message_buffer_overflow",
    DLT_SERVICE_ID_UNREGISTER_CONTEXT: "unregister_context",
    DLT_SERVICE_ID_CONNECTION_INFO: "connection_info",
    DLT_SERVICE_ID_TIMEZONE: "timezone",
    DLT_SERVICE_ID_MARKER: "marker",
}

# Control Return Type strings
DLT_CONTROL_RETURN_TYPE_STRINGS = {
    0x00: "ok",
    0x01: "not_supported",
    0x02: "error",
    0x08: "no_matching_context_id",
}

# Standard column list matching DLT Viewer export
DEFAULT_COLUMNS = [
    "Index",
    "Time",
    "Timestamp",
    "Count",
    "Ecuid",
    "Apid",
    "Ctid",
    "SessionId",
    "Type",
    "Subtype",
    "Mode",
    "#Args",
    "Payload",
]

# Forensic-enhanced column list with traceability fields
FORENSIC_COLUMNS = [
    "Index",
    "Time",
    "Timestamp",
    "Count",
    "Ecuid",
    "Apid",
    "Ctid",
    "SessionId",
    "Type",
    "Subtype",
    "Mode",
    "#Args",
    "Payload",
    "SourceFile",
    "FileOffset",
    "MsgLength",
    "MsgID",
    "EpochSeconds",
    "Microseconds",
]

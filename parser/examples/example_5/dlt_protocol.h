/**
 * @licence app begin@
 * Copyright (C) 2014  BMW AG
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 * Contributions are licensed to the COVESA Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \author
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \file dlt_protocol.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */


/*******************************************************************************
**                      Include Section                                       **
*******************************************************************************/
#ifndef DLT_PROTOCOL_H
#define DLT_PROTOCOL_H

/*
 * Macros to swap the byte order.
 */
#define DLT_SWAP_64(value) ((((uint64)DLT_SWAP_32((value)&0xffffffffull))<<32) | (DLT_SWAP_32((value)>>32)))
#define DLT_SWAP_16(value) ((((value) >> 8)&0xff) | (((value) << 8)&0xff00))
#define DLT_SWAP_32(value) ((((value) >> 24)&0xff) | (((value) << 8)&0xff0000) | (((value) >> 8)&0xff00) | (((value) << 24)&0xff000000))

/* Set Big Endian and Little Endian to a initial value, if not defined */
#ifndef LITTLE_ENDIAN
	#define LITTLE_ENDIAN 1234
#endif

#ifndef BIG_ENDIAN
	#define BIG_ENDIAN    4321
#endif

/* If byte order is not defined, default to little endian */
#ifndef BYTE_ORDER
	#define BYTE_ORDER LITTLE_ENDIAN
#endif

/* Check for byte-order */
#if (BYTE_ORDER==BIG_ENDIAN)
	/* #warning "Big Endian Architecture!" */
	#define DLT_HTOBE_16(x) (x)
	#define DLT_HTOLE_16(x) DLT_SWAP_16(x)
	#define DLT_BETOH_16(x) (x)
	#define DLT_LETOH_16(x) DLT_SWAP_16(x)

	#define DLT_HTOBE_32(x) (x)
	#define DLT_HTOLE_32(x) DLT_SWAP_32(x)
	#define DLT_BETOH_32(x) (x)
	#define DLT_LETOH_32(x) DLT_SWAP_32(x)

	#define DLT_HTOBE_64(x) (x)
	#define DLT_HTOLE_64(x) DLT_SWAP_64(x)
	#define DLT_BETOH_64(x) (x)
	#define DLT_LETOH_64(x) DLT_SWAP_64(x)
#else
	/* #warning "Litte Endian Architecture!" */
	#define DLT_HTOBE_16(x) DLT_SWAP_16(x)
	#define DLT_HTOLE_16(x) (x)
	#define DLT_BETOH_16(x) DLT_SWAP_16(x)
	#define DLT_LETOH_16(x) (x)

	#define DLT_HTOBE_32(x) DLT_SWAP_32(x)
	#define DLT_HTOLE_32(x) (x)
	#define DLT_BETOH_32(x) DLT_SWAP_32(x)
	#define DLT_LETOH_32(x) (x)

	#define DLT_HTOBE_64(x) DLT_SWAP_64(x)
	#define DLT_HTOLE_64(x) (x)
	#define DLT_BETOH_64(x) DLT_SWAP_64(x)
	#define DLT_LETOH_64(x) (x)
#endif

#define DLT_ENDIAN_GET_16(htyp,x) (((htyp) & DLT_HTYP_MSBF)?DLT_BETOH_16(x):DLT_LETOH_16(x))
#define DLT_ENDIAN_GET_32(htyp,x) (((htyp) & DLT_HTYP_MSBF)?DLT_BETOH_32(x):DLT_LETOH_32(x))
#define DLT_ENDIAN_GET_64(htyp,x) (((htyp) & DLT_HTYP_MSBF)?DLT_BETOH_64(x):DLT_LETOH_64(x))

#if !defined (PACKED)
#define PACKED __attribute__((packed))
#endif

/**
 * The size of a DLT ID
 */
#define DLT_ID_SIZE 4

/**
 * The definition of the serial header containing the characters "DLS" + 0x01.
 */
extern const char dltSerialHeader[DLT_ID_SIZE];

/**
 * The structure of the DLT standard header. This header is used in each DLT message.
 */
typedef struct
{
	uint8 htyp;           /**< This parameter contains several informations, see definitions below */
	uint8 mcnt;           /**< The message counter is increased with each sent DLT message */
	uint16 len;           /**< Length of the complete message, without storage header */
} PACKED DltStandardHeader;

/*
 * Definitions of the htyp parameter in standard header.
 */
#define DLT_HTYP_UEH  0x01 /**< use extended header */
#define DLT_HTYP_MSBF 0x02 /**< MSB first */
#define DLT_HTYP_WEID 0x04 /**< with ECU ID */
#define DLT_HTYP_WSID 0x08 /**< with session ID */
#define DLT_HTYP_WTMS 0x10 /**< with timestamp */
#define DLT_HTYP_VERS 0xe0 /**< version number, 0x1 */

#define DLT_IS_HTYP_UEH(htyp)  ((htyp) & DLT_HTYP_UEH)
#define DLT_IS_HTYP_MSBF(htyp) ((htyp) & DLT_HTYP_MSBF)
#define DLT_IS_HTYP_WEID(htyp) ((htyp) & DLT_HTYP_WEID)
#define DLT_IS_HTYP_WSID(htyp) ((htyp) & DLT_HTYP_WSID)
#define DLT_IS_HTYP_WTMS(htyp) ((htyp) & DLT_HTYP_WTMS)

#define DLT_HTYP_PROTOCOL_VERSION1 (1<<5)

/**
 * The structure of the DLT extra header parameters. Each parameter is sent only if enabled in htyp.
 */
typedef struct
{
	char ecu[DLT_ID_SIZE];       /**< ECU id */
	uint32 seid;     /**< Session number */
	uint32 tmsp;     /**< Timestamp since system start in 0.1 milliseconds */
} PACKED DltStandardHeaderExtra;

#define DLT_SIZE_WEID DLT_ID_SIZE
#define DLT_SIZE_WSID (sizeof(uint32))
#define DLT_SIZE_WTMS (sizeof(uint32))

/**
 * Get the size of extra header parameters, depends on htyp.
 */
#define DLT_STANDARD_HEADER_EXTRA_SIZE(htyp) ( (DLT_IS_HTYP_WEID(htyp) ? DLT_SIZE_WEID : 0) + (DLT_IS_HTYP_WSID(htyp) ? DLT_SIZE_WSID : 0) + (DLT_IS_HTYP_WTMS(htyp) ? DLT_SIZE_WTMS : 0) )

/**
 * The structure of the DLT extended header. This header is only sent if enabled in htyp parameter.
 */
typedef struct
{
	uint8 msin;          /**< messsage info */
	uint8 noar;          /**< number of arguments */
	char apid[DLT_ID_SIZE];          /**< application id */
	char ctid[DLT_ID_SIZE];          /**< context id */
} PACKED DltExtendedHeader;

/*
 * Definitions of msin parameter in extended header.
 */
#define DLT_MSIN_VERB 0x01 /**< verbose */
#define DLT_MSIN_MSTP 0x0e /**< message type */
#define DLT_MSIN_MTIN 0xf0 /**< message type info */

#define DLT_MSIN_MSTP_SHIFT 1 /**< shift right offset to get mstp value */
#define DLT_MSIN_MTIN_SHIFT 4 /**< shift right offset to get mtin value */

#define DLT_IS_MSIN_VERB(msin)   ((msin) & DLT_MSIN_VERB)
#define DLT_GET_MSIN_MSTP(msin) (((msin) & DLT_MSIN_MSTP) >> DLT_MSIN_MSTP_SHIFT)
#define DLT_GET_MSIN_MTIN(msin) (((msin) & DLT_MSIN_MTIN) >> DLT_MSIN_MTIN_SHIFT)

/*
 * Definitions of mstp parameter in extended header.
 */
#define DLTYPE_LOG       0x00 /**< Log message type */
#define DLTYPE_APPRACE 0x01 /**< Application trace message type */
#define DLTYPE_NWRACE  0x02 /**< Network trace message type */
#define DLTYPE_CONTROL   0x03 /**< Control message type */

/*
 * Definitions of msti parameter in extended header.
 */
#define DLTRACE_VARIABLE     0x01 /**< tracing of a variable */
#define DLTRACE_FUNCTION_IN  0x02 /**< tracing of function calls */
#define DLTRACE_FUNCTION_OUT 0x03 /**< tracing of function return values */
#define DLTRACE_STATE        0x04 /**< tracing of states of a state machine */
#define DLTRACE_VFB          0x05 /**< tracing of virtual function bus */

/*
 * Definitions of msbi parameter in extended header.
 */
#define DLT_NWRACE_IPC       0x01 /**< Interprocess communication */
#define DLT_NWRACE_CAN       0x02 /**< Controller Area Network Bus */
#define DLT_NWRACE_FLEXRAY   0x03 /**< Flexray Bus */
#define DLT_NWRACE_MOST      0x04 /**< Media Oriented System Transport Bus */

/*
 * Definitions of msci parameter in extended header.
 */
#define DLT_CONTROL_REQUEST    0x01 /**< Request message */
#define DLT_CONTROL_RESPONSE   0x02 /**< Response to request message */
#define DLT_CONTROLIME       0x03 /**< keep-alive message */

#define DLT_MSIN_CONTROL_REQUEST  ((DLTYPE_CONTROL << DLT_MSIN_MSTP_SHIFT) | (DLT_CONTROL_REQUEST  << DLT_MSIN_MTIN_SHIFT))
#define DLT_MSIN_CONTROL_RESPONSE ((DLTYPE_CONTROL << DLT_MSIN_MSTP_SHIFT) | (DLT_CONTROL_RESPONSE << DLT_MSIN_MTIN_SHIFT))

/*
 * Definitions of types of arguments in payload.
 */
#define DLTYPE_INFOYLE	0x0000000f /**< Length of standard data: 1 = 8bit, 2 = 16bit, 3 = 32 bit, 4 = 64 bit, 5 = 128 bit */
#define DLTYPE_INFO_BOOL	0x00000010 /**< Boolean data */
#define DLTYPE_INFO_SINT	0x00000020 /**< Signed integer data */
#define DLTYPE_INFO_UINT	0x00000040 /**< Unsigned integer data */
#define DLTYPE_INFO_FLOA	0x00000080 /**< Float data */
#define DLTYPE_INFO_ARAY	0x00000100 /**< Array of standard types */
#define DLTYPE_INFO_STRG	0x00000200 /**< String */
#define DLTYPE_INFO_RAWD	0x00000400 /**< Raw data */
#define DLTYPE_INFO_VARI	0x00000800 /**< Set, if additional information to a variable is available */
#define DLTYPE_INFO_FIXP	0x00001000 /**< Set, if quantization and offset are added */
#define DLTYPE_INFORAI	0x00002000 /**< Set, if additional trace information is added */
#define DLTYPE_INFO_STRU	0x00004000 /**< Struct */
#define DLTYPE_INFO_SCOD	0x00038000 /**< coding of the type string: 0 = ASCII, 1 = UTF-8 */

#define DLTYLE_8BIT		1
#define DLTYLE_16BIT		2
#define DLTYLE_32BIT		3
#define DLTYLE_64BIT		4
#define DLTYLE_128BIT		5

/*
 * Definitions of DLT services.
 */
#define DLT_SERVICE_ID_SET_LOG_LEVEL                   0x01 /**< Service ID: Set log level */
#define DLT_SERVICE_ID_SETRACE_STATUS                0x02 /**< Service ID: Set trace status */
#define DLT_SERVICE_ID_GET_LOG_INFO                    0x03 /**< Service ID: Get log info */
#define DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL           0x04 /**< Service ID: Get dafault log level */
#define DLT_SERVICE_ID_STORE_CONFIG                    0x05 /**< Service ID: Store configuration */
#define DLT_SERVICE_ID_RESETO_FACTORY_DEFAULT        0x06 /**< Service ID: Reset to factory defaults */
#define DLT_SERVICE_ID_SET_COM_INTERFACE_STATUS        0x07 /**< Service ID: Set communication interface status */
#define DLT_SERVICE_ID_SET_COM_INTERFACE_MAX_BANDWIDTH 0x08 /**< Service ID: Set communication interface maximum bandwidth */
#define DLT_SERVICE_ID_SET_VERBOSE_MODE                0x09 /**< Service ID: Set verbose mode */
#define DLT_SERVICE_ID_SET_MESSAGE_FILTERING           0x0A /**< Service ID: Set message filtering */
#define DLT_SERVICE_ID_SETIMING_PACKETS              0x0B /**< Service ID: Set timing packets */
#define DLT_SERVICE_ID_GET_LOCALIME                  0x0C /**< Service ID: Get local time */
#define DLT_SERVICE_ID_USE_ECU_ID                      0x0D /**< Service ID: Use ECU id */
#define DLT_SERVICE_ID_USE_SESSION_ID                  0x0E /**< Service ID: Use session id */
#define DLT_SERVICE_ID_USEIMESTAMP                   0x0F /**< Service ID: Use timestamp */
#define DLT_SERVICE_ID_USE_EXTENDED_HEADER             0x10 /**< Service ID: Use extended header */
#define DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL           0x11 /**< Service ID: Set default log level */
#define DLT_SERVICE_ID_SET_DEFAULTRACE_STATUS        0x12 /**< Service ID: Set default trace status */
#define DLT_SERVICE_ID_GET_SOFTWARE_VERSION            0x13 /**< Service ID: Get software version */
#define DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW         0x14 /**< Service ID: Message buffer overflow */
#define DLT_SERVICE_ID_CALLSW_CINJECTION              0xFFF /**< Service ID: Message Injection (minimal ID) */

/*
 * Definitions of DLT service response status
 */
#define DLT_SERVICE_RESPONSE_OK            0x00 /**< Control message response: OK */
#define DLT_SERVICE_RESPONSE_NOT_SUPPORTED 0x01 /**< Control message response: Not supported */
#define DLT_SERVICE_RESPONSE_ERROR         0x02 /**< Control message response: Error */

/*
 * Definitions of DLT log level
 */
#define DLT_LOG_DEFAULT    		  -1    /**< Default log level */
#define DLT_LOG_OFF        		0x00    /**< Log level off */
#define DLT_LOG_FATAL      		0x01 	/**< fatal system error */
#define DLT_LOG_ERROR      		0x02 	/**< error with impact to correct functionality */
#define DLT_LOG_WARN       		0x03 	/**< warning, correct behaviour could not be ensured */
#define DLT_LOG_INFO       		0x04 	/**< informational */
#define DLT_LOG_DEBUG      		0x05 	/**< debug  */
#define DLT_LOG_VERBOSE    		0x06 	/**< highest grade of information */

/*
 * Definitions of DLT trace status
 */
#define DLTRACE_STATUS_DEFAULT	  -1 	/**< Default trace status */
#define DLTRACE_STATUS_OFF		0x00 	/**< Trace status: Off */
#define DLTRACE_STATUS_ON		0x01 	/**< Trace status: On */

/**
 * The structure of the DLT file storage header. This header is used before each stored DLT message.
 */
typedef struct
{
	char pattern[DLT_ID_SIZE];		/**< This pattern should be DLT0x01 */
	uint32 seconds;				/**< seconds since 1.1.1970 */
	sint32 microseconds;			/**< Microseconds */
	char ecu[DLT_ID_SIZE];			/**< The ECU id is added, if it is not already in the DLT message itself */
} PACKED DltStorageHeader;

#endif

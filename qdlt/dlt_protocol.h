/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> 2011-2012
 * 
 * \file dlt_protocol.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_protocol.h                                                **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**              Markus Klein                                                  **
**                                                                            **
**  PURPOSE   :                                                               **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision$
 * $LastChangedDate$
 * $LastChangedBy$
 Initials    Date         Comment
 aw          13.01.2010   initial
 */
#ifndef DLT_PROTOCOL_H
#define DLT_PROTOCOL_H

/**
  \defgroup protocolapi DLT Protocol API
  \addtogroup protocolapi
  \{
*/

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
#define DLT_TYPE_LOG       0x00 /**< Log message type */
#define DLT_TYPE_APP_TRACE 0x01 /**< Application trace message type */
#define DLT_TYPE_NW_TRACE  0x02 /**< Network trace message type */
#define DLT_TYPE_CONTROL   0x03 /**< Control message type */

/*
 * Definitions of msti parameter in extended header.
 */
#define DLT_TRACE_VARIABLE     0x01 /**< tracing of a variable */
#define DLT_TRACE_FUNCTION_IN  0x02 /**< tracing of function calls */
#define DLT_TRACE_FUNCTION_OUT 0x03 /**< tracing of function return values */
#define DLT_TRACE_STATE        0x04 /**< tracing of states of a state machine */
#define DLT_TRACE_VFB          0x05 /**< tracing of virtual function bus */

/*
 * Definitions of msbi parameter in extended header.
 */

/* see file dlt_user.h */

/*
 * Definitions of msci parameter in extended header.
 */
#define DLT_CONTROL_REQUEST    0x01 /**< Request message */
#define DLT_CONTROL_RESPONSE   0x02 /**< Response to request message */
#define DLT_CONTROL_TIME       0x03 /**< keep-alive message */

#define DLT_MSIN_CONTROL_REQUEST  ((DLT_TYPE_CONTROL << DLT_MSIN_MSTP_SHIFT) | (DLT_CONTROL_REQUEST  << DLT_MSIN_MTIN_SHIFT))
#define DLT_MSIN_CONTROL_RESPONSE ((DLT_TYPE_CONTROL << DLT_MSIN_MSTP_SHIFT) | (DLT_CONTROL_RESPONSE << DLT_MSIN_MTIN_SHIFT))
#define DLT_MSIN_CONTROL_TIME     ((DLT_TYPE_CONTROL << DLT_MSIN_MSTP_SHIFT) | (DLT_CONTROL_TIME     << DLT_MSIN_MTIN_SHIFT))

/*
 * Definitions of types of arguments in payload.
 */
#define DLT_TYPE_INFO_TYLE	0x0000000f /**< Length of standard data: 1 = 8bit, 2 = 16bit, 3 = 32 bit, 4 = 64 bit, 5 = 128 bit */
#define DLT_TYPE_INFO_BOOL	0x00000010 /**< Boolean data */
#define DLT_TYPE_INFO_SINT	0x00000020 /**< Signed integer data */
#define DLT_TYPE_INFO_UINT	0x00000040 /**< Unsigned integer data */
#define DLT_TYPE_INFO_FLOA	0x00000080 /**< Float data */
#define DLT_TYPE_INFO_ARAY	0x00000100 /**< Array of standard types */
#define DLT_TYPE_INFO_STRG	0x00000200 /**< String */
#define DLT_TYPE_INFO_RAWD	0x00000400 /**< Raw data */
#define DLT_TYPE_INFO_VARI	0x00000800 /**< Set, if additional information to a variable is available */
#define DLT_TYPE_INFO_FIXP	0x00001000 /**< Set, if quantization and offset are added */
#define DLT_TYPE_INFO_TRAI	0x00002000 /**< Set, if additional trace information is added */
#define DLT_TYPE_INFO_STRU	0x00004000 /**< Struct */
#define DLT_TYPE_INFO_SCOD	0x00038000 /**< coding of the type string: 0 = ASCII, 1 = UTF-8 */

#define DLT_TYLE_8BIT		0x00000001
#define DLT_TYLE_16BIT		0x00000002
#define DLT_TYLE_32BIT		0x00000003
#define DLT_TYLE_64BIT		0x00000004
#define DLT_TYLE_128BIT		0x00000005

#define DLT_SCOD_ASCII      0x00000000
#define DLT_SCOD_UTF8       0x00008000
#define DLT_SCOD_HEX        0x00010000
#define DLT_SCOD_BIN        0x00018000
/*
 * Definitions of DLT services.
 */
#define DLT_SERVICE_ID_SET_LOG_LEVEL                   0x01 /**< Service ID: Set log level */
#define DLT_SERVICE_ID_SET_TRACE_STATUS                0x02 /**< Service ID: Set trace status */
#define DLT_SERVICE_ID_GET_LOG_INFO                    0x03 /**< Service ID: Get log info */
#define DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL           0x04 /**< Service ID: Get dafault log level */
#define DLT_SERVICE_ID_STORE_CONFIG                    0x05 /**< Service ID: Store configuration */
#define DLT_SERVICE_ID_RESET_TO_FACTORY_DEFAULT        0x06 /**< Service ID: Reset to factory defaults */
#define DLT_SERVICE_ID_SET_COM_INTERFACE_STATUS        0x07 /**< Service ID: Set communication interface status */
#define DLT_SERVICE_ID_SET_COM_INTERFACE_MAX_BANDWIDTH 0x08 /**< Service ID: Set communication interface maximum bandwidth */
#define DLT_SERVICE_ID_SET_VERBOSE_MODE                0x09 /**< Service ID: Set verbose mode */
#define DLT_SERVICE_ID_SET_MESSAGE_FILTERING           0x0A /**< Service ID: Set message filtering */
#define DLT_SERVICE_ID_SET_TIMING_PACKETS              0x0B /**< Service ID: Set timing packets */
#define DLT_SERVICE_ID_GET_LOCAL_TIME                  0x0C /**< Service ID: Get local time */
#define DLT_SERVICE_ID_USE_ECU_ID                      0x0D /**< Service ID: Use ECU id */
#define DLT_SERVICE_ID_USE_SESSION_ID                  0x0E /**< Service ID: Use session id */
#define DLT_SERVICE_ID_USE_TIMESTAMP                   0x0F /**< Service ID: Use timestamp */
#define DLT_SERVICE_ID_USE_EXTENDED_HEADER             0x10 /**< Service ID: Use extended header */
#define DLT_SERVICE_ID_SET_DEFAULT_LOG_LEVEL           0x11 /**< Service ID: Set default log level */
#define DLT_SERVICE_ID_SET_DEFAULT_TRACE_STATUS        0x12 /**< Service ID: Set default trace status */
#define DLT_SERVICE_ID_GET_SOFTWARE_VERSION            0x13 /**< Service ID: Get software version */
#define DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW         0x14 /**< Service ID: Message buffer overflow */
#define DLT_SERVICE_ID_UNREGISTER_CONTEXT             0xf01 /**< Service ID: Message unregister context */
#define DLT_SERVICE_ID_CONNECTION_INFO                0xf02 /**< Service ID: Message connection info */
#define DLT_SERVICE_ID_TIMEZONE						  0xf03 /**< Service ID: Timezone */
#define DLT_SERVICE_ID_MARKER						  0xf04 /**< Service ID: Timezone */
#define DLT_SERVICE_ID_CALLSW_CINJECTION              0xFFF /**< Service ID: Message Injection (minimal ID) */

/*
 * Definitions of DLT service response status
 */
#define DLT_SERVICE_RESPONSE_OK            0x00 /**< Control message response: OK */
#define DLT_SERVICE_RESPONSE_NOT_SUPPORTED 0x01 /**< Control message response: Not supported */
#define DLT_SERVICE_RESPONSE_ERROR         0x02 /**< Control message response: Error */

/*
 * Definitions of DLT service connection state
 */
#define DLT_CONNECTION_STATUS_DISCONNECTED 		0x01 /**< Client is disconnected */
#define DLT_CONNECTION_STATUS_CONNECTED 		0x02 /**< Client is connected */

/**
  \}
*/

#endif /* DLT_PROTOCOL_H */

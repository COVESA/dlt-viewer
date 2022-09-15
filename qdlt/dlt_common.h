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
 * \file dlt_common.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_common.h                                                  **
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
 * $LastChangedRevision: 1613 $
 * $LastChangedDate: 2011-02-28 14:54:27 +0100 (Mo, 28. Feb 2011) $
 * $LastChangedBy$
 Initials    Date         Comment
 aw          13.01.2010   initial
 */
#ifndef DLT_COMMON_H
#define DLT_COMMON_H

#include "export_rules.h"

/**
  \defgroup commonapi DLT Common API
  \addtogroup commonapi
  \{
*/

#include <stdio.h>

#if !defined(_MSC_VER)
#include <unistd.h>
#include <time.h>
#endif

#if !defined (__WIN32__) && !defined(_MSC_VER)
#include <termios.h>
#endif

#include "dlt_types.h"
#include "dlt_protocol.h"

#if !defined (PACKED)
#define PACKED __attribute__((aligned(1),packed))
#endif

#if defined (__MSDOS__) || defined (_MSC_VER)
/* set instead /Zp8 flag in Visual C++ configuration */
#undef PACKED
#define PACKED
#endif

/*
 * Macros to swap the byte order.
 */
#define DLT_SWAP_64(value) ((((uint64_t)DLT_SWAP_32((value)&0xffffffffull))<<32) | (DLT_SWAP_32((value)>>32)))

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
#define DLT_HTOBE_16(x) ((x))
#define DLT_HTOLE_16(x) DLT_SWAP_16((x))
#define DLT_BETOH_16(x) ((x))
#define DLT_LETOH_16(x) DLT_SWAP_16((x))

#define DLT_HTOBE_32(x) ((x))
#define DLT_HTOLE_32(x) DLT_SWAP_32((x))
#define DLT_BETOH_32(x) ((x))
#define DLT_LETOH_32(x) DLT_SWAP_32((x))

#define DLT_HTOBE_64(x) ((x))
#define DLT_HTOLE_64(x) DLT_SWAP_64((x))
#define DLT_BETOH_64(x) ((x))
#define DLT_LETOH_64(x) DLT_SWAP_64((x))
#else
/* #warning "Litte Endian Architecture!" */
#define DLT_HTOBE_16(x) DLT_SWAP_16((x))
#define DLT_HTOLE_16(x) ((x))
#define DLT_BETOH_16(x) DLT_SWAP_16((x))
#define DLT_LETOH_16(x) ((x))

#define DLT_HTOBE_32(x) DLT_SWAP_32((x))
#define DLT_HTOLE_32(x) ((x))
#define DLT_BETOH_32(x) DLT_SWAP_32((x))
#define DLT_LETOH_32(x) ((x))

#define DLT_HTOBE_64(x) DLT_SWAP_64((x))
#define DLT_HTOLE_64(x) ((x))
#define DLT_BETOH_64(x) DLT_SWAP_64((x))
#define DLT_LETOH_64(x) ((x))
#endif

#define DLT_ENDIAN_GET_16(htyp,x) ((((htyp) & DLT_HTYP_MSBF)>0)?DLT_BETOH_16(x):DLT_LETOH_16(x))
#define DLT_ENDIAN_GET_32(htyp,x) ((((htyp) & DLT_HTYP_MSBF)>0)?DLT_BETOH_32(x):DLT_LETOH_32(x))
#define DLT_ENDIAN_GET_64(htyp,x) ((((htyp) & DLT_HTYP_MSBF)>0)?DLT_BETOH_64(x):DLT_LETOH_64(x))

#if defined (__WIN32__) || defined (_MSC_VER)
#define	LOG_EMERG	0
#define	LOG_ALERT	1
#define	LOG_CRIT	2
#define	LOG_ERR		3
#define	LOG_WARNING	4
#define	LOG_NOTICE	5
#define	LOG_INFO	6
#define	LOG_DEBUG	7

#define	LOG_PID		0x01
#define	LOG_DAEMON	(3<<3)
#endif

/**
 * The standard TCP Port used for DLT daemon
 */
#define DLT_DAEMON_TCP_PORT 3490
#define DLT_DAEMON_UDP_PORT 3490

/**
 * The size of a DLT ID
 */
#define DLT_ID_SIZE 4

#define DLT_SIZE_WEID DLT_ID_SIZE
#define DLT_SIZE_WSID (sizeof(uint32_t))
#define DLT_SIZE_WTMS (sizeof(uint32_t))

/**
 * Get the size of extra header parameters, depends on htyp.
 */
#define DLT_STANDARD_HEADER_EXTRA_SIZE(htyp) ( (DLT_IS_HTYP_WEID(htyp) ? DLT_SIZE_WEID : 0) + (DLT_IS_HTYP_WSID(htyp) ? DLT_SIZE_WSID : 0) + (DLT_IS_HTYP_WTMS(htyp) ? DLT_SIZE_WTMS : 0) )


#if defined (__MSDOS__) || defined (_MSC_VER)
#define __func__ __FUNCTION__
#endif

#define PRINT_FUNCTION_VERBOSE(_verbose)  \
{ \
	static char _strbuf[255]; \
    \
    if(_verbose) \
    { \
		sprintf(_strbuf, "%s()\n",__func__); \
		dlt_log(LOG_INFO, _strbuf); \
    } \
}

#ifndef NULL
#define NULL (char*)0
#endif

#define DLT_MSG_IS_CONTROL(MSG)          ((DLT_IS_HTYP_UEH((MSG)->standardheader->htyp)) && \
                                          (DLT_GET_MSIN_MSTP((MSG)->extendedheader->msin)==DLT_TYPE_CONTROL))

#define DLT_MSG_IS_CONTROL_REQUEST(MSG)  ((DLT_IS_HTYP_UEH((MSG)->standardheader->htyp)) && \
                                          (DLT_GET_MSIN_MSTP((MSG)->extendedheader->msin)==DLT_TYPE_CONTROL) && \
                                          (DLT_GET_MSIN_MTIN((MSG)->extendedheader->msin)==DLT_CONTROL_REQUEST))

#define DLT_MSG_IS_CONTROL_RESPONSE(MSG) ((DLT_IS_HTYP_UEH((MSG)->standardheader->htyp)) && \
                                          (DLT_GET_MSIN_MSTP((MSG)->extendedheader->msin)==DLT_TYPE_CONTROL) && \
                                          (DLT_GET_MSIN_MTIN((MSG)->extendedheader->msin)==DLT_CONTROL_RESPONSE))

#define DLT_MSG_IS_CONTROL_TIME(MSG)     ((DLT_IS_HTYP_UEH((MSG)->standardheader->htyp)) && \
                                          (DLT_GET_MSIN_MSTP((MSG)->extendedheader->msin)==DLT_TYPE_CONTROL) && \
                                          (DLT_GET_MSIN_MTIN((MSG)->extendedheader->msin)==DLT_CONTROL_TIME))

#define DLT_MSG_IS_NW_TRACE(MSG)         ((DLT_IS_HTYP_UEH((MSG)->standardheader->htyp)) && \
									      (DLT_GET_MSIN_MSTP((MSG)->extendedheader->msin)==DLT_TYPE_NW_TRACE))

#define DLT_MSG_IS_TRACE_MOST(MSG)       ((DLT_IS_HTYP_UEH((MSG)->standardheader->htyp)) && \
									      (DLT_GET_MSIN_MSTP((MSG)->extendedheader->msin)==DLT_TYPE_NW_TRACE) && \
									      (DLT_GET_MSIN_MTIN((MSG)->extendedheader->msin)==DLT_NW_TRACE_MOST))

#define DLT_MSG_IS_NONVERBOSE(MSG)       (!(DLT_IS_HTYP_UEH((MSG)->standardheader->htyp)) || \
                                          ((DLT_IS_HTYP_UEH((MSG)->standardheader->htyp)) && (!(DLT_IS_MSIN_VERB((MSG)->extendedheader->msin)))))

/*

 * Definitions of DLT message buffer overflow
 */
#define DLT_MESSAGE_BUFFER_NO_OVERFLOW     0x00 /**< Buffer overflow has not occured */
#define DLT_MESSAGE_BUFFER_OVERFLOW        0x01 /**< Buffer overflow has occured */

/*
 * Definition of DLT output variants
 */
#define DLT_OUTPUT_HEX              1
#define DLT_OUTPUT_ASCII            2
#define DLT_OUTPUT_MIXED_FOR_PLAIN  3
#define DLT_OUTPUT_MIXED_FOR_HTML   4
#define DLT_OUTPUT_ASCII_LIMITED    5

#define DLT_FILTER_MAX 30 /**< Maximum number of filters */

#define DLT_MSG_READ_VALUE(dst,src,length,type) \
    { \
    if((length<0) || ((length)<((int32_t)sizeof(type)))) \
        { length = -1; } \
    else \
        { dst = *((type*)src);src+=sizeof(type);length-=sizeof(type); } \
    }

#define DLT_MSG_READ_ID(dst,src,length) \
    { \
    if((length<0) || ((length)<DLT_ID_SIZE)) \
        { length = -1; } \
    else \
        { memcpy(dst,src,DLT_ID_SIZE);src+=DLT_ID_SIZE;length-=DLT_ID_SIZE; } \
    }

#define DLT_MSG_READ_STRING(dst,src,maxlength,length) \
    { \
    if(((maxlength)<0) || ((length)<0) || ((maxlength)<(length))) \
        { maxlength = -1; } \
    else \
        { memcpy(dst,src,length);dlt_clean_string(dst,length);dst[length]=0; \
	  src+=length;maxlength-=length; } \
    }

#define DLT_MSG_READ_NULL(src,maxlength,length) \
    { \
    if(((maxlength)<0) || ((length)<0) || ((maxlength)<(length))) \
        { length = -1; } \
    else \
        { src+=length;maxlength-=length; } \
    }

#define DLT_HEADER_SHOW_NONE	   0x0000
#define DLT_HEADER_SHOW_TIME       0x0001
#define DLT_HEADER_SHOW_TMSTP      0x0002
#define DLT_HEADER_SHOW_MSGCNT     0x0004
#define DLT_HEADER_SHOW_ECUID      0x0008
#define DLT_HEADER_SHOW_APID       0x0010
#define DLT_HEADER_SHOW_CTID       0x0020
#define DLT_HEADER_SHOW_MSGTYPE    0x0040
#define DLT_HEADER_SHOW_MSGSUBTYPE 0x0080
#define DLT_HEADER_SHOW_VNVSTATUS  0x0100
#define DLT_HEADER_SHOW_NOARG      0x0200
#define DLT_HEADER_SHOW_ALL        0xFFFF

/**
 * The definition of the serial header containing the characters "DLS" + 0x01.
 */
#ifdef __cplusplus
extern "C" {
#endif
QDLT_C_EXPORT extern const char dltSerialHeader[DLT_ID_SIZE];
#ifdef __cplusplus
}
#endif

/**
 * The definition of the serial header containing the characters "DLS" + 0x01 as char.
 */
QDLT_C_EXPORT extern char dltSerialHeaderChar[DLT_ID_SIZE];

/**

 * The type of a DLT ID (context id, application id, etc.)
 */
typedef char ID4[DLT_ID_SIZE];

/**
 * The structure of the DLT file storage header. This header is used before each stored DLT message.
 */
typedef struct
{
    char pattern[DLT_ID_SIZE];		/**< This pattern should be DLT0x01 */
    uint32_t seconds;			    /**< seconds since 1.1.1970 */
    int32_t microseconds;			/**< Microseconds */
    char ecu[DLT_ID_SIZE];			/**< The ECU id is added, if it is not already in the DLT message itself */
} PACKED DltStorageHeader;

/**
 * The structure of the DLT standard header. This header is used in each DLT message.
 */
typedef struct
{
    uint8_t htyp;           /**< This parameter contains several informations, see definitions below */
    uint8_t mcnt;           /**< The message counter is increased with each sent DLT message */
    uint16_t len;           /**< Length of the complete message, without storage header */
} PACKED DltStandardHeader;

/**
 * The structure of the DLT extra header parameters. Each parameter is sent only if enabled in htyp.
 */
typedef struct
{
    char ecu[DLT_ID_SIZE];       /**< ECU id */
    uint32_t seid;     /**< Session number */
    uint32_t tmsp;     /**< Timestamp since system start in 0.1 milliseconds */
} PACKED DltStandardHeaderExtra;

/**
 * The structure of the DLT extended header. This header is only sent if enabled in htyp parameter.
 */
typedef struct
{
    uint8_t msin;          /**< messsage info */
    uint8_t noar;          /**< number of arguments */
    char apid[DLT_ID_SIZE];          /**< application id */
    char ctid[DLT_ID_SIZE];          /**< context id */
} PACKED DltExtendedHeader;

/**
 * The structure to organise the DLT messages.
 * This structure is used by the corresponding functions.
 */
typedef struct sDltMessage
{
    /* flags */
    int8_t found_serialheader;

    /* offsets */
    int32_t resync_offset;

    /* size parameters */
    int32_t headersize;    /**< size of complete header including storage header */
    int32_t datasize;      /**< size of complete payload */

    /* buffer for current loaded message */
    uint8_t headerbuffer[sizeof(DltStorageHeader)+
                         sizeof(DltStandardHeader)+sizeof(DltStandardHeaderExtra)+sizeof(DltExtendedHeader)]; /**< buffer for loading complete header */
    uint8_t *databuffer;         /**< buffer for loading payload */

    /* header values of current loaded message */
    DltStorageHeader       *storageheader;  /**< pointer to storage header of current loaded header */
    DltStandardHeader      *standardheader; /**< pointer to standard header of current loaded header */
    DltStandardHeaderExtra headerextra;     /**< extra parameters of current loaded header */
    DltExtendedHeader      *extendedheader; /**< pointer to extended of current loaded header */
} DltMessage;

/**
 * The structure of the DLT Service Get Log Info.
 */
typedef struct
{
    uint32_t service_id;            /**< service ID */
    uint8_t options;                /**< type of request */
    char apid[DLT_ID_SIZE];                   /**< application id */
    char ctid[DLT_ID_SIZE];                   /**< context id */
    char com[DLT_ID_SIZE];                    /**< communication interface */
} PACKED DltServiceGetLogInfoRequest;

/**
 * The structure of the DLT Service Set Log Level.
 */
typedef struct
{
    uint32_t service_id;            /**< service ID */
    char apid[DLT_ID_SIZE];                   /**< application id */
    char ctid[DLT_ID_SIZE];                   /**< context id */
    uint8_t log_level;              /**< log level to be set */
    char com[DLT_ID_SIZE];                    /**< communication interface */
} PACKED DltServiceSetLogLevel;

/**
 * The structure of the DLT Service Set Default Log Level.
 */
typedef struct
{
    uint32_t service_id;            /**< service ID */
    uint8_t log_level;              /**< default log level to be set */
    char com[DLT_ID_SIZE];                    /**< communication interface */
} PACKED DltServiceSetDefaultLogLevel;

/**
 * The structure of the DLT Service Set Verbose Mode
 */
typedef struct
{
    uint32_t service_id;            /**< service ID */
    uint8_t new_status;             /**< new status to be set */
} PACKED DltServiceSetVerboseMode;

/**
 * The structure of the DLT Service Set Communication Interface Status
 */
typedef struct
{
    uint32_t service_id;            /**< service ID */
    char com[DLT_ID_SIZE];                    /**< communication interface */
    uint8_t new_status;             /**< new status to be set */
} PACKED DltServiceSetCommunicationInterfaceStatus;

/**
 * The structure of the DLT Service Set Communication Maximum Bandwidth
 */
typedef struct
{
    uint32_t service_id;            /**< service ID */
    char com[DLT_ID_SIZE];                    /**< communication interface */
    uint32_t max_bandwidth;         /**< maximum bandwith */
} PACKED DltServiceSetCommunicationMaximumBandwidth;

typedef struct
{
    uint32_t service_id;            /**< service ID */
    uint8_t status;                 /**< reponse status */
} PACKED DltServiceResponse;

typedef struct
{
    uint32_t service_id;            /**< service ID */
    uint8_t status;                 /**< reponse status */
    uint8_t log_level;              /**< log level */
} PACKED DltServiceGetDefaultLogLevelResponse;

typedef struct
{
    uint32_t service_id;            /**< service ID */
    uint8_t status;                 /**< reponse status */
    uint8_t overflow;               /**< overflow status */
} PACKED DltServiceMessageBufferOverflowResponse;

typedef struct
{
    uint32_t service_id;            /**< service ID */
} PACKED DltServiceGetSoftwareVersion;

typedef struct
{
    uint32_t service_id;            /**< service ID */
    uint8_t  status;                /**< reponse status */
    uint32_t length;                /**< length of following payload */
    /* char [] payload */
} PACKED DltServiceGetSoftwareVersionResponse;

/**
 * The structure of the DLT Service Unregister Context.
 */
typedef struct
{
    uint32_t service_id;            /**< service ID */
    uint8_t status;                 /**< reponse status */
    char apid[DLT_ID_SIZE];         /**< application id */
    char ctid[DLT_ID_SIZE];         /**< context id */
    char comid[DLT_ID_SIZE];        /**< communication interface */
} PACKED DltServiceUnregisterContext;

/**
 * The structure of the DLT Service Connection Info
 */
typedef struct
{
    uint32_t service_id;            /**< service ID */
    uint8_t status;                 /**< reponse status */
    uint8_t state;                  /**< new state */
    char comid[DLT_ID_SIZE];        /**< communication interface */
} PACKED DltServiceConnectionInfo;

/**
 * The structure of the DLT Service Timezone
 */
typedef struct
{
    uint32_t service_id;            /**< service ID */
    uint8_t status;                 /**< reponse status */
    int32_t timezone;               /**< Timezone in seconds */
    uint8_t isdst;                  /**< Is daylight saving time */
} PACKED DltServiceTimezone;

/**
 * The structure of the DLT Service Marker
 */
typedef struct
{
    uint32_t service_id;            /**< service ID */
    uint8_t status;                 /**< reponse status */
} PACKED DltServiceMarker;

/**
 * Structure to store filter parameters.
 * ID are maximal four characters. Unused values are filled with zeros.
 * If every value as filter is valid, the id should be empty by having only zero values.
 */
typedef struct
{
    char apid[DLT_FILTER_MAX][DLT_ID_SIZE]; /**< application id */
    char ctid[DLT_FILTER_MAX][DLT_ID_SIZE]; /**< context id */
    int  counter;                           /**< number of filters */
} DltFilter;

/**
 * The structure to organise the access to DLT files.
 * This structure is used by the corresponding functions.
 */
typedef struct sDltFile
{
    /* file handle and index for fast access */
    FILE *handle;      /**< file handle of opened DLT file */
    long *index;       /**< file positions of all DLT messages for fast access to file, only filtered messages */

    /* size parameters */
    int32_t counter;       /**< number of messages in DLT file with filter */
    int32_t counter_total; /**< number of messages in DLT file without filter */
    int32_t position;      /**< current index to message parsed in DLT file starting at 0 */
    long file_length;  /**< length of the file */
    long file_position; /**< current position in the file */

    /* error counters */
    int32_t error_messages; /**< number of incomplete DLT messages found during file parsing */

    /* filter parameters */
    DltFilter *filter;  /**< pointer to filter list. Zero if no filter is set. */
    int32_t filter_counter; /**< number of filter set */

    /* current loaded message */
    DltMessage msg;     /**< pointer to message */

} DltFile;

/**
 * The structure is used to organise the receiving of data
 * including buffer handling.
 * This structure is used by the corresponding functions.
 */
typedef struct
{
    int32_t lastBytesRcvd;    /**< bytes received in last receive call */
    int32_t bytesRcvd;        /**< received bytes */
    int32_t totalBytesRcvd;   /**< total number of received bytes */
    char *buffer;         /**< pointer to receiver buffer */
    char *buf;            /**< pointer to position within receiver buffer */
    int fd;               /**< connection handle */
    int32_t buffersize;       /**< size of receiver buffer */
} DltReceiver;

typedef struct
{
    char*   buffer;         /**< Ptr. to buffer */
    uint32_t    size;       /**< Maximum size of buffer */
    uint32_t    pos_write;  /**< current writing position in bytes*/
    uint32_t    pos_read;   /**< current reading position in bytes*/
    uint32_t    count;      /**< nr. of entries */
} DltRingBuffer;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Helper function to print a byte array in hex.
     * @param ptr pointer to the byte array.
     * @param size number of bytes to be printed.
     */
    QDLT_C_EXPORT void dlt_print_hex(uint8_t *ptr,int size);
    /**
     * Helper function to print a byte array in hex into a string.
     * @param text pointer to a ASCII string, in which the text is written
     * @param textlength maximal size of text buffer
     * @param ptr pointer to the byte array.
     * @param size number of bytes to be printed.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_print_hex_string(char *text,int textlength,uint8_t *ptr,int size);
    /**
     * Helper function to print a byte array in hex and ascii into a string.
     * @param text pointer to a ASCII string, in which the text is written
     * @param textlength maximal size of text buffer
     * @param ptr pointer to the byte array.
     * @param size number of bytes to be printed.
     * @param html output is html? 0 - false, 1 - true
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_print_mixed_string(char *text,int textlength,uint8_t *ptr,int size,int html);
    /**
     * Helper function to print a byte array in ascii into a string.
     * @param text pointer to a ASCII string, in which the text is written
     * @param textlength maximal size of text buffer
     * @param ptr pointer to the byte array.
     * @param size number of bytes to be printed.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_print_char_string(char **text,int textlength,uint8_t *ptr,int size);

    /**
     * Helper function to print an id.
     * @param text pointer to ASCII string where to write the id
     * @param id four byte char array as used in DLT messages as IDs.
     */
    QDLT_C_EXPORT void dlt_print_id(char *text,const char *id);

    /**
     * Helper function to set an ID parameter.
     * @param id four byte char array as used in DLT messages as IDs.
     * @param text string to be copied into char array.
     */
    QDLT_C_EXPORT void dlt_set_id(char *id,const char *text);

    /**
     * Helper function to remove not nice to print characters, e.g. NULL or carriage return.
     * @param text pointer to string to be cleaned.
     * @param length length of string excluding terminating zero.
     */
    QDLT_C_EXPORT void dlt_clean_string(char *text,int length);

    /**
     * Initialise the filter list.
     * This function must be called before using further dlt filter.
     * @param filter pointer to structure of organising DLT filter
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_filter_init(DltFilter *filter,int verbose);
    /**
     * Free the used memory by the organising structure of filter.
     * @param filter pointer to structure of organising DLT filter
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_filter_free(DltFilter *filter,int verbose);
    /**
     * Load filter list from file.
     * @param filter pointer to structure of organising DLT filter
     * @param filename name of the file to load filters from
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_filter_load(DltFilter *filter,const char *filename,int verbose);
    /**
     * Save filter list to file.
     * @param filter pointer to structure of organising DLT filter
     * @param filename name of the file to load filters from
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_filter_save(DltFilter *filter,const char *filename,int verbose);
    /**
     * Find index of filter in filter list
     * @param filter pointer to structure of organising DLT filter
     * @param apid application id to be found in filter list
     * @param ctid context id to be found in filter list
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error (or not found), else return index of filter
     */
    QDLT_C_EXPORT int dlt_filter_find(DltFilter *filter,const char *apid,const char *ctid, int verbose);
    /**
     * Add new filter to filter list.
     * @param filter pointer to structure of organising DLT filter
     * @param apid application id to be added to filter list (must always be set).
     * @param ctid context id to be added to filter list. empty equals don't care.
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_filter_add(DltFilter *filter,const char *apid,const char *ctid,int verbose);
    /**
     * Delete filter from filter list
     * @param filter pointer to structure of organising DLT filter
     * @param apid application id to be deleted from filter list
     * @param ctid context id to be deleted from filter list
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_filter_delete(DltFilter *filter,const char *apid,const char *ctid,int verbose);

    /**
     * Initialise the structure used to access a DLT message.
     * This function must be called before using further dlt_message functions.
     * @param msg pointer to structure of organising access to DLT messages
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_init(DltMessage *msg,int verbose);
    /**
     * Free the used memory by the organising structure of file.
     * @param msg pointer to structure of organising access to DLT messages
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_free(DltMessage *msg,int verbose);
    /**
     * Print Header into an ASCII string.
     * This function calls dlt_message_header_flags() with flags=DLT_HEADER_SHOW_ALL
     * @param msg pointer to structure of organising access to DLT messages
     * @param text pointer to a ASCII string, in which the header is written
     * @param textlength maximal size of text buffer
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_header(DltMessage *msg,char *text,int textlength,int verbose);
    /**
     * Print Header into an ASCII string, selective.
     * @param msg pointer to structure of organising access to DLT messages
     * @param text pointer to a ASCII string, in which the header is written
     * @param textlength maximal size of text buffer
     * @param flags select, bit-field to select, what should be printed (DLT_HEADER_SHOW_...)
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_header_flags(DltMessage *msg,char *text,int textlength,int flags, int verbose);
    /**
     * Print Payload into an ASCII string.
     * @param msg pointer to structure of organising access to DLT messages
     * @param text pointer to a ASCII string, in which the header is written
     * @param textlength maximal size of text buffer
     * @param type 1 = payload as hex, 2 = payload as ASCII.
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_payload(DltMessage *msg,char *text,int textlength,int type,int verbose);
    /**
     * Check if message is filtered or not. All filters are applied (logical OR).
     * @param msg pointer to structure of organising access to DLT messages
     * @param filter pointer to filter
     * @param verbose if set to true verbose information is printed out.
     * @return 1 = filter matches, 0 = filter does not match, negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_filter_check(DltMessage *msg,DltFilter *filter,int verbose);

    /**
     * Read message from memory buffer.
     * Message in buffer has no storage header.
     * @param msg pointer to structure of organising access to DLT messages
     * @param buffer pointer to memory buffer
     * @param length length of message in buffer
     * @param resync if set to true re-sync to serial header is enforced
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_read(DltMessage *msg,uint8_t *buffer,unsigned int length,int resync,int verbose);

    /**
     * Get standard header extra parameters
     * @param msg pointer to structure of organising access to DLT messages
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_get_extraparameters(DltMessage *msg,int verbose);

    /**
     * Set standard header extra parameters
     * @param msg pointer to structure of organising access to DLT messages
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_set_extraparameters(DltMessage *msg,int verbose);

    /**
     * Initialise the structure used to access a DLT file.
     * This function must be called before using further dlt_file functions.
     * @param file pointer to structure of organising access to DLT file
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_file_init(DltFile *file,int verbose);
    /**
     * Set a list to filters.
     * This function should be called before loading a DLT file, if filters should be used.
     * A filter list is an array of filters. Several filters are combined logically by or operation.
     * The filter list is not copied, so take care to keep list in memory.
     * @param file pointer to structure of organising access to DLT file
     * @param filter pointer to filter list array
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_file_set_filter(DltFile *file,DltFilter *filter,int verbose);
    /**
     * Initialising loading a DLT file.
     * @param file pointer to structure of organising access to DLT file
     * @param filename filename of DLT file
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_file_open(DltFile *file,const char *filename,int verbose);
    /**
     * Find next message in the DLT file and parse them.
     * This function finds the next message in the DLT file.
     * If a filter is set, the filter list is used.
     * @param file pointer to structure of organising access to DLT file
     * @param verbose if set to true verbose information is printed out.
     * @return 0 = message does not match filter, 1 = message was read, negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_file_read(DltFile *file,int verbose);
    /**
     * Find next message in the DLT file in RAW format (without storage header) and parse them.
     * This function finds the next message in the DLT file.
     * If a filter is set, the filter list is used.
     * @param file pointer to structure of organising access to DLT file
     * @param resync Resync to serial header when set to true
     * @param verbose if set to true verbose information is printed out.
     * @return 0 = message does not match filter, 1 = message was read, negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_file_read_raw(DltFile *file,int resync,int verbose);
    /**
     * Closing loading a DLT file.
     * @param file pointer to structure of organising access to DLT file
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_file_close(DltFile *file,int verbose);
    /**
     * Load standard header of a message from file
     * @param file pointer to structure of organising access to DLT file
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_file_read_header(DltFile *file,int verbose);
    /**
     * Load standard header of a message from file in RAW format (without storage header)
     * @param file pointer to structure of organising access to DLT file
     * @param resync Resync to serial header when set to true
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_file_read_header_raw(DltFile *file,int resync,int verbose);
    /**
     * Load, if available in message, extra standard header fields and
     * extended header of a message from file
     * (dlt_file_read_header() must have been called before this call!)
     * @param file pointer to structure of organising access to DLT file
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_file_read_header_extended(DltFile *file, int verbose);
    /**
     * Load payload of a message from file
     * (dlt_file_read_header() must have been called before this call!)
     * @param file pointer to structure of organising access to DLT file
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_file_read_data(DltFile *file, int verbose);
    /**
     * Load headers and payload of a message selected by the index.
     * If filters are set, index is based on the filtered list.
     * @param file pointer to structure of organising access to DLT file
     * @param index position of message in the files beginning from zero
     * @param verbose if set to true verbose information is printed out.
     * @return number of messages loaded, negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_file_message(DltFile *file,int index,int verbose);
    /**
     * Free the used memory by the organising structure of file.
     * @param file pointer to structure of organising access to DLT file
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_file_free(DltFile *file,int verbose);

    /**
     * Initialize (external) logging facility
     * @param mode positive, if syslog should be used; zero for console output
     */
    QDLT_C_EXPORT void dlt_log_init(int mode);
    /**
     * Log ASCII string with null-termination to (external) logging facility
     * @param prio priority (see syslog() call)
     * @param s Pointer to ASCII string with null-termination
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_log(int prio, char *s);
    /**
     * De-Initialize (external) logging facility
     */
    QDLT_C_EXPORT void dlt_log_free(void);

    /**
     * Initialising a dlt receiver structure
     * @param receiver pointer to dlt receiver structure
     * @param _fd handle to file/socket/fifo, from which the data should be received
     * @param _buffersize size of data buffer for storing the received data
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_receiver_init(DltReceiver *receiver,int _fd, int _buffersize);
    /**
     * De-Initialize a dlt receiver structure
     * @param receiver pointer to dlt receiver structure
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_receiver_free(DltReceiver *receiver);
    /**
     * Receive data from socket using the dlt receiver structure
     * @param receiver pointer to dlt receiver structure
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_receiver_receive_socket(DltReceiver *receiver);
    /**
     * Receive data from file/fifo using the dlt receiver structure
     * @param receiver pointer to dlt receiver structure
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_receiver_receive_fd(DltReceiver *receiver);
    /**
     * Remove a specific size of bytes from the received data
     * @param receiver pointer to dlt receiver structure
     * @param size amount of bytes to be removed
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_receiver_remove(DltReceiver *receiver,int size);
    /**
     * Move data from last receive call to front of receive buffer
     * @param receiver pointer to dlt receiver structure
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_receiver_move_to_begin(DltReceiver *receiver);

    /**
     * Fill out storage header of a dlt message
     * @param storageheader pointer to storage header of a dlt message
     * @param ecu name of ecu to be set in storage header
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_set_storageheader(DltStorageHeader *storageheader, const char *ecu);
    /**
     * Check if a storage header contains its marker
     * @param storageheader pointer to storage header of a dlt message
     * @return 0 no, 1 yes, negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_check_storageheader(DltStorageHeader *storageheader);

    /**
     * Initialize ringbuffer of with a maximum size of size
     * @param dltbuf Pointer to ringbuffer structure
     * @param size Maximum size of buffer in bytes
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_ringbuffer_init(DltRingBuffer *dltbuf, uint32_t size);

    /**
     * Release and free memory used by ringbuffer
     * @param dltbuf Pointer to ringbuffer structure
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_ringbuffer_free(DltRingBuffer *dltbuf);

    /**
     * Write one entry to ringbuffer
     * @param dltbuf Pointer to ringbuffer structure
     * @param data Pointer to data to be written to ringbuffer
     * @param size Size of data in bytes to be written to ringbuffer
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_ringbuffer_put(DltRingBuffer *dltbuf, void *data, uint32_t size);

    /**
     * Write one entry given as 3 chunks to ringbuffer
     * @param dltbuf Pointer to ringbuffer structure
     * @param data1 Pointer to data1 to be written to ringbuffer
     * @param size1 Size of data1 in bytes to be written to ringbuffer
     * @param data2 Pointer to data2 to be written to ringbuffer
     * @param size2 Size of data2 in bytes to be written to ringbuffer
     * @param data3 Pointer to data3 to be written to ringbuffer
     * @param size3 Size of data3 in bytes to be written to ringbuffer
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_ringbuffer_put3(DltRingBuffer *dltbuf, void *data1, uint32_t size1, void *data2, uint32_t size2, void *data3, uint32_t size3);

    /**
     * Read one entry from ringbuffer
     * @param dltbuf Pointer to ringbuffer structure
     * @param data Pointer to data read from ringbuffer
     * @param size Size of read data in bytes from ringbuffer
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_ringbuffer_get(DltRingBuffer *dltbuf, void *data, size_t *size);

    /**
     * Helper function: Skip one readable entry in ringbuffer
     * @param dltbuf Pointer to ringbuffer structure
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_ringbuffer_get_skip(DltRingBuffer *dltbuf);

    /**
     * Helper function: Get free space in bytes for writting between write and read position
     * @param dltbuf Pointer to ringbuffer structure
     * @param freespace Free Space in bytes for writting is returned
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_ringbuffer_freespacewrite(DltRingBuffer *dltbuf, uint32_t *freespace);

    /**
     * Helper function: Check free space and if necessary discard entries, so that at least
     * reqspace bytes are available for writting
     * @param dltbuf Pointer to ringbuffer structure
     * @param reqspace Requested space for writting in bytes
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_ringbuffer_checkandfreespace(DltRingBuffer *dltbuf, uint32_t reqspace);

#if !defined (__WIN32__)

    /**
     * Helper function: Setup serial connection
     * @param fd File descriptor of serial tty device
     * @param speed Serial line speed, as defined in termios.h
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_setup_serial(int fd, speed_t speed);

    /**
     * Helper function: Convert serial line baudrate (as number) to line speed (as defined in termios.h)
     * @param baudrate Serial line baudrate (as number)
     * @return Serial line speed, as defined in termios.h
     */
    QDLT_C_EXPORT speed_t dlt_convert_serial_speed(int baudrate);

    /**
     * Print dlt version and dlt svn version to buffer
     * @param buf Pointer to buffer
     */
    QDLT_C_EXPORT void dlt_get_version(char *buf);

#endif

    /* Function prototypes which should be used only internally */
    /*                                                          */

    /**
     * Common part of initialisation
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_init_common(void);

    /**
     * Return the uptime of the system in 0.1 ms resolution
     * @return 0 if there was an error
     */
    QDLT_C_EXPORT uint32_t dlt_uptime(void);

    /**
     * Print header of a DLT message
     * @param message pointer to structure of organising access to DLT messages
     * @param text pointer to a ASCII string, in which the header is written
     * @param size maximal size of text buffer
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_print_header(DltMessage *message, char *text, uint32_t size, int verbose);

    /**
     * Print payload of a DLT message as Hex-Output
     * @param message pointer to structure of organising access to DLT messages
     * @param text pointer to a ASCII string, in which the output is written
     * @param size maximal size of text buffer
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_print_hex(DltMessage *message, char *text, uint32_t size, int verbose);

    /**
     * Print payload of a DLT message as ASCII-Output
     * @param message pointer to structure of organising access to DLT messages
     * @param text pointer to a ASCII string, in which the output is written
     * @param size maximal size of text buffer
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_print_ascii(DltMessage *message, char *text, uint32_t size, int verbose);

    /**
     * Print payload of a DLT message as Mixed-Ouput (Hex and ASCII), for plain text output
     * @param message pointer to structure of organising access to DLT messages
     * @param text pointer to a ASCII string, in which the output is written
     * @param size maximal size of text buffer
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_print_mixed_plain(DltMessage *message, char *text, uint32_t size, int verbose);

    /**
     * Print payload of a DLT message as Mixed-Output (Hex and ASCII), for HTML text output
     * @param message pointer to structure of organising access to DLT messages
     * @param text pointer to a ASCII string, in which the output is written
     * @param size maximal size of text buffer
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_print_mixed_html(DltMessage *message, char *text, uint32_t size, int verbose);

    /**
     * Decode and print a argument of a DLT message
     * @param msg pointer to structure of organising access to DLT messages
     * @param type_info Type of argument
     * @param ptr pointer to pointer to data (pointer to data is changed within this function)
     * @param datalength pointer to datalength (datalength is changed within this function)
     * @param text pointer to a ASCII string, in which the output is written
     * @param textlength maximal size of text buffer
     * @param byteLength If argument is a string, and this value is 0 or greater, this value will be taken as string length
     * @param verbose if set to true verbose information is printed out.
     * @return negative value if there was an error
     */
    QDLT_C_EXPORT int dlt_message_argument_print(DltMessage *msg,uint32_t type_info,uint8_t **ptr,int32_t *datalength,char *text,int textlength,int byteLength,int verbose);

#ifdef __cplusplus
}
#endif

/**
  \}
*/

#endif /* DLT_COMMON_H */

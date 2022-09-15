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
 * \file dlt_user.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_user.h                                                    **
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
 * $LastChangedRevision: 1533 $
 * $LastChangedDate: 2010-12-14 16:30:35 +0100 (Di, 14. Dez 2010) $
 * $LastChangedBy$
 Initials    Date         Comment
 aw          13.01.2010   initial
 */

#ifndef DLT_USER_H
#define DLT_USER_H

/**
  \defgroup userapi DLT User API
  \addtogroup userapi
  \{
*/

#include "dlt_types.h"
#include "dlt_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Definitions of DLT log level
 */
typedef enum
{
	DLT_LOG_DEFAULT =    		  -1,   /**< Default log level */
	DLT_LOG_OFF     =      		0x00,   /**< Log level off */
	DLT_LOG_FATAL   =   		0x01, 	/**< fatal system error */
	DLT_LOG_ERROR   =   		0x02, 	/**< error with impact to correct functionality */
	DLT_LOG_WARN    =   		0x03, 	/**< warning, correct behaviour could not be ensured */
	DLT_LOG_INFO    =   		0x04, 	/**< informational */
	DLT_LOG_DEBUG   =   		0x05, 	/**< debug  */
	DLT_LOG_VERBOSE =   		0x06 	/**< highest grade of information */
} DltLogLevelType;

/**
 * Definitions of DLT Format
 */
typedef enum
{
    DLT_FORMAT_DEFAULT   =          0x00,   /**< no sepecial format */
    DLT_FORMAT_HEX8  =   		0x01, 	/**< Hex 8 */
    DLT_FORMAT_HEX16 =   		0x02, 	/**< Hex 16 */
    DLT_FORMAT_HEX32 =   		0x03, 	/**< Hex 32 */
    DLT_FORMAT_HEX64 =   		0x04, 	/**< Hex 64 */
    DLT_FORMAT_BIN8  =   		0x05, 	/**< Binary 8 */
    DLT_FORMAT_BIN16 =   		0x06 	/**< Binary 16  */
} DltFormatType;

/**
 * Definitions of DLT trace status
 */
typedef enum
{
	DLT_TRACE_STATUS_DEFAULT =   -1, 	/**< Default trace status */
	DLT_TRACE_STATUS_OFF	 = 0x00, 	/**< Trace status: Off */
	DLT_TRACE_STATUS_ON      = 0x01 	/**< Trace status: On */
} DltTraceStatusType;

/**
 * Definitions for  dlt_user_trace_network/DLT_TRACE_NETWORK()
 * as defined in the DLT protocol
 */
typedef enum
{
	DLT_NW_TRACE_IPC     = 0x01, 	/**< Interprocess communication */
	DLT_NW_TRACE_CAN     = 0x02, 	/**< Controller Area Network Bus */
	DLT_NW_TRACE_FLEXRAY = 0x03, 	/**< Flexray Bus */
	DLT_NW_TRACE_MOST    = 0x04, 	/**< Media Oriented System Transport Bus */
        DLT_NW_TRACE_RESERVED0 = 0x05,
        DLT_NW_TRACE_RESERVED1 = 0x06,
        DLT_NW_TRACE_RESERVED2 = 0x07,
        DLT_NW_TRACE_USER_DEFINED0 = 0x08,
        DLT_NW_TRACE_USER_DEFINED1 = 0x09,
        DLT_NW_TRACE_USER_DEFINED2 = 0x0A,
        DLT_NW_TRACE_USER_DEFINED3 = 0x0B,
        DLT_NW_TRACE_USER_DEFINED4 = 0x0C,
        DLT_NW_TRACE_USER_DEFINED5 = 0x0D,
        DLT_NW_TRACE_USER_DEFINED6 = 0x0E,
        DLT_NW_TRACE_USER_DEFINED7 = 0x0F
} DltNetworkTraceType;

#define DLT_USER_BUF_MAX_SIZE 2048               /**< maximum size of each user buffer, also used for injection buffer */

/* Use a semaphore or mutex from your OS to prevent concurrent access to the DLT buffer. */
#define DLT_SEM_LOCK() { sem_wait(&dlt_mutex); }
#define DLT_SEM_FREE() { sem_post(&dlt_mutex); }

/**
 * This structure is used for every context used in an application.
 */
typedef struct
{
	char contextID[4];                            /**< context id */
    int32_t log_level_pos;                        /**< offset in user-application context field */
} DltContext;

/**
 * This structure is used for context data used in an application.
 */
typedef struct
{
    DltContext *handle;                           /**< pointer to DltContext */
	unsigned char buffer[DLT_USER_BUF_MAX_SIZE];  /**< buffer for building log message*/
	int32_t size;                                 /**< payload size */
	int32_t log_level;                            /**< log level */
	int32_t trace_status;                         /**< trace status */
	int32_t args_num;                             /**< number of arguments for extended header*/
	uint8_t mcnt;                                 /**< message counter */
	char* context_description;                    /**< description of context */
} DltContextData;

typedef struct
{
	uint32_t service_id;
	int (*injection_callback)(uint32_t service_id, void *data, uint32_t length);
} DltUserInjectionCallback;

/**
 * This structure is used in a table managing all contexts and the corresponding log levels in an application.
 */
typedef struct
{
    char contextID[DLT_ID_SIZE];      /**< Context ID */
    int8_t log_level;                 /**< Log level */
    int8_t trace_status;              /**< Trace status */
    char *context_description;        /**< description of context */
    DltUserInjectionCallback *injection_table; /**< Table with pointer to injection functions and service ids */
    uint32_t nrcallbacks;
} dlt_ll_ts_type;

/**
 * This structure is used once for one application.
 */
typedef struct
{
    char ecuID[DLT_ID_SIZE];             /**< ECU ID */
    char appID[DLT_ID_SIZE];             /**< Application ID */
    int dlt_log_handle;                  /**< Handle to fifo of dlt daemon */
    int dlt_user_handle;                 /**< Handle to own fifo */

    int8_t dlt_is_file;                  /**< Target of logging: 1 to file, 0 to daemon */

    dlt_ll_ts_type *dlt_ll_ts; //[MAX_DLT_LL_TS_ENTRIES]; /**< Internal management struct for all contexts */
    uint32_t dlt_ll_ts_max_num_entries;   /**< Maximum number of contexts */

    uint32_t dlt_ll_ts_num_entries;       /**< Number of used contexts */

    int8_t overflow;                     /**< Overflow marker, set to 1 on overflow, 0 otherwise */

    char *application_description;        /**< description of application */

    DltReceiver receiver;                 /**< Receiver for internal user-defined messages from daemon */

    int8_t verbose_mode;				  /**< Verbose mode enabled: 1 enabled, 0 disabled */

    int8_t enable_local_print;            /**< Local printing of log messages: 1 enabled, 0 disabled */
    int8_t local_print_mode;              /**< Local print mode, controlled by environment variable */

    DltRingBuffer rbuf;                   /**< Ring-buffer for buffering messages during startup and missing connection */
} DltUser;

/**************************************************************************************************
* The folowing API functions define a low level function interface for DLT
**************************************************************************************************/

/**
 * Initialise the generation of a DLT log message (intended for usage in non-verbose mode)
 * This function has to be called first, when an application wants to send a new log messages.
 * @param handle pointer to an object containing information about one special logging context
 * @param log pointer to an object containing information about logging context data
 * @param loglevel this is the current log level of the log message to be sent
 * @return negative value if there was an error
 */
int dlt_user_log_write_start(DltContext *handle, DltContextData *log, DltLogLevelType loglevel);

/**
 * Initialise the generation of a DLT log message (intended for usage in verbose mode)
 * This function has to be called first, when an application wants to send a new log messages.
 * @param handle pointer to an object containing information about one special logging context
 * @param log pointer to an object containing information about logging context data
 * @param loglevel this is the current log level of the log message to be sent
 * @param messageid message id of message
 * @return negative value if there was an error
 */
int dlt_user_log_write_start_id(DltContext *handle, DltContextData *log, DltLogLevelType loglevel, uint32_t messageid);

/**
 * Finishing the generation of a DLT log message and sending it to the DLT daemon.
 * This function has to be called after writing all the log attributes of a log message.
 * @param log pointer to an object containing information about logging context data
 * @return negative value if there was an error
 */
int dlt_user_log_write_finish(DltContextData *log);

/**
 * Write a boolean parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data boolean parameter written into log message (mapped to uint8)
 * @return negative value if there was an error
 */
int dlt_user_log_write_bool(DltContextData *log, uint8_t data);

/**
 * Write a float parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data float32_t parameter written into log message.
 * @return negative value if there was an error
 */
int dlt_user_log_write_float32(DltContextData *log, float32_t data);

/**
 * Write a double parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data float64_t parameter written into log message.
 * @return negative value if there was an error
 */
int dlt_user_log_write_float64(DltContextData *log, double data);

/**
 * Write a uint parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data unsigned int parameter written into log message.
 * @return negative value if there was an error
 */
int dlt_user_log_write_uint(DltContextData *log, unsigned int data);
int dlt_user_log_write_uint8(DltContextData *log, uint8_t data);
int dlt_user_log_write_uint16(DltContextData *log, uint16_t data);
int dlt_user_log_write_uint32(DltContextData *log, uint32_t data);
int dlt_user_log_write_uint64(DltContextData *log, uint64_t data);
/**
 * Write a int parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data int parameter written into log message.
 * @return negative value if there was an error
 */
int dlt_user_log_write_int(DltContextData *log, int data);
int dlt_user_log_write_int8(DltContextData *log, int8_t data);
int dlt_user_log_write_int16(DltContextData *log, int16_t data);
int dlt_user_log_write_int32(DltContextData *log, int32_t data);
int dlt_user_log_write_int64(DltContextData *log, int64_t data);
/**
 * Write a null terminated ASCII string into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param text pointer to the parameter written into log message containing null termination.
 * @return negative value if there was an error
 */
int dlt_user_log_write_string( DltContextData *log, const char *text);

/**
 * Write a binary memory block into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about logging context data
 * @param data pointer to the parameter written into log message.
 * @param length length in bytes of the parameter written into log message.
 * @return negative value if there was an error
 */
int dlt_user_log_write_raw(DltContextData *log,void *data,uint16_t length);

/**
 * Trace network message
 * @param handle pointer to an object containing information about one special logging context
 * @param nw_trace_type type of network trace (DLT_NW_TRACE_IPC, DLT_NW_TRACE_CAN, DLT_NW_TRACE_FLEXRAY, or DLT_NW_TRACE_MOST)
 * @param header_len length of network message header
 * @param header pointer to network message header
 * @param payload_len length of network message payload
 * @param payload pointer to network message payload
 * @return negative value if there was an error
 */
int dlt_user_trace_network(DltContext *handle, DltNetworkTraceType nw_trace_type, uint16_t header_len, void *header, uint16_t payload_len, void *payload);

/**************************************************************************************************
* The folowing API functions define a high level function interface for DLT
**************************************************************************************************/

/**
 * Initialise the user lib communication with daemon.
 * This function has to be called first, before using any DLT user lib functions.
 * @return negative value if there was an error
 */
int dlt_init();

/**
 * Initialise the user lib writing only to file.
 * This function has to be called first, before using any DLT user lib functions.
 * @param name name of an optional log file
 * @return negative value if there was an error
 */
int dlt_init_file(const char *name);

/**
 * Terminate the user lib.
 * This function has to be called when finishing using the DLT user lib.
 * @return negative value if there was an error
 */
int dlt_free();

/**
 * Register an application in the daemon.
 * @param appid four byte long character array with the application id
 * @param description long name of the application
 * @return negative value if there was an error
 */
int dlt_register_app(const char *appid, const char * description);

/**
 * Unregister an application in the daemon.
 * This function has to be called when finishing using an application.
 * @return negative value if there was an error
 */
int dlt_unregister_app(void);

/**
 * Register a context in the daemon.
 * This function has to be called before first usage of the context.
 * @param handle pointer to an object containing information about one special logging context
 * @param contextid four byte long character array with the context id
 * @param description long name of the context
 * @return negative value if there was an error
 */
int dlt_register_context(DltContext *handle, const char *contextid, const char * description);

/**
 * Register a context in the daemon with pre-defined log level and pre-defined trace status.
 * This function has to be called before first usage of the context.
 * @param handle pointer to an object containing information about one special logging context
 * @param contextid four byte long character array with the context id
 * @param description long name of the context
 * @param loglevel This is the log level to be pre-set for this context
          (DLT_LOG_DEFAULT is not allowed here)
 * @param tracestatus This is the trace status to be pre-set for this context
		  (DLT_TRACE_STATUS_DEFAULT is not allowed here)
 * @return negative value if there was an error
 */
int dlt_register_context_ll_ts(DltContext *handle, const char *contextid, const char * description, int loglevel, int tracestatus);

/**
 * Unregister a context in the DLT daemon.
 * This function has to be called when finishing using a context.
 * @param handle pointer to an object containing information about one special logging context
 * @return negative value if there was an error
 */
int dlt_unregister_context(DltContext *handle);

/**
 * Register callback function called when injection message was received
 * @param handle pointer to an object containing information about one special logging context
 * @param service_id the service id to be waited for
 * @param (*dlt_injection_callback) function pointer to callback function
 * @return negative value if there was an error
 */
int dlt_register_injection_callback(DltContext *handle, uint32_t service_id,
      int (*dlt_injection_callback)(uint32_t service_id, void *data, uint32_t length));

/**
 * Switch to verbose mode
 *
 */
int dlt_verbose_mode(void);

/**
 * Switch to non-verbose mode
 *
 */
int dlt_nonverbose_mode(void);

/**
 * Set maximum logged log level and trace status of application
 *
 * @param loglevel This is the log level to be set for the whole application
 * @param tracestatus This is the trace status to be set for the whole application
 * @return negative value if there was an error
 */
int dlt_set_application_ll_ts_limit(DltLogLevelType loglevel, DltTraceStatusType tracestatus);

/**
 * Enable local printing of messages
 *
 */
int dlt_enable_local_print(void);

/**
 * Disable local printing of messages
 *
 */
int dlt_disable_local_print(void);

/**
 * Write a null terminated ASCII string into a DLT log message.
 * @param handle pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @param text pointer to the ASCII string written into log message containing null termination.
 * @return negative value if there was an error
 */
int dlt_log_string(DltContext *handle,DltLogLevelType loglevel, const char *text);

/**
 * Write a null terminated ASCII string and an integer value into a DLT log message.
 * @param handle pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @param text pointer to the ASCII string written into log message containing null termination.
 * @param data integer value written into the log message
 * @return negative value if there was an error
 */
int dlt_log_string_int(DltContext *handle,DltLogLevelType loglevel, const char *text, int data);

/**
 * Write a null terminated ASCII string and an unsigned integer value into a DLT log message.
 * @param handle pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @param text pointer to the ASCII string written into log message containing null termination.
 * @param data unsigned integer value written into the log message
 * @return negative value if there was an error
 */
int dlt_log_string_uint(DltContext *handle,DltLogLevelType loglevel, const char *text, unsigned int data);

/**
 * Write an integer value into a DLT log message.
 * @param handle pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @param data integer value written into the log message
 * @return negative value if there was an error
 */
int dlt_log_int(DltContext *handle,DltLogLevelType loglevel, int data);

/**
 * Write an unsigned integer value into a DLT log message.
 * @param handle pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @param data unsigned integer value written into the log message
 * @return negative value if there was an error
 */
int dlt_log_uint(DltContext *handle,DltLogLevelType loglevel, unsigned int data);

/**
 * Write an unsigned integer value into a DLT log message.
 * @param handle pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @param data pointer to the parameter written into log message.
 * @param length length in bytes of the parameter written into log message.
 * @return negative value if there was an error
 */
int dlt_log_raw(DltContext *handle,DltLogLevelType loglevel, void *data,uint16_t length);

/**
 * Forward a complete DLT message to the DLT daemon
 * @param msgdata Message data of DLT message
 * @param size Size of DLT message
 * @return negative value if there was an error
 */
int dlt_forward_msg(void *msgdata,size_t size);

#ifdef __cplusplus
}
#endif

/**
  \}
*/

#endif /* DLT_USER_H */

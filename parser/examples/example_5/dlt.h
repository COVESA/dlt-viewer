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
 * \file dlt.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef DLT_H
#define DLT_H

/*#include "typedefs.h" */
#include "dlt_config.h"
#include "dlt_protocol.h"


/**
 * Maximum size of a DLT ID.
 */
#define DLT_ID_SIZE 4

/**
 * Default message id for non-verbose mode, if no message id was provided.
 */
#define DLT_USER_DEFAULT_MSGID 0xffffffff

/**
 * Parameters used for DLT implementation.
 * This structure is used once for one application.
 */
typedef struct
{
    unsigned char *buffer;				 /**< Pointer to begin of buffer */
    unsigned char *buf_write;   		 /**< Pointer to current write position in buffer */
    unsigned char *buf_read;    		 /**< Pointer to current read position in buffer */
    unsigned char *buf_end;      		 /**< Pointer to position after last byte used in buffer */
	DltStandardHeader *standardheader;	 /**< Pointer to standard header of current writte DLt message */
    DltExtendedHeader *extendedheader;	 /**< Pointer to extended header of current writte DLt message */
} DltUser;

/**
 * This structure is used for every context used in an application.
 */
typedef struct
{
	unsigned char mcnt;
    const char *appid;
    const char *contextid;
    int loglevel;
} DltContext;


/**
 * This enumeration is used to identify the type of arguments which
 * are used in log messages.
 */
typedef enum
{
	DLT_ARG_CSTRING 	= 0x00,
	DLT_ARG_STRING		= 0x01,
	DLT_ARG_INT8		= 0x02,
	DLT_ARG_INT16		= 0x03,
	DLT_ARG_INT32		= 0x04,
	DLT_ARG_INT64		= 0x05,
	DLT_ARG_UINT8		= 0x06,
	DLT_ARG_UINT16		= 0x07,
	DLT_ARG_UINT32		= 0x08,
	DLT_ARG_UINT64		= 0x09,
	DLT_ARG_FLOAT32		= 0x10,
	DLT_ARG_FLOAT64	 	= 0x11,
	DLT_ARG_RAW			= 0x12,
    DLT_ARG_BOOL		= 0x13,
    DLT_ARG_NONE		= 0xFF
} DltArgumentType;


/**
 * Create an object for a new context.
 * This macro has to be called first for every.
 * @param CONTEXT object containing information about one special logging context
 */
#define DLT_DECLARE_CONTEXT(CONTEXT) \
    DltContext CONTEXT;

/**
 * Use an object of a new context created in another module.
 * This macro has to be called first for every.
 * @param CONTEXT object containing information about one special logging context
 */
#define DLT_IMPORT_CONTEXT(CONTEXT) \
    extern DltContext CONTEXT;

/**
 * Register application.
 * @param APPID application id with maximal four characters
 * @param DESCRIPTION ASCII string containing description
 */
#define DLT_REGISTER_APP(APPID,DESCRIPTION) \
    dlt_register_app( APPID, DESCRIPTION);

/**
 * Unregister application.
 */
#define DLT_UNREGISTER_APP() \
	dlt_unregister_app();

/**
 * Register context.
 * @param CONTEXT object containing information about one special logging context
 * @param APPID application id with maximal four characters
 * @param CONTEXTID context id with maximal four characters
 * @param DESCRIPTION ASCII string containing description
 */
#define DLT_REGISTER_CONTEXT(CONTEXT,APPID,CONTEXTID,DESCRIPTION) \
    dlt_register_context(&(CONTEXT), APPID, CONTEXTID, DESCRIPTION);

/**
 * Unregister context.
 * @param CONTEXT object containing information about one special logging context
 */
#define DLT_UNREGISTER_CONTEXT(CONTEXT) \
	dlt_unregister_context(&(CONTEXT));

/**
 * Set log level of context to send only DLT message with log level below this log level.
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL DLT message are only sent up to this log level
 */
#define DLT_SET_LOG_LEVEL(CONTEXT,LOGLEVEL) \
    dlt_set_log_level(&(CONTEXT),LOGLEVEL);
	
	
/**
 * Send log message with variable list of messages (intended for verbose mode)
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param ARGS variable list of arguments
 */
/*****************************************/
#define DLT_LOG0(CONTEXT,LOGLEVEL) \
	dlt_log(&CONTEXT, LOGLEVEL, DLT_USER_DEFAULT_MSGID, 0)
/*****************************************/
#define DLT_LOG1(CONTEXT,LOGLEVEL,ARGS1) \
	dlt_log(&CONTEXT, LOGLEVEL, DLT_USER_DEFAULT_MSGID, 1, ARGS1)
/*****************************************/
#define DLT_LOG2(CONTEXT,LOGLEVEL,ARGS1,ARGS2) \
	dlt_log(&CONTEXT, LOGLEVEL, DLT_USER_DEFAULT_MSGID, 2, ARGS1, ARGS2)
/*****************************************/
#define DLT_LOG3(CONTEXT,LOGLEVEL,ARGS1,ARGS2,ARGS3) \
	dlt_log(&CONTEXT, LOGLEVEL, DLT_USER_DEFAULT_MSGID, 3, ARGS1, ARGS2, ARGS3)
/***********************************************************/
#define DLT_LOG4(CONTEXT,LOGLEVEL,ARGS1,ARGS2,ARGS3,ARGS4) \
	dlt_log(&CONTEXT, LOGLEVEL, DLT_USER_DEFAULT_MSGID, 4, ARGS1, ARGS2, ARGS3, ARGS4)
/***********************************************************/
#define DLT_LOG5(CONTEXT,LOGLEVEL,ARGS1,ARGS2,ARGS3,ARGS4,ARGS5) \
	dlt_log(&CONTEXT, LOGLEVEL, DLT_USER_DEFAULT_MSGID, 5, ARGS1, ARGS2, ARGS3, ARGS4,ARGS5)
/***********************************************************/
#define DLT_LOG6(CONTEXT,LOGLEVEL,ARGS1,ARGS2,ARGS3,ARGS4,ARGS5,ARGS6) \
	dlt_log(&CONTEXT, LOGLEVEL, DLT_USER_DEFAULT_MSGID, 6, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6)
/***********************************************************/
#define DLT_LOG7(CONTEXT,LOGLEVEL,ARGS1,ARGS2,ARGS3,ARGS4,ARGS5,ARGS6,ARGS7) \
	dlt_log(&CONTEXT, LOGLEVEL, DLT_USER_DEFAULT_MSGID, 7, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7)

/**
 * Send log message with variable list of messages (intended for non-verbose mode)
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param MSGID the message id of log message
 * @param ARGS variable list of arguments
 */
/*****************************************/		
#define DLT_LOG_ID0(CONTEXT,LOGLEVEL,MSGID) \
	dlt_log(&CONTEXT, LOGLEVEL, MSGID, 0)
/*****************************************/		
#define DLT_LOG_ID1(CONTEXT,LOGLEVEL,MSGID,ARGS1) \
	dlt_log(&CONTEXT, LOGLEVEL, MSGID, 1, ARGS1)
/*****************************************/	
#define DLT_LOG_ID2(CONTEXT,LOGLEVEL,MSGID,ARGS1,ARGS2) \
	dlt_log(&CONTEXT, LOGLEVEL, MSGID, 2, ARGS1, ARGS2)
/*****************************************/
#define DLT_LOG_ID3(CONTEXT,LOGLEVEL,MSGID,ARGS1,ARGS2,ARGS3) \
	dlt_log(&CONTEXT, LOGLEVEL, MSGID, 3, ARGS1, ARGS2, ARGS3)
/***********************************************************/	
#define DLT_LOG_ID4(CONTEXT,LOGLEVEL,MSGID,ARGS1,ARGS2,ARGS3,ARGS4) \
	dlt_log(&CONTEXT, LOGLEVEL, MSGID, 4, ARGS1, ARGS2, ARGS3, ARGS4)
/***********************************************************/
#define DLT_LOG_ID5(CONTEXT,LOGLEVEL,MSGID,ARGS1,ARGS2,ARGS3,ARGS4,ARGS5) \
	dlt_log(&CONTEXT, LOGLEVEL, MSGID, 5, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5)
/***********************************************************/
#define DLT_LOG_ID6(CONTEXT,LOGLEVEL,MSGID,ARGS1,ARGS2,ARGS3,ARGS4,ARGS5,ARGS6) \
	dlt_log(&CONTEXT, LOGLEVEL, MSGID, 6, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6)
/***********************************************************/
#define DLT_LOG_ID7(CONTEXT,LOGLEVEL,MSGID,ARGS1,ARGS2,ARGS3,ARGS4,ARGS5,ARGS6,ARGS7) \
	dlt_log(&CONTEXT, LOGLEVEL, MSGID, 7, ARGS1, ARGS2, ARGS3, ARGS4, ARGS5, ARGS6, ARGS7)
	
/**
 * Add string parameter to the log messsage.
 * @param TEXT ASCII string
 */
#define DLT_STRING(TEXT)	DLT_ARG_STRING, TEXT 

/**
 * Add constant string parameter to the log messsage.
 * In non-verbose mode constant strings are discarded.
 * @param TEXT ASCII string
 */
#ifdef DLT_VERBOSE_MODE

#define DLT_CSTRING(TEXT) 	DLT_ARG_STRING, TEXT

#else /* DLT_VERBOSE_MODE */

#define DLT_CSTRING(TEXT) 	DLT_ARG_NONE

#endif /* DLT_VERBOSE_MODE */

/**
 * Add boolean parameter to the log messsage.
 * @param BOOL_VAR Boolean value (mapped to uint8)
 */
#define DLT_BOOL(BOOL_VAR) 	DLT_ARG_BOOL, BOOL_VAR


/**
 * Add float32 parameter to the log messsage.
 * @param FLOAT32_VAR Float32 value (mapped to float)
 */
#define DLT_FLOAT32(FLOAT32_VAR)	DLT_ARG_FLOAT32, FLOAT32_VAR 

/**
 * Add float64 parameter to the log messsage.
 * @param FLOAT64_VAR Float64 value (mapped to double)
 */
#define DLT_FLOAT64(FLOAT64_VAR)	DLT_ARG_FLOAT64, FLOAT64_VAR

/**
 * Add integer parameter to the log messsage.
 * @param INT_VAR integer value
 */
#define DLT_INT(INT_VAR) 	DLT_ARG_INT32, INT_VAR
#define DLT_INT8(INT_VAR)	DLT_ARG_INT8,  INT_VAR
#define DLT_INT16(INT_VAR) 	DLT_ARG_INT16, INT_VAR
#define DLT_INT32(INT_VAR) 	DLT_ARG_INT32, INT_VAR
#define DLT_INT64(INT_VAR) 	DLT_ARG_INT64, INT_VAR

/**
 * Add unsigned integer parameter to the log messsage.
 * @param UINT_VAR unsigned integer value
 */
#define DLT_UINT(INT_VAR) 		DLT_ARG_UINT32, INT_VAR
#define DLT_UINT8(INT_VAR)		DLT_ARG_UINT8,  INT_VAR
#define DLT_UINT16(INT_VAR) 	DLT_ARG_UINT16, INT_VAR
#define DLT_UINT32(INT_VAR) 	DLT_ARG_UINT32, INT_VAR
#define DLT_UINT64(INT_VAR) 	DLT_ARG_UINT64, INT_VAR

/**
 * Add binary memory block to the log messages.
 * @param BUF pointer to memory block
 * @param LEN length of memory block
 */
#define DLT_RAW(BUF,LEN)		DLT_ARG_RAW, BUF, LEN
	
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
 * The default log level is set.
 * @param log pointer to an object containing information about one special logging context
 * @param appid four byte long character array with the application id
 * @param contextid four byte long character array with the context id
 * @param description long name of the context
 * @return negative value if there was an error
 */
int dlt_register_context(DltContext *log, const char *appid, const char *contextid, const char * description);

/**
 * Unregister a context in the DLT daemon.
 * This function has to be called when finishing using a context.
 * @param log pointer to an object containing information about one special logging context
 * @return negative value if there was an error
 */
int dlt_unregister_context(DltContext *log);

/**
 * Set log level of context to send only DLT message with log level below this log level.
 * Function must be called after dlt_register_context.
 * @param log pointer to an object containing information about one special logging context
 * @param loglevel DLT message are only sent up to this log level
 * @return negative value if there was an error
 */
int dlt_set_log_level(DltContext *log,int loglevel);

/**
 * Log function
 *
 * @param a_context reference to object containing information about one special logging context
 * @param a_logLevel DLT message are only sent up to this log level
 * @param a_messageId the message id of log messag, can be set to DLT_USER_DEFAULT_MSGID if 
 *        no message id is available
 * @param a_numberOfArguments number of log elements. A log element is a call to one of the DLT_LOGx
 * 		  macros
 */ 
int dlt_log(DltContext* a_context, int a_logLevel, uint32 a_messageId, int a_numberOfArguments, ...);	

/**
 * Initialise the use of DLT.
 * Initialise buffer.
 * @return negative value if there was an error
 */
int dlt_user_init();

/**
 * Initialise the use of DLT.
 * Initialise buffer.
 * @return negative value if there was an error
 */
int dlt_user_free();

/**
 * Read data from the buffer.
 * If data is available at beginning and end of buffer two calls can be necessary.
 * After data is forwarded, dlt_user_log_read_ack must be called to free the buffer
 * @param data pointer to pointer which contains data which can be read
 * @return number of bytes read, 0 if no bytes left, negative value if there was an error
 */
int dlt_user_log_read(unsigned char **data);

/**
 * Acknowledge, that read data can be removed from buffer
 * @param length the number of bytes which can be removed from buffer
 * @return negative value if there was an error
 */
int dlt_user_log_read_ack(uint16 length);

/**
 * Initialise the generation of a DLT log message (intended for usage in non-verbose mode)
 * This function has to be called first, when an application wants to send a new log messages.
 * @param log pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @return negative value if there was an error
 */
int dlt_user_log_write_start(DltContext *log,int loglevel);

/**
 * Initialise the generation of a DLT log message (intended for usage in verbose mode)
 * This function has to be called first, when an application wants to send a new log messages.
 * @param log pointer to an object containing information about one special logging context
 * @param loglevel this is the current log level of the log message to be sent
 * @param messageid message id of message
 * @return negative value if there was an error
 */
int dlt_user_log_write_start_id(DltContext *log,int loglevel, uint32 messageid);

/**
 * Finishing the generation of a DLT log message and sending it to the DLT daemon.
 * This function has to be called after writing all the log attributes of a log message.
 * @param log pointer to an object containing information about one special logging context
 * @return negative value if there was an error
 */
int dlt_user_log_write_finish(DltContext *log);

/**
 * Write a boolean parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about one special logging context
 * @param data boolean parameter written into log message (mapped to uint8)
 * @return negative value if there was an error
 */
int dlt_user_log_write_bool(DltContext *log, uint8 data);

/**
 * Write a float parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about one special logging context
 * @param data unsigned int parameter written into log message.
 * @return negative value if there was an error
 */
int dlt_user_log_write_float32(DltContext *log, float data);

/**
 * Write a double parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about one special logging context
 * @param data unsigned int parameter written into log message.
 * @return negative value if there was an error
 */
int dlt_user_log_write_float64(DltContext *log, double data);

/**
 * Write a uint parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about one special logging context
 * @param data unsigned int parameter written into log message.
 * @return negative value if there was an error
 */
int dlt_user_log_write_uint(DltContext *log, unsigned int data);
int dlt_user_log_write_uint8(DltContext *log, uint8 data);
int dlt_user_log_write_uint16(DltContext *log, uint16 data);
int dlt_user_log_write_uint32(DltContext *log, uint32 data);
int dlt_user_log_write_uint64(DltContext *log, uint64 data);
/**
 * Write a int parameter into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about one special logging context
 * @param data int parameter written into log message.
 * @return negative value if there was an error
 */
int dlt_user_log_write_int(DltContext *log, int data);
int dlt_user_log_write_int8(DltContext *log, sint8 data);
int dlt_user_log_write_int16(DltContext *log, sint16 data);
int dlt_user_log_write_int32(DltContext *log, sint32 data);
int dlt_user_log_write_int64(DltContext *log, sint64 data);
/**
 * Write a null terminated ASCII string into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about one special logging context
 * @param text pointer to the parameter written into log message containing null termination.
 * @return negative value if there was an error
 */
int dlt_user_log_write_string( DltContext *log, const char *text);

/**
 * Write a binary memory block into a DLT log message.
 * dlt_user_log_write_start has to be called before adding any attributes to the log message.
 * Finish sending log message by calling dlt_user_log_write_finish.
 * @param log pointer to an object containing information about one special logging context
 * @param data pointer to the parameter written into log message.
 * @param length length in bytes of the parameter written into log message.
 * @return negative value if there was an error
 */
int dlt_user_log_write_raw(DltContext *log,void *data,uint16 length);

#endif

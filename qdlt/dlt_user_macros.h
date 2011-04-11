/*
 * Dlt- Diagnostic Log and Trace console apps
 * @licence app begin@
 *
 * Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the 
 * GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 * Public License, version 2.1, for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 * with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * 
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 * also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * 
 * Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 * license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 * license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 * may use the program under the following exception.
 * 
 * As a special exception, the copyright holders of DLT give you permission to combine DLT 
 * with software programs or libraries that are released under any license unless such a combination is not
 * permitted by the license of such a software program or library. You may copy and distribute such a 
 * system following the terms of the GNU Lesser General Public License, version 2.1, including this
 * special exception, for DLT and the licenses of the other code concerned.
 * 
 * Note that people who make modified versions of DLT are not obligated to grant this special exception 
 * for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 * version 2.1, gives permission to release a modified version without this exception; this exception 
 * also makes it possible to release a modified version which carries forward this exception.
 *
 * @licence end@
 */


/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_user_macros.h                                             **
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
 * $LastChangedRevision: 1515 $
 * $LastChangedDate: 2010-12-13 09:18:54 +0100 (Mon, 13 Dec 2010) $
 * $LastChangedBy$
 Initials    Date         Comment
 aw          13.01.2010   initial
 */

#ifndef DLT_USER_MACROS_H
#define DLT_USER_MACORS_H

/**
  \defgroup userapi DLT User API
  \addtogroup userapi
  \{
*/

/**************************************************************************************************
* The folowing macros define a macro interface for DLT
**************************************************************************************************/

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
 * Register context (with default log level and default trace status)
 * @param CONTEXT object containing information about one special logging context
 * @param CONTEXTID context id with maximal four characters
 * @param DESCRIPTION ASCII string containing description
 */
#define DLT_REGISTER_CONTEXT(CONTEXT,CONTEXTID,DESCRIPTION) \
	dlt_register_context(&(CONTEXT), CONTEXTID, DESCRIPTION);

/**
 * Register context with pre-defined log level and pre-defined trace status.
 * @param CONTEXT object containing information about one special logging context
 * @param CONTEXTID context id with maximal four characters
 * @param DESCRIPTION ASCII string containing description
 * @param LOGLEVEL log level to be pre-set for this context
          (DLT_LOG_DEFAULT is not allowed here)
 * @param TRACESTATUS trace status to be pre-set for this context
		  (DLT_TRACE_STATUS_DEFAULT is not allowed here)
 */
#define DLT_REGISTER_CONTEXT_LL_TS(CONTEXT,CONTEXTID,DESCRIPTION,LOGLEVEL,TRACESTATUS) \
	dlt_register_context_ll_ts(&(CONTEXT), CONTEXTID, DESCRIPTION, LOGLEVEL, TRACESTATUS);

/**
 * Unregister context.
 * @param CONTEXT object containing information about one special logging context
 */
#define DLT_UNREGISTER_CONTEXT(CONTEXT) \
	dlt_unregister_context(&(CONTEXT));

/**
 * Register callback function called when injection message was received
 * @param CONTEXT object containing information about one special logging context
 * @param SERVICEID service id of the injection message
 * @param CALLBACK function pointer to callback function
 */
#define DLT_REGISTER_INJECTION_CALLBACK(CONTEXT, SERVICEID, CALLBACK) \
	dlt_register_injection_callback(&(CONTEXT),SERVICEID, CALLBACK);

/**
 * Send log message with variable list of messages (intended for verbose mode)
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param ARGS variable list of arguments
 */
#ifdef _MSC_VER
  /* DLT_LOG is not supported by MS Visual C++ */
  /* use function interface instead            */
#else
#define DLT_LOG(CONTEXT,LOGLEVEL,ARGS...) \
	{ \
		DltContextData log; \
		if (dlt_user_log_write_start(&CONTEXT,&log,LOGLEVEL)) \
		{ \
            ARGS; \
            dlt_user_log_write_finish(&log); \
		} \
	}
#endif

/**
 * Send log message with variable list of messages (intended for non-verbose mode)
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param MSGID the message id of log message
 * @param ARGS variable list of arguments: 
          calls to DLT_STRING(), DLT_BOOL(), DLT_FLOAT32(), DLT_FLOAT64(),
          DLT_INT(), DLT_UINT(), DLT_RAW()
 */
#ifdef _MSC_VER
  /* DLT_LOG_ID is not supported by MS Visual C++ */
  /* use function interface instead               */
#else
#define DLT_LOG_ID(CONTEXT,LOGLEVEL,MSGID,ARGS...) \
	{ \
		DltContextData log; \
		if (dlt_user_log_write_start_id(&CONTEXT,&log,LOGLEVEL,MSGID)) \
		{ \
            ARGS; \
            dlt_user_log_write_finish(&log); \
		} \
	}
#endif

/**
 * Add string parameter to the log messsage.
 * @param TEXT ASCII string
 */
#define DLT_STRING(TEXT) ({ \
        dlt_user_log_write_string(&log,TEXT); \
	})

/**
 * Add boolean parameter to the log messsage.
 * @param BOOL_VAR Boolean value (mapped to uint8)
 */
#define DLT_BOOL(BOOL_VAR) ({ \
        dlt_user_log_write_bool(&log,BOOL_VAR); \
	})

/**
 * Add float32 parameter to the log messsage.
 * @param FLOAT32_VAR Float32 value (mapped to float)
 */
#define DLT_FLOAT32(FLOAT32_VAR) ({ \
        dlt_user_log_write_float32(&log,FLOAT32_VAR); \
	})

/**
 * Add float64 parameter to the log messsage.
 * @param FLOAT64_VAR Float64 value (mapped to double)
 */
#define DLT_FLOAT64(FLOAT64_VAR) ({ \
        dlt_user_log_write_float64(&log,FLOAT64_VAR); \
	})

/**
 * Add integer parameter to the log messsage.
 * @param INT_VAR integer value
 */
#define DLT_INT(INT_VAR) ({ \
        dlt_user_log_write_int(&log,INT_VAR); \
	})
#define DLT_INT8(INT_VAR) ({ \
        dlt_user_log_write_int8(&log,INT_VAR); \
	})
#define DLT_INT16(INT_VAR) ({ \
        dlt_user_log_write_int16(&log,INT_VAR); \
	})
#define DLT_INT32(INT_VAR) ({ \
        dlt_user_log_write_int32(&log,INT_VAR); \
	})
#define DLT_INT64(INT_VAR) ({ \
        dlt_user_log_write_int64(&log,INT_VAR); \
	})

/**
 * Add unsigned integer parameter to the log messsage.
 * @param UINT_VAR unsigned integer value
 */
#define DLT_UINT(UINT_VAR) ({ \
        dlt_user_log_write_uint(&log,UINT_VAR); \
	})
#define DLT_UINT8(UINT_VAR) ({ \
        dlt_user_log_write_uint8(&log,UINT_VAR); \
	})
#define DLT_UINT16(UINT_VAR) ({ \
        dlt_user_log_write_uint16(&log,UINT_VAR); \
	})
#define DLT_UINT32(UINT_VAR) ({ \
        dlt_user_log_write_uint32(&log,UINT_VAR); \
	})
#define DLT_UINT64(UINT_VAR) ({ \
        dlt_user_log_write_uint64(&log,UINT_VAR); \
	})

/**
 * Add binary memory block to the log messages.
 * @param BUF pointer to memory block
 * @param LEN length of memory block
 */
#define DLT_RAW(BUF,LEN) ({ \
        dlt_user_log_write_raw(&log,BUF,LEN); \
	})

/**
 * Trace network message
 * @param CONTEXT object containing information about one special logging context
 * @param TYPE type of network trace message
 * @param HEADERLEN length of network message header
 * @param HEADER pointer to network message header
 * @param PAYLOADLEN length of network message payload
 * @param PAYLOAD pointer to network message payload
 */
#define DLT_TRACE_NETWORK(CONTEXT,TYPE,HEADERLEN,HEADER,PAYLOADLEN,PAYLOAD) \
    { \
        dlt_user_trace_network(&(CONTEXT),TYPE,HEADERLEN,HEADER,PAYLOADLEN,PAYLOAD); \
    }

/**
 * Send log message with string parameter.
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param TEXT ASCII string
 */
#define DLT_LOG_STRING(CONTEXT,LOGLEVEL,TEXT) \
    { \
        dlt_log_string(&(CONTEXT), LOGLEVEL, TEXT); \
    }


/**
 * Send log message with string parameter and integer parameter.
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log messages
 * @param TEXT ASCII string
 * @param INT_VAR integer value
 */
#define DLT_LOG_STRING_INT(CONTEXT,LOGLEVEL,TEXT,INT_VAR) \
	{ \
        dlt_log_string_int(&(CONTEXT), LOGLEVEL, TEXT, INT_VAR); \
	}

/**
 * Send log message with string parameter and unsigned integer parameter.
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param TEXT ASCII string
 * @param UINT_VAR unsigned integer value
 */
#define DLT_LOG_STRING_UINT(CONTEXT,LOGLEVEL,TEXT,UINT_VAR) \
	{ \
		dlt_log_string_uint(&(CONTEXT),LOGLEVEL,TEXT,UINT_VAR); \
	}

/**
 * Send log message with unsigned integer parameter.
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param UINT_VAR unsigned integer value
 */
#define DLT_LOG_UINT(CONTEXT,LOGLEVEL,UINT_VAR) \
	{ \
		dlt_log_uint(&(CONTEXT),LOGLEVEL,UINT_VAR); \
	}

/**
 * Send log message with integer parameter.
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param INT_VAR integer value
 */
#define DLT_LOG_INT(CONTEXT,LOGLEVEL,INT_VAR) \
	{ \
		dlt_log_int(&(CONTEXT),LOGLEVEL,INT_VAR); \
	}

/**
 * Send log message with binary memory block.
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param BUF pointer to memory block
 * @param LEN length of memory block
 */
#define DLT_LOG_RAW(CONTEXT,LOGLEVEL,BUF,LEN) \
	{ \
		dlt_log_raw(&(CONTEXT),LOGLEVEL,BUF,LEN); \
	}

/**
 * Switch to verbose mode
 *
 */
#define DLT_VERBOSE_MODE() \
	dlt_verbose_mode();

/**
 * Switch to non-verbose mode
 *
 */
#define DLT_NONVERBOSE_MODE() \
	dlt_nonverbose_mode();

/**
 * Set maximum logged log level and trace status of application
 *
 * @param LOGLEVEL This is the log level to be set for the whole application
 * @param TRACESTATUS This is the trace status to be set for the whole application
 */
#define DLT_SET_APPLICATION_LL_TS_LIMIT(LOGLEVEL, TRACESTATUS) \
    dlt_set_application_ll_ts_limit(LOGLEVEL, TRACESTATUS);

/**
 * Enable local printing of messages
 *
 */
#define DLT_ENABLE_LOCAL_PRINT() \
	dlt_enable_local_print();

/**
 * Disable local printing of messages
 *
 */
#define DLT_DISABLE_LOCAL_PRINT() \
	dlt_disable_local_print();

/**
  \}
*/

#endif /* DLT_USER_MACROS_H */

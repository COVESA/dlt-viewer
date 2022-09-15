/**
 * @licence app begin@
 * Copyright (C) 2014  BMW AG
 *
 * This file is part of COVESA Project Dlt - Diagnostic Log and Trace console apps.
 *
 * Contributions are licensed to the COVESA Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> BMW 2011-2012
 *
 * \file dlt_common_api.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_commpn_api.h                                              **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
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
*******************************************************************************/

#ifndef DLT_COMMON_API_H
#define DLT_COMMON_API_H

#include "dlt.h"

/**
 * Create an object for a new context.
 * Common API with DLT Embedded
 * This macro has to be called first for every.
 * @param CONTEXT object containing information about one special logging context
 */
/* #define DLT_DECLARE_CONTEXT(CONTEXT) */
/* UNCHANGED */

/**
 * Use an object of a new context created in another module.
 * Common API with DLT Embedded
 * This macro has to be called first for every.
 * @param CONTEXT object containing information about one special logging context
 */
/* #define DLT_IMPORT_CONTEXT(CONTEXT) */
/* UNCHANGED */

/**
 * Register application.
 * Common API with DLT Embedded
 * @param APPID application id with maximal four characters
 * @param DESCRIPTION ASCII string containing description
 */
/* #define DLT_REGISTER_APP(APPID,DESCRIPTION) */
/* UNCHANGED */

/**
 * Register context including application (with default log level and default trace status)
 * Common API with DLT Embedded
 * @param CONTEXT object containing information about one special logging context
 * @param CONTEXTID context id with maximal four characters
 * @param APPID context id with maximal four characters
 * @param DESCRIPTION ASCII string containing description
 */
#define DLT_REGISTER_CONTEXT_APP(CONTEXT,CONTEXTID,APPID,DESCRIPTION) \
    DLT_REGISTER_CONTEXT(CONTEXT,APPID,CONTEXTID,DESCRIPTION)

/**
 * Send log message with variable list of messages (intended for verbose mode)
 * Common API with DLT Embedded
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param ARGS variable list of arguments
 */
/*****************************************/
/*#define DLT_LOG0(CONTEXT,LOGLEVEL) */
/* UNCHANGED */

/**
 * Send log message with variable list of messages (intended for non-verbose mode)
 * Common API with DLT Embedded
 * @param CONTEXT object containing information about one special logging context
 * @param LOGLEVEL the log level of the log message
 * @param MSGID the message id of log message
 * @param ARGS variable list of arguments:
 calls to DLT_STRING(), DLT_BOOL(), DLT_FLOAT32(), DLT_FLOAT64(),
 DLT_INT(), DLT_UINT(), DLT_RAW()
 */
/*****************************************/
/* #define DLT_LOG_ID0(CONTEXT,LOGLEVEL,MSGID) */
/* UNCHANGED */

/**
 * Unregister context.
 * Common API with DLT Embedded
 * @param CONTEXT object containing information about one special logging context
 */
/* #define DLT_UNREGISTER_CONTEXT(CONTEXT) */
/* UNCHANGED */

/**
 * Unregister application.
 * Common API with DLT Embedded
 */
/* #define DLT_UNREGISTER_APP() */
/* UNCHANGED */

/**
 * Add string parameter to the log messsage.
 * Common API with DLT Embedded
 * In the future in none verbose mode the string will not be sent via DLT message.
 * @param TEXT ASCII string
 */
/* #define DLT_CSTRING(TEXT) */
/* UNCHANGED */

#endif /* DLT_COMMON_API_H */


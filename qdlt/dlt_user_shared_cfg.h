/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \author Alexander Wenzel <alexander.wenzel@bmw.de> 2011-2012
 * 
 * \file <FILE>
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_user_shared_cfg.h                                         **
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
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

#ifndef DLT_USER_SHARED_CFG_H
#define DLT_USER_SHARED_CFG_H

/*************/
/* Changable */
/*************/

/* Directory, whre the named pipes to the DLT daemon resides */
#define DLT_USER_DIR  "/tmp"

/* Name of named pipe to DLT daemon */
#define DLT_USER_FIFO "/tmp/dlt"

/************************/
/* Don't change please! */
/************************/

/* The different types of internal messages between user application and daemon. */
#define DLT_USER_MESSAGE_LOG 1
#define DLT_USER_MESSAGE_REGISTER_APPLICATION 2
#define DLT_USER_MESSAGE_UNREGISTER_APPLICATION 3
#define DLT_USER_MESSAGE_REGISTER_CONTEXT 4
#define DLT_USER_MESSAGE_UNREGISTER_CONTEXT 5
#define DLT_USER_MESSAGE_LOG_LEVEL 6
#define DLT_USER_MESSAGE_INJECTION 7
#define DLT_USER_MESSAGE_OVERFLOW 8
#define DLT_USER_MESSAGE_APP_LL_TS 9

/* Internal defined values */

/* must be different from DltLogLevelType */
#define DLT_USER_LOG_LEVEL_NOT_SET    -2
/* must be different from DltTraceStatusType */
#define DLT_USER_TRACE_STATUS_NOT_SET -2

#endif /* DLT_USER_SHARED_CFG_H */


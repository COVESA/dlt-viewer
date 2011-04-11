/*
 * Dlt- Diagnostic Log and Trace daemon
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


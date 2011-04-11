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
**  SRC-MODULE: dlt_user_shared.h                                             **
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

#ifndef DLT_USER_SHARED_H
#define DLT_USER_SHARED_H

#include "dlt_types.h"
#include "dlt_user.h"

#include <sys/types.h>

/**
 * This are the return values for the functions dlt_user_log_out2() and dlt_user_log_out3()
 */
typedef enum
{
	DLT_RETURN_PIPE_FULL  = -3,
	DLT_RETURN_PIPE_ERROR = -2,
	DLT_RETURN_ERROR      = -1,
	DLT_RETURN_OK         =  0
} DltReturnValue;

/**
 * This is the header of each message to be exchanged between application and daemon.
 */
typedef struct
{
	char pattern[DLT_ID_SIZE];      /**< This pattern should be DUH0x01 */
	uint32_t message;               /**< messsage info */
} PACKED DltUserHeader;

/**
 * This is the internal message content to exchange control msg register app information between application and daemon.
 */
typedef struct
{
	char apid[DLT_ID_SIZE];          /**< application id */
	pid_t pid;                       /**< process id of user application */
    uint32_t description_length;     /**< length of description */
} PACKED DltUserControlMsgRegisterApplication;

/**
 * This is the internal message content to exchange control msg unregister app information between application and daemon.
 */
typedef struct
{
	char apid[DLT_ID_SIZE];         /**< application id */
    pid_t pid;                      /**< process id of user application */
} PACKED DltUserControlMsgUnregisterApplication;

/**
 * This is the internal message content to exchange control msg register information between application and daemon.
 */
typedef struct
{
	char apid[DLT_ID_SIZE];          /**< application id */
	char ctid[DLT_ID_SIZE];          /**< context id */
	int32_t log_level_pos;           /**< offset in management structure on user-application side */
	int8_t log_level;                /**< log level */
	int8_t trace_status;             /**< trace status */
	pid_t pid;                       /**< process id of user application */
    uint32_t description_length;     /**< length of description */
} PACKED DltUserControlMsgRegisterContext;

/**
 * This is the internal message content to exchange control msg unregister information between application and daemon.
 */
typedef struct
{
	char apid[DLT_ID_SIZE];         /**< application id */
	char ctid[DLT_ID_SIZE];         /**< context id */
	pid_t pid;                      /**< process id of user application */
} PACKED DltUserControlMsgUnregisterContext;

/**
 * This is the internal message content to exchange control msg log level information between application and daemon.
 */
typedef struct
{
	uint8_t log_level;             /**< log level */
	uint8_t trace_status;          /**< trace status */
	int32_t log_level_pos;          /**< offset in management structure on user-application side */
} PACKED DltUserControlMsgLogLevel;

/**
 * This is the internal message content to exchange control msg injection information between application and daemon.
 */
typedef struct
{
	int32_t log_level_pos;          /**< offset in management structure on user-application side */
	uint32_t service_id;            /**< service id of injection */
    uint32_t data_length_inject;    /**< length of injection message data field */
} PACKED DltUserControlMsgInjection;

/**
 * This is the internal message content to exchange information about application log level and trace stats between
 * application and daemon.
 */
typedef struct
{
    char apid[DLT_ID_SIZE];        /**< application id */
	uint8_t log_level;             /**< log level */
	uint8_t trace_status;          /**< trace status */
} PACKED DltUserControlMsgAppLogLevelTraceStatus;

/**************************************************************************************************
* The folowing functions are used shared between the user lib and the daemon implementation
**************************************************************************************************/

/**
 * Set user header marker and store message type in user header
 * @param userheader pointer to the userheader
 * @param mtype user message type of internal message
 * @return negative value if there was an error {
                    	return -1;
                    }
 */
int dlt_user_set_userheader(DltUserHeader *userheader, uint32_t mtype);

/**
 * Check if user header contains its marker
 * @param userheader pointer to the userheader
 * @return 0 no, 1 yes, negative value if there was an error
 */
int dlt_user_check_userheader(DltUserHeader *userheader);

/**
 * Atomic write to file descriptor, using vector of 2 elements
 * @param handle file descriptor
 * @param ptr1 generic pointer to first segment of data to be written
 * @param len1 length of first segment of data to be written
 * @param ptr2 generic pointer to second segment of data to be written
 * @param len2 length of second segment of data to be written
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_out2(int handle, void *ptr1, size_t len1, void* ptr2, size_t len2);

/**
 * Atomic write to file descriptor, using vector of 3 elements
 * @param handle file descriptor
 * @param ptr1 generic pointer to first segment of data to be written
 * @param len1 length of first segment of data to be written
 * @param ptr2 generic pointer to second segment of data to be written
 * @param len2 length of second segment of data to be written
 * @param ptr3 generic pointer to third segment of data to be written
 * @param len3 length of third segment of data to be written
 * @return Value from DltReturnValue enum
 */
DltReturnValue dlt_user_log_out3(int handle, void *ptr1, size_t len1, void* ptr2, size_t len2, void *ptr3, size_t len3);

#endif /* DLT_USER_SHARED_H */

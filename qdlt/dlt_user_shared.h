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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> 2011-2012
 * 
 * \file dlt_user_shared.h
 * For further information see http://www.genivi.org/.
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

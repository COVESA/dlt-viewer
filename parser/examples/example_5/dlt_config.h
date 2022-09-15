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
 * \file dlt_config.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef DLT_CONFIG_H
#define DLT_CONFIG_H

/* Definition of the types used in DLT */
#ifdef DLT_EXAMPLE
    typedef unsigned char uint8 ;
    typedef unsigned short uint16 ;
    typedef unsigned int uint32 ;
    typedef unsigned long long uint64 ;
    typedef signed char sint8 ;
    typedef signed short sint16 ;
    typedef signed int sint32 ;
    typedef signed long long sint64 ;
#endif

/* Definition of the types for variable argument lists used in DLT */
#ifdef DLT_EXAMPLE
    typedef int var_uint8 ;
    typedef int var_uint16 ;
    typedef unsigned int var_uint32 ;
    typedef unsigned long long var_uint64 ;
    typedef signed int var_sint8 ;
    typedef signed int var_sint16 ;
    typedef signed int var_sint32 ;
    typedef signed long long var_sint64 ;
    typedef unsigned int var_bool ;
    typedef double var_float32 ;
    typedef double var_float64 ;
#endif

/* Define the byte order used on this machine, used to set the correct values in the DLT header. */
/* Default: LITTLE_ENDIAN is used. */
#ifdef DLT_EXAMPLE
        #define BYTE_ORDER LITTLE_ENDIAN
#else
	#define BYTE_ORDER BIG_ENDIAN
#endif

/* Use a semaphore or mutex from your OS to prevent concurrent access to the DLT buffer. */
#define DLT_SEM_LOCK() {}
#define DLT_SEM_FREE() {}

/* Send DLT messages in verbose mode instead of non-verbose mode. Non-verbose mode is the default mode. */
//#define DLT_VERBOSE_MODE 1

/* Set the used default log level */
#define DLT_DEFAULT_LOG_LEVEL DLT_LOG_INFO

/* Define the maximum size of a single DLT message including all headers used. */
/* Standardheader uses 4 Bytes. Message Id needs additional 4 Bytes. */
#define DLT_MESSAGE_MAX_SIZE 128

/* Size of the complete buffer used for storing temporarily DLT messages. */
/* Buffer needed for asynchronous communication. */
#define DLT_BUFFER_SIZE 1024

/* Address of buffer to be used. */
/* Buffer can be allocated dynamically or static address is used. */
//#define DLT_BUFFER_ALLOCATE malloc(DLT_BUFFER_SIZE);
#ifdef DLT_EXAMPLE
	#define DLT_BUFFER_ALLOCATE malloc(DLT_BUFFER_SIZE);
#else
	#define DLT_BUFFER_ALLOCATE DLT_BUFFER;
#endif

/* Free used buffer if dynamically allocated. */
#define DLT_BUFFER_FREE(address) free(address);

/* Add serial header to each DLT message, if serial communication is used. */
#define DLT_USE_SERIAL_HEADER 1

/* Add storage header to each DLT message, if messages are stored in a file  */
/* Only useful for testing purpose */
//#define DLT_USE_STORAGE_HEADER 1

/* A callback function can be added which is called, when a new DLT message is available in the buffer.  */
#define DLT_CALLBACK_DATA_AVAILABLE() {}

#endif

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
 * \file dlt.c
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "dlt.h"

/* Global variables used for DLT implementation */
DltUser dltuser;
unsigned char DLT_BUFFER[DLT_BUFFER_SIZE];

/* Variable to check if DLt is already initialised */
int dlt_user_initialised = 0;

void dlt_set_id(char *id,const char *text)
{
    id[0] = 0;
    id[1] = 0;
    id[2] = 0;
    id[3] = 0;

    if (!text) return;

    if (text[0]!=0) id[0] = text[0];
    else return;
    if (text[1]!=0) id[1] = text[1];
    else return;
    if (text[2]!=0) id[2] = text[2];
    else return;
    if (text[3]!=0) id[3] = text[3];
    else return;
}

int dlt_user_init()
{
	
	/* allocate buffer memory, dynamically or statically configured */
	dltuser.buffer = (unsigned char*) DLT_BUFFER_ALLOCATE;

	/* initialise buffer pointers */
	dltuser.buf_write = dltuser.buffer;
	dltuser.buf_read = dltuser.buffer;
	dltuser.buf_end = dltuser.buffer;
	
	return 0;
}

int dlt_user_free()
{
	
	/* free buffer if dynamically allocated */
	DLT_BUFFER_FREE(dltuser.buffer);

        return 0;

}

int dlt_register_app(const char *appid, const char * description)
{
	DLT_SEM_LOCK();

	/* initialise DLT, if not done already */
	if(!dlt_user_initialised)
	{
		dlt_user_init();
		dlt_user_initialised=1;
	}

	/* no more implementation done in DLT embedded */
	/* app id must not be stored */

	DLT_SEM_FREE();
	
	return 0;
}

int dlt_unregister_app(void)
{
	DLT_SEM_LOCK();
	
	/* nothing to be done in DLT embedded */
	
	DLT_SEM_FREE();

	return 0;
}

int dlt_register_context(DltContext *log, const char *appid, const char *contextid, const char * description)
{
	DLT_SEM_LOCK();

	/* initialise DLT, if not done already */
	if(!dlt_user_initialised)
	{
		dlt_user_init();
		dlt_user_initialised=1;
	}

    /* reset message counter */
    log->mcnt = 0;

    /* store ids for verbose mode */
    log->appid = appid;
    log->contextid = contextid;
	
    /* set the default log level */
    log->loglevel = DLT_DEFAULT_LOG_LEVEL;

	DLT_SEM_FREE();

	return 0;
}

int dlt_unregister_context(DltContext *log)
{
	DLT_SEM_LOCK();

	/* nothing to be done in DLT embedded */

	DLT_SEM_FREE();

	return 0;
}

int dlt_set_log_level(DltContext *log,int loglevel)
{
    if(loglevel == DLT_LOG_DEFAULT)
        log->loglevel = DLT_DEFAULT_LOG_LEVEL;
    else
        log->loglevel = loglevel;


    return 0;
}

int dlt_log(DltContext* a_context, int a_logLevel, uint32 a_dltId, int a_numberOfArguments, ...)
{
    void * data;
    int len;

	if (a_logLevel <= a_context->loglevel)
    { 
        DLT_SEM_LOCK(); 
        if (dlt_user_log_write_start_id(a_context, a_logLevel, a_dltId)) 
		{
			int i;
			DltArgumentType l_argumentType;
			va_list l_arguments;
			va_start(l_arguments, a_numberOfArguments);
			
			for (i = 0; i < a_numberOfArguments; ++i)
			{
				l_argumentType = va_arg(l_arguments, DltArgumentType);
				switch (l_argumentType)
				{
					case DLT_ARG_CSTRING:
						dlt_user_log_write_string(a_context, va_arg(l_arguments, char*));
						break;
				
					case DLT_ARG_STRING:
						dlt_user_log_write_string(a_context, va_arg(l_arguments, char*));
						break;
						
					case DLT_ARG_INT8:
                        dlt_user_log_write_int8(a_context, va_arg(l_arguments, var_sint8));
						break;

					case DLT_ARG_INT16:
                        dlt_user_log_write_int16(a_context, va_arg(l_arguments, var_sint16));
						break;

					case DLT_ARG_INT32:
                        dlt_user_log_write_int32(a_context, va_arg(l_arguments, var_sint32));
						break;

					case DLT_ARG_INT64:
                        dlt_user_log_write_int64(a_context, va_arg(l_arguments, var_sint64));
						break;
						
					case DLT_ARG_UINT8:
                        dlt_user_log_write_uint8(a_context, va_arg(l_arguments, var_uint8));
						break;

                    case DLT_ARG_BOOL:
                        dlt_user_log_write_bool(a_context, va_arg(l_arguments, var_bool));
                        break;

					case DLT_ARG_UINT16:
                        dlt_user_log_write_uint16(a_context, va_arg(l_arguments, var_uint16));
						break;

					case DLT_ARG_UINT32:
                        dlt_user_log_write_uint32(a_context, va_arg(l_arguments, var_uint32));
						break;						

					case DLT_ARG_UINT64:
                        dlt_user_log_write_uint64(a_context, va_arg(l_arguments, var_uint64));
						break;						

					case DLT_ARG_FLOAT32:
                        dlt_user_log_write_float32(a_context, va_arg(l_arguments, var_float32));
						break;						

					case DLT_ARG_FLOAT64:
                        dlt_user_log_write_float64(a_context, va_arg(l_arguments, var_float64));
						break;								
						
					case DLT_ARG_RAW:
                        data = va_arg(l_arguments, void*);
                        len = va_arg(l_arguments, var_uint16);
                        dlt_user_log_write_raw(a_context, data,len);
						break;
				}
			}
			
			va_end(l_arguments);
			dlt_user_log_write_finish(a_context);
		}
        DLT_SEM_FREE(); 
		DLT_CALLBACK_DATA_AVAILABLE();
	}
	
	return 0;
}

int dlt_user_log_write_start_id(DltContext *log,int loglevel, uint32 messageid)
{
#ifdef DLT_USE_STORAGE_HEADER

	/* optional write storage header "DLT"+x01 */
	DltStorageHeader *storageheader;
#endif /* DLT_USE_STORAGE_HEADER */
  	
	/* check if message fits buffer */
	if( (dltuser.buf_write < dltuser.buf_read) )
	{
		if( (dltuser.buf_write + DLT_MESSAGE_MAX_SIZE) > dltuser.buf_read)
		{
			/* buffer is full, discard message */
			log->mcnt++;
			return 0;
		}
	}
	else if((dltuser.buf_write + DLT_MESSAGE_MAX_SIZE) > (dltuser.buffer + DLT_BUFFER_SIZE))
	{
		/* message could exceed end of buffer */
		/* check if message fits beginning of buffer */
		if((dltuser.buffer + DLT_MESSAGE_MAX_SIZE) > dltuser.buf_read)
		{
			/* message does not fit, discard */
			log->mcnt++;
			return 0;
		}
		else
		{
			/* message fits beginning of buffer */
			dltuser.buf_write = dltuser.buffer;
		}
	}
	
#ifdef DLT_USE_STORAGE_HEADER

	/* optional write storage header "DLT"+x01 */
	storageheader = (DltStorageHeader*) (dltuser.buf_write);
	storageheader->pattern[0] = 'D';
	storageheader->pattern[1] = 'L';
	storageheader->pattern[2] = 'T';
	storageheader->pattern[3] = 0x01;
	storageheader->seconds = 0;
	storageheader->microseconds = 0;
	storageheader->ecu[0] = 'E';
	storageheader->ecu[1] = 'C';
	storageheader->ecu[2] = 'U';
	storageheader->ecu[3] = 0;
	dltuser.buf_write += sizeof(DltStorageHeader);

#endif /* DLT_USE_STORAGE_HEADER */

#ifdef DLT_USE_SERIAL_HEADER

	/* optional write serial header "DLS"+x01 */
	dltuser.buf_write[0] = 'D';
	dltuser.buf_write[1] = 'L';
	dltuser.buf_write[2] = 'S';
	dltuser.buf_write[3] = 0x01;
	dltuser.buf_write += 4;

#endif /* DLT_USE_SERIAL_HEADER */

	/* write standardheader */
	dltuser.standardheader = (DltStandardHeader*) (dltuser.buf_write);
    dltuser.standardheader->htyp = DLT_HTYP_PROTOCOL_VERSION1 ;

#ifdef DLT_VERBOSE_MODE

    dltuser.standardheader->htyp |= DLT_HTYP_UEH;

#endif /* DLT_VERBOSE_MODE */

    #if (BYTE_ORDER==BIG_ENDIAN)
        dltuser.standardheader->htyp |= DLT_HTYP_MSBF;
	#endif
    dltuser.standardheader->mcnt = log->mcnt++;
   	dltuser.standardheader->len = sizeof(DltStandardHeader);
	dltuser.buf_write += sizeof(DltStandardHeader);

#ifdef DLT_VERBOSE_MODE

    dltuser.extendedheader = (DltExtendedHeader*) (dltuser.buf_write);
    dltuser.extendedheader->msin = DLT_MSIN_VERB | (DLTYPE_LOG << 1);

    if(loglevel < DLT_LOG_OFF)
        dltuser.extendedheader->msin |= ( DLT_LOG_OFF & 0x0f ) << 4;
    else
        dltuser.extendedheader->msin |= ( loglevel & 0x0f ) << 4;

    dltuser.extendedheader->noar = 0;
    dlt_set_id(dltuser.extendedheader->apid,log->appid);
    dlt_set_id(dltuser.extendedheader->ctid,log->contextid);
    dltuser.buf_write += sizeof(DltExtendedHeader);
    dltuser.standardheader->len += sizeof(DltExtendedHeader);

#else /* DLT_VERBOSE_MODE */

    /* write message id for non-verbose mode */
	memcpy(dltuser.buf_write,&(messageid),sizeof(uint32));
	dltuser.buf_write += sizeof(uint32);
	dltuser.standardheader->len += sizeof(uint32);

#endif /* DLT_VERBOSE_MODE */
	
	return 1;
}

int dlt_user_log_write_start(DltContext *log,int loglevel)
{
	return dlt_user_log_write_start_id(log,loglevel,DLT_USER_DEFAULT_MSGID);
}

int dlt_user_log_write_finish(DltContext *log)
{

	/* set len parameter to correct byte order */
    dltuser.standardheader->len = DLT_HTOBE_16(dltuser.standardheader->len);

	/* update pointer to end of buffer */
	if( (dltuser.buf_write >= dltuser.buf_read) )
	{
			dltuser.buf_end = dltuser.buf_write;
	}
	

	return 0;
}

int dlt_user_log_read(unsigned char **data)
{
	uint16 length;
	
	DLT_SEM_LOCK();

  if((dltuser.buf_end == dltuser.buf_read) && (dltuser.buf_write < dltuser.buf_read))
	{
    dltuser.buf_read = dltuser.buffer;
    dltuser.buf_end = dltuser.buf_write;
  }
  
	*data = dltuser.buf_read;
	length = dltuser.buf_end-dltuser.buf_read;
	
	DLT_SEM_FREE();
	
	return length;
}

int dlt_user_log_read_ack(uint16 length)
{
	DLT_SEM_LOCK();

	if(dltuser.buf_write > dltuser.buf_read )
	{
		if(length > (dltuser.buf_end-dltuser.buf_read))
		{
			DLT_SEM_FREE();

			return -1;
		}
		else
		{
			dltuser.buf_read += length;

			DLT_SEM_FREE();

			return 0;
		}
	}
	else
	{
		if(length > (dltuser.buf_end-dltuser.buf_read))
		{
			DLT_SEM_FREE();

			return -1;
		}
		else if(length == (dltuser.buf_end-dltuser.buf_read))
		{
			dltuser.buf_end = dltuser.buf_write;
			dltuser.buf_read = dltuser.buffer;

			DLT_SEM_FREE();

			return 0;
		}
		else
		{
			dltuser.buf_read += length;

			DLT_SEM_FREE();

			return 0;
		}
	}
	
}

int dlt_user_log_write_buffer(DltContext *log,const void *data, uint16 length, uint32 type,int with_length)
{
    uint16 lengthvalue;
#ifdef DLT_VERBOSE_MODE
    uint32 *dltype;
#endif /* DLT_VERBOSE_MODE */

#ifdef DLT_VERBOSE_MODE
    if((dltuser.standardheader->len + length + (with_length?sizeof(uint16):0) + sizeof(uint32)) > DLT_MESSAGE_MAX_SIZE)
#else /* DLT_VERBOSE_MODE */
        if((dltuser.standardheader->len + length + (with_length?sizeof(uint16):0)) > DLT_MESSAGE_MAX_SIZE)
#endif /* DLT_VERBOSE_MODE */
    {
        /* data fits not max message size, return error */
        return -1;
    }

#ifdef DLT_VERBOSE_MODE

    dltype = (uint32*) dltuser.buf_write;
    *dltype = type;
    dltuser.buf_write += sizeof(uint32);
    dltuser.standardheader->len += sizeof(uint32);

#endif /* DLT_VERBOSE_MODE */

    if(with_length)
    {
        lengthvalue = length;
        memcpy(dltuser.buf_write,&lengthvalue,sizeof(uint16));
        dltuser.buf_write += sizeof(uint16);
        dltuser.standardheader->len += sizeof(uint16);
    }

	/* write data into buffer and increase pointers */
	memcpy(dltuser.buf_write,data,length);
	dltuser.buf_write += length;
	dltuser.standardheader->len += length;

#ifdef DLT_VERBOSE_MODE

    dltuser.extendedheader->noar += 1;

#endif /* DLT_VERBOSE_MODE */

	/* data is written into buffer */
	return 0;
}

int dlt_user_log_write_bool(DltContext *log, uint8 data)
{
    return dlt_user_log_write_buffer(log,&data,sizeof(uint8),DLTYPE_INFO_BOOL,0);
}

int dlt_user_log_write_float32(DltContext *log, float data)
{
    return dlt_user_log_write_buffer(log,&data,sizeof(float),DLTYPE_INFO_FLOA | DLTYLE_32BIT,0);
}

int dlt_user_log_write_float64(DltContext *log, double data)
{
    return dlt_user_log_write_buffer(log,&data,sizeof(double),DLTYPE_INFO_FLOA | DLTYLE_64BIT,0);
}

int dlt_user_log_write_uint(DltContext *log, unsigned int data)
{
    return dlt_user_log_write_buffer(log,&data,sizeof(unsigned int),DLTYPE_INFO_UINT | DLTYLE_32BIT,0);
}

int dlt_user_log_write_uint8(DltContext *log, uint8 data)
{
    return dlt_user_log_write_buffer(log,&data,sizeof(uint8),DLTYPE_INFO_UINT | DLTYLE_8BIT,0);
}

int dlt_user_log_write_uint16(DltContext *log, uint16 data)
{
    return dlt_user_log_write_buffer(log,&data,sizeof(uint16),DLTYPE_INFO_UINT | DLTYLE_16BIT,0);
}

int dlt_user_log_write_uint32(DltContext *log, uint32 data)
{
    return dlt_user_log_write_buffer(log,&data,sizeof(uint32),DLTYPE_INFO_UINT | DLTYLE_32BIT,0);
}

int dlt_user_log_write_uint64(DltContext *log, uint64 data)
{
    return dlt_user_log_write_buffer(log,&data,sizeof(uint64),DLTYPE_INFO_UINT | DLTYLE_64BIT,0);
}

int dlt_user_log_write_int(DltContext *log, int data)
{
    return dlt_user_log_write_buffer(log,&data,sizeof(int),DLTYPE_INFO_SINT | DLTYLE_32BIT,0);
}

int dlt_user_log_write_int8(DltContext *log, sint8 data)
{
    return dlt_user_log_write_buffer(log,&data,sizeof(sint8),DLTYPE_INFO_SINT | DLTYLE_8BIT,0);
}

int dlt_user_log_write_int16(DltContext *log, sint16 data)
{
    return dlt_user_log_write_buffer(log,&data,sizeof(sint16),DLTYPE_INFO_SINT | DLTYLE_16BIT,0);
}

int dlt_user_log_write_int32(DltContext *log, sint32 data)
{
    return dlt_user_log_write_buffer(log,&data,sizeof(sint32),DLTYPE_INFO_SINT | DLTYLE_32BIT,0);
}

int dlt_user_log_write_int64(DltContext *log, sint64 data)
{
    return dlt_user_log_write_buffer(log,&data,sizeof(sint64),DLTYPE_INFO_SINT | DLTYLE_64BIT,0);
}

int dlt_user_log_write_string( DltContext *log, const char *text)
{
	uint16 length;

	length=strlen(text)+1;
    dlt_user_log_write_buffer(log,text,length,DLTYPE_INFO_STRG,1);

    return 0;
}

int dlt_user_log_write_raw(DltContext *log,void *data,uint16 length)
{
    dlt_user_log_write_buffer(log,data,length,DLTYPE_INFO_RAWD,1);

    return 0;
}

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
 * \file dlt_common.c
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_common.c                                                  **
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
**              Mikko Rapeli               BMW                                **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision: 1613 $
 * $LastChangedDate: 2011-02-28 14:54:27 +0100 (Mo, 28. Feb 2011) $
 * $LastChangedBy$
 Initials    Date         Comment
 aw          13.01.2010   initial
 */
#include <stdio.h>
#include <stdlib.h> /* for malloc(), free() */
#include <string.h> /* for strlen(), memcmp(), memmove() */
#include <time.h>   /* for localtime(), strftime() */

#include "dlt_common.h"
#include "dlt_common_cfg.h"

#include "version.h"

#if defined (__WIN32__) || defined (_MSC_VER)
#include <winsock2.h> /* for socket(), connect(), send(), and recv() */
#include <stdint.h>
#else
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <syslog.h>
#include <time.h> /* for clock_gettime() */
#endif

#include <inttypes.h> /* for PRIu64 and PRId64 */

#if defined (__APPLE__)
#include <mach/mach_time.h>
#endif

#if defined (_MSC_VER)
#include <io.h>
#else
#include <unistd.h>     /* for read(), close() */
#include <sys/time.h>	/* for gettimeofday() */
#endif

#if defined (__MSDOS__) || defined (_MSC_VER)
#pragma warning(disable : 4996) /* Switch off C4996 warnings */
#include <windows.h>
#include <winbase.h>
#endif

static char str[DLT_COMMON_BUFFER_LENGTH];

const char dltSerialHeader[] = { 'D','L','S',1 };
char dltSerialHeaderChar[] = { 'D','L','S',1 };

static int log_as_daemon = 0;

char *message_type[] = {"log","app_trace","nw_trace","control","","","",""};
char *log_info[] = {"","fatal","error","warn","info","debug","verbose","","","","","","","","",""};
char *trace_type[] = {"","variable","func_in","func_out","state","vfb","","","","","","","","","",""};
char *nw_trace_type[] = {"","ipc","can","flexray","most","vfb","","","","","","","","","",""};
char *control_type[] = {"","request","response","time","","","","","","","","","","","",""};
static char *service_id[] = {"","set_log_level","set_trace_status","get_log_info","get_default_log_level","store_config","reset_to_factory_default",
                             "set_com_interface_status","set_com_interface_max_bandwidth","set_verbose_mode","set_message_filtering","set_timing_packets",
                             "get_local_time","use_ecu_id","use_session_id","use_timestamp","use_extended_header","set_default_log_level","set_default_trace_status",
                             "get_software_version","message_buffer_overflow"
                            };
static char *return_type[] = {"ok","not_supported","error","","","","","","no_matching_context_id"};

void dlt_print_hex(uint8_t *ptr,int size)
{
    int num;

    if (ptr==0)
    {
        return;
    }

    for (num=0;num<size;num++)
    {
        if (num>0)
        {
            printf(" ");
        }

        printf("%.2x",((uint8_t*)ptr)[num]);
    }
}

int dlt_print_hex_string(char *text,int textlength,uint8_t *ptr,int size)
{
    int num;

    if ((ptr==0) || (text==0) || (textlength<=0) || (size<0))
    {
        return -1;
    }

    /* Length 3: AB_ , A is first digit of hex number, B is second digit of hex number, _ is space */
    if (textlength<(size*3))
    {
        dlt_log(LOG_ERR, "String does not fit binary data!\n");
        return -1;
    }

    for (num=0;num<size;num++)
    {
        if (num>0)
        {
            sprintf(text," ");
            text++;
        }

        sprintf(text,"%.2x",((uint8_t*)ptr)[num]);
        text+=2; /* 2 chars */
    }

    return 0;
}

int dlt_print_mixed_string(char *text,int textlength,uint8_t *ptr,int size,int html)
{
    int required_size = 0;
    int lines, rest, i;

    if ((ptr==0) || (text==0) || (textlength<=0) || (size<0))
    {
        return -1;
    }

    /* Check maximum required size and do a length check */
    if (html==0)
    {
        required_size=(DLT_COMMON_HEX_LINELEN+(2*DLT_COMMON_HEX_CHARS+(DLT_COMMON_HEX_CHARS-1))+DLT_COMMON_CHARLEN+DLT_COMMON_HEX_CHARS+DLT_COMMON_CHARLEN) *
                      ((size/DLT_COMMON_HEX_CHARS) + 1);
        /* Example: (8 chars line number + (2*16 chars + 15 spaces) + space + 16 ascii chars + CR) *
           ((size/16) lines + extra line for the rest) */
    }
    else
    {
        required_size=(DLT_COMMON_HEX_LINELEN+(2*DLT_COMMON_HEX_CHARS+(DLT_COMMON_HEX_CHARS-1))+DLT_COMMON_CHARLEN+DLT_COMMON_HEX_CHARS+4*DLT_COMMON_CHARLEN) *
                      ((size/DLT_COMMON_HEX_CHARS) + 1);
        /* Example: (8 chars line number + (2*16 chars + 15 spaces) + space + 16 ascii chars + 4 [HTML CR: <BR>]) *
           ((size/16) lines + extra line for the rest) */
    }

    if (textlength<required_size)
    {
        sprintf(str, "String does not fit mixed data (available=%d, required=%d) !\n", textlength, required_size);
        dlt_log(LOG_ERR, str);
        return -1;
    }

    /* print full lines */
    for (lines=0; lines< (size / DLT_COMMON_HEX_CHARS); lines++)
    {
        /* Line number */
        sprintf(text,"%.6x: ",lines * DLT_COMMON_HEX_CHARS);
        text+=DLT_COMMON_HEX_LINELEN; /* 'XXXXXX: ' */

        /* Hex-Output */
        /* It is not required to decrement textlength, as it was already checked, that
           there is enough space for the complete output */
        dlt_print_hex_string(text,textlength,(uint8_t*)(ptr+(lines*DLT_COMMON_HEX_CHARS)),DLT_COMMON_HEX_CHARS);
        text+=((2*DLT_COMMON_HEX_CHARS)+(DLT_COMMON_HEX_CHARS-1)); /* 32 characters + 15 spaces */

        sprintf(text," ");
        text+=DLT_COMMON_CHARLEN;

        /* Char-Output */
        /* It is not required to decrement textlength, as it was already checked, that
           there is enough space for the complete output */
        dlt_print_char_string(&text,textlength,(uint8_t*)(ptr+(lines*DLT_COMMON_HEX_CHARS)),DLT_COMMON_HEX_CHARS);

        if (html==0)
        {
            sprintf(text,"\n");
            text+=DLT_COMMON_CHARLEN;
        }
        else
        {
            sprintf(text,"<BR>");
            text+=(4*DLT_COMMON_CHARLEN);
        }
    }

    /* print partial line */
    rest = size % DLT_COMMON_HEX_CHARS;

    if (rest>0)
    {
        /* Line number */
        sprintf(text,"%.6x: ", (size / DLT_COMMON_HEX_CHARS) * DLT_COMMON_HEX_CHARS);
        text+=DLT_COMMON_HEX_LINELEN; /* 'XXXXXX: ' */

        /* Hex-Output */
        /* It is not required to decrement textlength, as it was already checked, that
           there is enough space for the complete output */
        dlt_print_hex_string(text,textlength,(uint8_t*)(ptr+ ((size / DLT_COMMON_HEX_CHARS) * DLT_COMMON_HEX_CHARS)),rest);
        text+=2*rest+(rest-1);

        for (i=0;i<(DLT_COMMON_HEX_CHARS-rest);i++)
        {
            sprintf(text," xx");
            text+=(3*DLT_COMMON_CHARLEN);
        }

        sprintf(text," ");
        text+=DLT_COMMON_CHARLEN;

        /* Char-Output */
        /* It is not required to decrement textlength, as it was already checked, that
           there is enough space for the complete output */
        dlt_print_char_string(&text,textlength,(uint8_t*)(ptr+ ((size / DLT_COMMON_HEX_CHARS) * DLT_COMMON_HEX_CHARS)),rest);
    }

    return 0;
}

int dlt_print_char_string(char **text,int textlength,uint8_t *ptr,int size)
{
    int num;

    if (text==0)
    {
        return -1;
    }

    if ((ptr==0) || (*text==0) || (textlength<=0) || (size<0))
    {
        return -1;
    }

    if (textlength< size)
    {
        dlt_log(LOG_ERR, "String does not fit character data!\n");
        return -1;
    }

    for (num=0;num<size;num++)
    {
        if ( (((char*)ptr)[num]<DLT_COMMON_ASCII_CHAR_SPACE) || (((char*)ptr)[num]>DLT_COMMON_ASCII_CHAR_TILDE) )
        {
            sprintf(*text,".");
        }
        else
        {
            /* replace < with . */
            if (((char*)ptr)[num]!=DLT_COMMON_ASCII_CHAR_LT)
            {
                sprintf(*text,"%c",((char *)ptr)[num]);
            }
            else
            {
                sprintf(*text,".");
            }
        }
        (*text)++;
    }

    return 0;
}

void dlt_print_id(char *text,const char *id)
{
    size_t i, len;

    if (text==0)
    {
        return;
    }

    /* Initialize text */
    for (i=0; i<DLT_ID_SIZE; i++)
    {
        text[i]='-';
    }

    text[DLT_ID_SIZE] = 0;

    len = ((strlen(id)<=DLT_ID_SIZE)?strlen(id):DLT_ID_SIZE);

    /* Check id*/
    for (i=0; i<len; i++)
        text[i] = id[i];
}

void dlt_set_id(char *id,const char *text)
{
    id[0] = 0;
    id[1] = 0;
    id[2] = 0;
    id[3] = 0;

    if (text==0)
    {
        return;
    }

    if (text[0]!=0)
    {
        id[0] = text[0];
    }
    else
    {
        return;
    }

    if (text[1]!=0)
    {
        id[1] = text[1];
    }
    else
    {
        return;
    }

    if (text[2]!=0)
    {
        id[2] = text[2];
    }
    else
    {
        return;
    }

    if (text[3]!=0)
    {
        id[3] = text[3];
    }
    else
    {
        return;
    }
}

void dlt_clean_string(char *text,int length)
{
    int num;

    if (text==0)
    {
        return;
    }

    for (num=0;num<length;num++)
    {
        if (text[num]=='\r' || text[num]=='\n')
        {
            text[num] = ' ';
        }
    }
}

int dlt_filter_init(DltFilter *filter,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (filter==0)
    {
        return -1;
    }

    filter->counter = 0;

    return 0;
}

int dlt_filter_free(DltFilter *filter,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (filter==0)
    {
        return -1;
    }

    return 0;
}

int dlt_filter_load(DltFilter *filter,const char *filename,int verbose)
{
    FILE *handle;
    char str1[DLT_COMMON_BUFFER_LENGTH];
    char apid[DLT_ID_SIZE],ctid[DLT_ID_SIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    if (filter==0)
    {
        return -1;
    }

    handle = fopen(filename,"r");
    if (handle == 0)
    {
        sprintf(str,"Filter file %s cannot be opened!\n",filename);
        dlt_log(LOG_ERR, str);
        return -1;
    }

    /* Reset filters */
    filter->counter=0;

    while (!feof(handle))
    {
        str1[0]=0;
        /* 254 == DLT_COMMON_BUFFER_LENGTH - 1 */
        if (fscanf(handle,"%254s",str1)!=1)
        {
            break;
        }
        if (str1[0]==0)
        {
            break;
        }
        printf(" %s",str1);
        if (strcmp(str1,"----")==0)
        {
            dlt_set_id(apid,"");
        }
        else
        {
            dlt_set_id(apid,str1);
        }

        str1[0]=0;
        /* 254 == DLT_COMMON_BUFFER_LENGTH - 1 */
        if (fscanf(handle,"%254s",str1)!=1)
        {
            break;
        }
        if (str1[0]==0)
        {
            break;
        }
        printf(" %s\r\n",str1);
        if (strcmp(str1,"----")==0)
        {
            dlt_set_id(ctid,"");
        }
        else
        {
            dlt_set_id(ctid,str1);
        }

        if (filter->counter<DLT_FILTER_MAX)
        {
            dlt_filter_add(filter,apid,ctid,verbose);
        }
        else
        {
            sprintf(str, "Maximum number (%d) of allowed filters reached!\n", DLT_FILTER_MAX);
            dlt_log(LOG_ERR, str);
            fclose(handle);
            return 0;
        }
    }

    fclose(handle);

    return 0;
}

int dlt_filter_save(DltFilter *filter,const char *filename,int verbose)
{
    FILE *handle;
    int num;
    char buf[DLT_COMMON_BUFFER_LENGTH];

    PRINT_FUNCTION_VERBOSE(verbose);

    if (filter==0)
    {
        return -1;
    }

    handle = fopen(filename,"w");
    if (handle == 0)
    {
        sprintf(str,"Filter file %s cannot be opened!\n",filename);
        dlt_log(LOG_ERR, str);
        return -1;
    }

    for (num=0;num<filter->counter;num++)
    {
        if (filter->apid[num][0]==0)
        {
            fprintf(handle,"---- ");
        }
        else
        {
            dlt_print_id(buf,filter->apid[num]);
            fprintf(handle,"%s ",buf);
        }
        if (filter->ctid[num][0]==0)
        {
            fprintf(handle,"---- ");
        }
        else
        {
            dlt_print_id(buf,filter->ctid[num]);
            fprintf(handle,"%s ",buf);
        }
    }

    fclose(handle);

    return 0;
}

int dlt_filter_find(DltFilter *filter,const char *apid,const char *ctid, int verbose)
{
    int num;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((filter==0) || (apid==0))
    {
        return -1;
    }

    for (num=0; num<filter->counter; num++)
    {
        if (memcmp(filter->apid[num],apid,DLT_ID_SIZE)==0)
        {
            /* apid matches, now check for ctid */
            if (ctid==0)
            {
                /* check if empty ctid matches */
                if (memcmp(filter->ctid[num],"",DLT_ID_SIZE)==0)
                {
                    return num;
                }
            }
            else
            {
                if (memcmp(filter->ctid[num],ctid,DLT_ID_SIZE)==0)
                {
                    return num;
                }
            }
        }
    }

    return -1; /* Not found */
}

int dlt_filter_add(DltFilter *filter,const char *apid,const char *ctid, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if ((filter==0) || (apid==0))
    {
        return -1;
    }

    if (filter->counter >= DLT_FILTER_MAX)
    {
        dlt_log(LOG_ERR, "Maximum numbers of allowed filters reached!\n");
        return -1;
    }

    /* add each filter (apid, ctid) only once to filter array */
    if (dlt_filter_find(filter,apid,ctid,verbose)<0)
    {
        /* filter not found, so add it to filter array */
        if (filter->counter<DLT_FILTER_MAX)
        {
            dlt_set_id(filter->apid[filter->counter],apid);
            dlt_set_id(filter->ctid[filter->counter],(ctid?ctid:""));

            filter->counter++;

            return 0;
        }
    }

    return -1;
}

int dlt_filter_delete(DltFilter *filter,const char *apid,const char *ctid, int verbose)
{
    int j,k;
    int found=0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((filter==0) || (apid==0))
    {
        return -1;
    }

    if (filter->counter>0)
    {
        /* Get first occurence of apid and ctid in filter array */
        for (j=0; j<filter->counter; j++)
        {
            if ((memcmp(filter->apid[j],apid,DLT_ID_SIZE)==0) &&
                (memcmp(filter->ctid[j],ctid,DLT_ID_SIZE)==0)
               )

            {
                found=1;
                break;
            }
        }

        if (found)
        {
            /* j is index */
            /* Copy from j+1 til end to j til end-1 */

            dlt_set_id(filter->apid[j],"");
            dlt_set_id(filter->ctid[j],"");

            for (k=j; k<(filter->counter-1); k++)
            {
                dlt_set_id(filter->apid[k],filter->apid[k+1]);
                dlt_set_id(filter->ctid[k],filter->ctid[k+1]);
            }

            filter->counter--;
            return 0;
        }
    }

    return -1;
}

int dlt_message_init(DltMessage *msg,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (msg==0)
    {
        return -1;
    }

    /* initalise structure parameters */
    msg->headersize = 0;
    msg->datasize = 0;

    msg->databuffer = 0;

    msg->storageheader = 0;
    msg->standardheader = 0;
    msg->extendedheader = 0;

    msg->found_serialheader = 0;

    return 0;
}

int dlt_message_free(DltMessage *msg,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (msg==0)
    {
        return -1;
    }
    /* delete databuffer if exists */
    if (msg->databuffer)
    {
        free(msg->databuffer);
    }
    msg->databuffer = 0;

    return 0;
}

int dlt_message_header(DltMessage *msg,char *text,int textlength,int verbose)
{
    return dlt_message_header_flags(msg,text,textlength,DLT_HEADER_SHOW_ALL,verbose);
}

int dlt_message_header_flags(DltMessage *msg,char *text,int textlength,int flags, int verbose)
{
    struct tm * timeinfo;
    char buffer [DLT_COMMON_BUFFER_LENGTH];

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((msg==0) || (text==0) || (textlength<=0))
    {
        return -1;
    }

    if ((flags<DLT_HEADER_SHOW_NONE) || (flags>DLT_HEADER_SHOW_ALL))
    {
        return -1;
    }

    text[0] = 0;

    if ((flags & DLT_HEADER_SHOW_TIME) == DLT_HEADER_SHOW_TIME)
    {
        /* print received time */
        timeinfo = localtime ((const time_t*)(&(msg->storageheader->seconds)));

        if (timeinfo!=0)
        {
            strftime (buffer,sizeof(buffer),"%Y/%m/%d %H:%M:%S",timeinfo);
            sprintf(text,"%s.%.6d ",buffer,msg->storageheader->microseconds);
        }
    }

    if ((flags & DLT_HEADER_SHOW_TMSTP) == DLT_HEADER_SHOW_TMSTP)
    {
        /* print timestamp if available */
        if ( DLT_IS_HTYP_WTMS(msg->standardheader->htyp) )
        {
            sprintf(text+strlen(text),"%10u ",msg->headerextra.tmsp);
        }
        else
        {
            sprintf(text+strlen(text),"---------- ");
        }
    }

    if ((flags & DLT_HEADER_SHOW_MSGCNT) == DLT_HEADER_SHOW_MSGCNT)
    {
        /* print message counter */
        sprintf(text+strlen(text),"%.3d ",msg->standardheader->mcnt);
    }

    if ((flags & DLT_HEADER_SHOW_ECUID) == DLT_HEADER_SHOW_ECUID)
    {
        /* print ecu id, use header extra if available, else storage header value */
        if ( DLT_IS_HTYP_WEID(msg->standardheader->htyp) )
        {
            dlt_print_id(text+strlen(text),msg->headerextra.ecu);
        }
        else
        {
            dlt_print_id(text+strlen(text),msg->storageheader->ecu);
        }
    }

    /* print app id and context id if extended header available, else '----' */#
    if ((flags & DLT_HEADER_SHOW_APID) == DLT_HEADER_SHOW_APID)
    {
        sprintf(text+strlen(text)," ");
        if ((DLT_IS_HTYP_UEH(msg->standardheader->htyp)) && (msg->extendedheader->apid[0]!=0))
        {
            dlt_print_id(text+strlen(text),msg->extendedheader->apid);
        }
        else
        {
            sprintf(text+strlen(text),"----");
        }

        sprintf(text+strlen(text)," ");
    }

    if ((flags & DLT_HEADER_SHOW_CTID) == DLT_HEADER_SHOW_CTID)
    {
        if ((DLT_IS_HTYP_UEH(msg->standardheader->htyp)) && (msg->extendedheader->ctid[0]!=0))
        {
            dlt_print_id(text+strlen(text),msg->extendedheader->ctid);
        }
        else
        {
            sprintf(text+strlen(text),"----");
        }

        sprintf(text+strlen(text)," ");
    }

    /* print info about message type and length */
    if (DLT_IS_HTYP_UEH(msg->standardheader->htyp))
    {
        if ((flags & DLT_HEADER_SHOW_MSGTYPE) == DLT_HEADER_SHOW_MSGTYPE)
        {
            sprintf(text+strlen(text),"%s",message_type[DLT_GET_MSIN_MSTP(msg->extendedheader->msin)]);
            sprintf(text+strlen(text)," ");
        }

        if ((flags & DLT_HEADER_SHOW_MSGSUBTYPE) == DLT_HEADER_SHOW_MSGSUBTYPE)
        {
            if ((DLT_GET_MSIN_MSTP(msg->extendedheader->msin))==DLT_TYPE_LOG)
            {
                sprintf(text+strlen(text),"%s",log_info[DLT_GET_MSIN_MTIN(msg->extendedheader->msin)]);
            }

            if ((DLT_GET_MSIN_MSTP(msg->extendedheader->msin))==DLT_TYPE_APP_TRACE)
            {
                sprintf(text+strlen(text),"%s",trace_type[DLT_GET_MSIN_MTIN(msg->extendedheader->msin)]);
            }

            if ((DLT_GET_MSIN_MSTP(msg->extendedheader->msin))==DLT_TYPE_NW_TRACE)
            {
                sprintf(text+strlen(text),"%s",nw_trace_type[DLT_GET_MSIN_MTIN(msg->extendedheader->msin)]);
            }

            if ((DLT_GET_MSIN_MSTP(msg->extendedheader->msin))==DLT_TYPE_CONTROL)
            {
                sprintf(text+strlen(text),"%s",control_type[DLT_GET_MSIN_MTIN(msg->extendedheader->msin)]);
            }

            sprintf(text+strlen(text)," ");
        }

        if ((flags & DLT_HEADER_SHOW_VNVSTATUS) == DLT_HEADER_SHOW_VNVSTATUS)
        {
            /* print verbose status pf message */
            if (DLT_IS_MSIN_VERB(msg->extendedheader->msin))
            {
                sprintf(text+strlen(text),"V");
            }
            else
            {
                sprintf(text+strlen(text),"N");
            }

            sprintf(text+strlen(text)," ");
        }

        if ((flags & DLT_HEADER_SHOW_NOARG) == DLT_HEADER_SHOW_NOARG)
        {
            /* print number of arguments */
            sprintf(text+strlen(text),"%d", msg->extendedheader->noar);
        }

    }
    else
    {
        if ((flags & DLT_HEADER_SHOW_MSGTYPE) == DLT_HEADER_SHOW_MSGTYPE)
        {
            sprintf(text+strlen(text),"--- ");
        }

        if ((flags & DLT_HEADER_SHOW_MSGSUBTYPE) == DLT_HEADER_SHOW_MSGSUBTYPE)
        {
            sprintf(text+strlen(text),"--- ");
        }

        if ((flags & DLT_HEADER_SHOW_VNVSTATUS) == DLT_HEADER_SHOW_VNVSTATUS)
        {
            sprintf(text+strlen(text),"N ");
        }

        if ((flags & DLT_HEADER_SHOW_NOARG) == DLT_HEADER_SHOW_NOARG)
        {
            sprintf(text+strlen(text),"-");
        }
    }

    return 0;
}

int dlt_message_payload(DltMessage *msg,char *text,int textlength,int type,int verbose)
{
    uint32_t id=0,id_tmp=0;
    uint8_t retval=0;

    uint8_t *ptr;
    int32_t datalength;

    /* Pointer to ptr and datalength */
    uint8_t **pptr;
    int32_t *pdatalength;

    int ret=0;

    int num;
    uint32_t type_info=0,type_info_tmp=0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((msg==0) || (text==0))
    {
        return -1;
    }

    if (textlength<=0)
    {
        dlt_log(LOG_ERR, "String does not fit binary data!\n");
        return -1;
    }

    /* start with empty string */
    text[0] = 0;

    /* print payload only as hex */
    if (type==DLT_OUTPUT_HEX)
    {
        return dlt_print_hex_string(text,textlength,msg->databuffer,msg->datasize);
    }

    /* print payload as mixed */
    if (type==DLT_OUTPUT_MIXED_FOR_PLAIN)
    {
        return dlt_print_mixed_string(text,textlength,msg->databuffer,msg->datasize,0);
    }

    if (type==DLT_OUTPUT_MIXED_FOR_HTML)
    {
        return dlt_print_mixed_string(text,textlength,msg->databuffer,msg->datasize,1);
    }

    ptr = msg->databuffer;
    datalength = msg->datasize;

    /* Pointer to ptr and datalength */
    pptr = &ptr;
    pdatalength = &datalength;

    /* non-verbose mode */

    /* print payload as hex */
    if (DLT_MSG_IS_NONVERBOSE(msg))
    {

        DLT_MSG_READ_VALUE(id_tmp,ptr,datalength,uint32_t);
        id=DLT_ENDIAN_GET_32(msg->standardheader->htyp, id_tmp);

        if (textlength<((datalength*3)+20))
        {
            dlt_log(LOG_ERR, "String does not fit binary data!\n");
            return -1;
        }

        /* process message id / service id */
        if (DLT_MSG_IS_CONTROL(msg))
        {
            if (id > 0 && id <= DLT_SERVICE_ID_MESSAGE_BUFFER_OVERFLOW)
            {
                sprintf(text+strlen(text),"%s",service_id[id]); /* service id */
            }
            else
            {
                if (!(DLT_MSG_IS_CONTROL_TIME(msg)))
                {
                    sprintf(text+strlen(text),"service(%u)",id); /* service id */
                }
            }

            if (datalength>0)
            {
                sprintf(text+strlen(text),", ");
            }
        }
        else
        {
            sprintf(text+strlen(text),"%u, ",id); /* message id */
        }

        /* process return value */
        if (DLT_MSG_IS_CONTROL_RESPONSE(msg))
        {
            if (datalength>0)
            {
                DLT_MSG_READ_VALUE(retval,ptr,datalength,uint8_t); /* No endian conversion necessary */
                if ( (retval<3) || (retval==8))
                {
                    sprintf(text+strlen(text),"%s",return_type[retval]);
                }
                else
                {
                    sprintf(text+strlen(text),"%.2x",retval);
                }

                if (datalength>=1)
                {
                    sprintf(text+strlen(text),", ");
                }
            }
        }

        if (type==DLT_OUTPUT_ASCII_LIMITED)
        {
            ret=dlt_print_hex_string(text+strlen(text),(int)(textlength-strlen(text)),ptr,
                                     (datalength>DLT_COMMON_ASCII_LIMIT_MAX_CHARS?DLT_COMMON_ASCII_LIMIT_MAX_CHARS:datalength));
            if ((datalength>DLT_COMMON_ASCII_LIMIT_MAX_CHARS) &&
                    ((textlength-strlen(text))>4))
            {
                sprintf(text+strlen(text)," ...");
            }
        }
        else
        {
            ret=dlt_print_hex_string(text+strlen(text),(int)(textlength-strlen(text)),ptr,datalength);
        }

        return ret;
    }

    /* At this point, it is ensured that a extended header is available */

    /* verbose mode */
    // type_info=0;
    type_info_tmp=0;

    for (num=0;num<(int)(msg->extendedheader->noar);num++)
    {
        if (num!=0)
        {
            sprintf(text+strlen(text)," ");
        }

        /* first read the type info of the argument */
        DLT_MSG_READ_VALUE(type_info_tmp,ptr,datalength,uint32_t);
        type_info=DLT_ENDIAN_GET_32(msg->standardheader->htyp, type_info_tmp);

        /* print out argument */
        if (dlt_message_argument_print(msg, type_info, pptr, pdatalength, text, textlength, -1, 0)==-1)
        {
            return -1;
        }
    }

    return 0;
}

int dlt_message_filter_check(DltMessage *msg,DltFilter *filter,int verbose)
{
    /* check the filters if message is used */
    int num;
    int found = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((msg==0) || (filter==0))
    {
        return -1;
    }

    if ((filter->counter==0) || (!(DLT_IS_HTYP_UEH(msg->standardheader->htyp))))
    {
        /* no filter is set, or no extended header is available, so do as filter is matching */
        return 1;
    }

    for (num=0;num<filter->counter;num++)
    {
        /* check each filter if it matches */
        if ((DLT_IS_HTYP_UEH(msg->standardheader->htyp)) &&
                (filter->apid[num][0]==0 || memcmp(filter->apid[num],msg->extendedheader->apid,DLT_ID_SIZE)==0) &&
                (filter->ctid[num][0]==0 || memcmp(filter->ctid[num],msg->extendedheader->ctid,DLT_ID_SIZE)==0) )
        {
            found = 1;
            break;
        }
    }

    return found;
}

int dlt_message_read(DltMessage *msg,uint8_t *buffer,unsigned int length,int resync, int verbose)
{
    int extra_size = 0;

    PRINT_FUNCTION_VERBOSE(verbose);

    if ((msg==0) || (buffer==0) || (length<=0))
    {
        return -1;
    }

    /* initialize resync_offset */
    msg->resync_offset=0;

    /* check if message contains serial header, smaller than standard header */
    if (length<sizeof(dltSerialHeader))
    {
        /* dlt_log(LOG_ERR, "Length smaller than serial header!\n"); */
        return -1;
    }

    if (memcmp(buffer,dltSerialHeader,sizeof(dltSerialHeader)) == 0)
    {
        /* serial header found */
        msg->found_serialheader = 1;
        buffer += sizeof(dltSerialHeader);
        length -= sizeof(dltSerialHeader);
    }
    else
    {
        /* serial header not found */
        msg->found_serialheader = 0;
        if (resync)
        {
            /* resync if necessary */
            msg->resync_offset=0;

            do
            {
                if (memcmp(buffer+msg->resync_offset,dltSerialHeader,sizeof(dltSerialHeader)) == 0)
                {
                    /* serial header found */
                    msg->found_serialheader = 1;
                    buffer += sizeof(dltSerialHeader);
                    length -= sizeof(dltSerialHeader);
                    break;
                }

                msg->resync_offset++;
            }
            while ((sizeof(dltSerialHeader)+msg->resync_offset)<=length);

            /* Set new start offset */
            if (msg->resync_offset>0)
            {
                /* Resyncing connection */
                buffer+=msg->resync_offset;
                length-=msg->resync_offset;
            }
        }
    }

    /* check that standard header fits buffer */
    if (length<sizeof(DltStandardHeader))
    {
        /* dlt_log(LOG_ERR, "Length smaller than standard header!\n"); */
        return -1;
    }
    memcpy(msg->headerbuffer+sizeof(DltStorageHeader),buffer,sizeof(DltStandardHeader));

    /* set ptrs to structures */
    msg->storageheader = (DltStorageHeader*) msg->headerbuffer;
    msg->standardheader = (DltStandardHeader*) (msg->headerbuffer + sizeof(DltStorageHeader));

    /* calculate complete size of headers */
    extra_size = DLT_STANDARD_HEADER_EXTRA_SIZE(msg->standardheader->htyp)+(DLT_IS_HTYP_UEH(msg->standardheader->htyp) ? sizeof(DltExtendedHeader) : 0);
    msg->headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) + extra_size;
    msg->datasize =  DLT_BETOH_16(msg->standardheader->len) - (msg->headersize - sizeof(DltStorageHeader));

    if (verbose)
    {
        sprintf(str,"Buffer length: %d\n",length);
        dlt_log(LOG_INFO, str);
    }
    if (verbose)
    {
        sprintf(str,"Header Size: %d\n",msg->headersize);
        dlt_log(LOG_INFO, str);
    }
    if (verbose)
    {
        sprintf(str,"Data Size: %d\n",msg->datasize);
        dlt_log(LOG_INFO, str);
    }

    /* check data size */
    if (msg->datasize < 0)
    {
        sprintf(str,"Plausibility check failed. Complete message size too short (%d)!\n",msg->datasize);
        dlt_log(LOG_ERR, str);
        return -1;
    }

    /* load standard header extra parameters and Extended header if used */
    if (extra_size>0)
    {
        if (length  < (msg->headersize - sizeof(DltStorageHeader)))
        {
            return -1;
        }

        memcpy(msg->headerbuffer+sizeof(DltStorageHeader)+sizeof(DltStandardHeader),buffer+sizeof(DltStandardHeader),extra_size);

        /* set extended header ptr and get standard header extra parameters */
        if (DLT_IS_HTYP_UEH(msg->standardheader->htyp))
        {
            msg->extendedheader = (DltExtendedHeader*) (msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) +
                                  DLT_STANDARD_HEADER_EXTRA_SIZE(msg->standardheader->htyp));
        }
        else
        {
            msg->extendedheader = 0;
        }

        dlt_message_get_extraparameters(msg,verbose);
    }

    /* check if payload fits length */
    if (length  < (msg->headersize - sizeof(DltStorageHeader) + msg->datasize))
    {
        /* dlt_log(LOG_ERR,"length does not fit!\n"); */
        return -1;
    }

    /* free last used memory for buffer */
    if (msg->databuffer)
    {
        free(msg->databuffer);
    }

    /* get new memory for buffer */
    msg->databuffer = (uint8_t *)malloc(msg->datasize);
    if (msg->databuffer == 0)
    {
        sprintf(str,"Cannot allocate memory for payload buffer of size %d!\n",msg->datasize);
        dlt_log(LOG_ERR, str);
        return -1;
    }

    /* load payload data from buffer */
    memcpy(msg->databuffer,buffer+(msg->headersize-sizeof(DltStorageHeader)),msg->datasize);

    return 0;
}

int dlt_message_get_extraparameters(DltMessage *msg,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (msg==0)
    {
        return -1;
    }

    if (DLT_IS_HTYP_WEID(msg->standardheader->htyp))
    {
        memcpy(msg->headerextra.ecu,msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader),DLT_ID_SIZE);
    }

    if (DLT_IS_HTYP_WSID(msg->standardheader->htyp))
    {
        memcpy(&(msg->headerextra.seid),msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader)
               + (DLT_IS_HTYP_WEID(msg->standardheader->htyp) ? DLT_SIZE_WEID : 0), DLT_SIZE_WSID);
        msg->headerextra.seid = DLT_BETOH_32(msg->headerextra.seid);
    }

    if (DLT_IS_HTYP_WTMS(msg->standardheader->htyp))
    {
        memcpy(&(msg->headerextra.tmsp),msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader)
               + (DLT_IS_HTYP_WEID(msg->standardheader->htyp) ? DLT_SIZE_WEID : 0)
               + (DLT_IS_HTYP_WSID(msg->standardheader->htyp) ? DLT_SIZE_WSID : 0),DLT_SIZE_WTMS);
        msg->headerextra.tmsp = DLT_BETOH_32(msg->headerextra.tmsp);
    }

    return 0;
}

int dlt_message_set_extraparameters(DltMessage *msg,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (msg==0)
    {
        return -1;
    }

    if (DLT_IS_HTYP_WEID(msg->standardheader->htyp))
    {
        memcpy(msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader),msg->headerextra.ecu,DLT_ID_SIZE);
    }

    if (DLT_IS_HTYP_WSID(msg->standardheader->htyp))
    {
        msg->headerextra.seid = DLT_HTOBE_32(msg->headerextra.seid);
        memcpy(msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader)
               + (DLT_IS_HTYP_WEID(msg->standardheader->htyp) ? DLT_SIZE_WEID : 0), &(msg->headerextra.seid),DLT_SIZE_WSID);
    }

    if (DLT_IS_HTYP_WTMS(msg->standardheader->htyp))
    {
        msg->headerextra.tmsp = DLT_HTOBE_32(msg->headerextra.tmsp);
        memcpy(msg->headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader)
               + (DLT_IS_HTYP_WEID(msg->standardheader->htyp) ? DLT_SIZE_WEID : 0)
               + (DLT_IS_HTYP_WSID(msg->standardheader->htyp) ? DLT_SIZE_WSID : 0), &(msg->headerextra.tmsp),DLT_SIZE_WTMS);
    }

    return 0;
}

int dlt_file_init(DltFile *file,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file==0)
    {
        return -1;
    }

    /* initalise structure parameters */
    file->handle = 0;
    file->counter = 0;
    file->counter_total = 0;
    file->index = 0;

    file->filter = 0;
    file->filter_counter = 0;
    file->file_position = 0;

    file->position = 0;

    file->error_messages = 0;

    return dlt_message_init(&(file->msg),verbose);
}

int dlt_file_set_filter(DltFile *file,DltFilter *filter,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file==0)
    {
        return -1;
    }

    /* set filter */
    file->filter = filter;

    return 0;
}

int dlt_file_read_header(DltFile *file,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file==0)
    {
        return -1;
    }

    /* load header from file */
    if (fread(file->msg.headerbuffer,sizeof(DltStorageHeader)+sizeof(DltStandardHeader),1,file->handle)!=1)
    {
        if (!feof(file->handle))
        {
            dlt_log(LOG_ERR, "Cannot read header from file!\n");
        }
        return -1;
    }

    /* set ptrs to structures */
    file->msg.storageheader = (DltStorageHeader*) file->msg.headerbuffer;
    file->msg.standardheader = (DltStandardHeader*) (file->msg.headerbuffer + sizeof(DltStorageHeader));

    /* check id of storage header */
    if (dlt_check_storageheader(file->msg.storageheader)==0)
    {
        dlt_log(LOG_ERR, "DLT storage header pattern not found!\n");
        return -1;
    }

    /* calculate complete size of headers */
    file->msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) +
                           DLT_STANDARD_HEADER_EXTRA_SIZE(file->msg.standardheader->htyp) + (DLT_IS_HTYP_UEH(file->msg.standardheader->htyp) ? sizeof(DltExtendedHeader) : 0);
    file->msg.datasize = DLT_BETOH_16(file->msg.standardheader->len) + sizeof(DltStorageHeader) - file->msg.headersize;
    if (verbose)
    {
        sprintf(str,"Header Size: %d\n",file->msg.headersize);
        dlt_log(LOG_INFO, str);
    }
    if (verbose)
    {
        sprintf(str,"Data Size: %d\n",file->msg.datasize);
        dlt_log(LOG_INFO, str);
    }

    /* check data size */
    if (file->msg.datasize < 0)
    {
        sprintf(str,"Plausibility check failed. Complete message size too short! (%d)\n", file->msg.datasize);
        dlt_log(LOG_ERR, str);
        return -1;
    }

    return 0;
}

int dlt_file_read_header_raw(DltFile *file,int resync,int verbose)
{
    char dltSerialHeaderBuffer[DLT_ID_SIZE];

    PRINT_FUNCTION_VERBOSE(verbose);

    if (file==0)
    {
        return -1;
    }

    /* check if serial header exists, ignore if found */
    if (fread(dltSerialHeaderBuffer,sizeof(dltSerialHeaderBuffer),1,file->handle)!=1)
    {
        /* cannot read serial header, not enough data available in file */
        if (!feof(file->handle))
        {
            dlt_log(LOG_ERR, "Cannot read header from file!\n");
        }
        return -1;
    }
    if (memcmp(dltSerialHeaderBuffer,dltSerialHeader,sizeof(dltSerialHeader)) == 0)
    {
        /* serial header found */
        /* nothing to do continue reading */

    }
    else
    {
        /* serial header not found */
        if(resync)
        {
            /* increase error counter */
            file->error_messages++;

            /* resync to serial header */
            do
            {
                memmove(dltSerialHeaderBuffer,dltSerialHeaderBuffer+1,sizeof(dltSerialHeader)-1);
                if (fread(dltSerialHeaderBuffer+3,1,1,file->handle)!=1)
                {
                    /* cannot read any data, perhaps end of file reached */
                    return -1;
                }
                if (memcmp(dltSerialHeaderBuffer,dltSerialHeader,sizeof(dltSerialHeader)) == 0)
                {
                    /* serial header synchronised */
                    break;
                }
            } while(1);
        }
        else
        {
            /* go back to last file position */
            fseek(file->handle,file->file_position,SEEK_SET);
        }
    }

    /* load header from file */
    if (fread(file->msg.headerbuffer+sizeof(DltStorageHeader),sizeof(DltStandardHeader),1,file->handle)!=1)
    {
        if (!feof(file->handle))
        {
            dlt_log(LOG_ERR, "Cannot read header from file!\n");
        }
        return -1;
    }

    /* set ptrs to structures */
    file->msg.storageheader = (DltStorageHeader*) file->msg.headerbuffer; // this points now to a empty storage header (filled with '0')
    file->msg.standardheader = (DltStandardHeader*) (file->msg.headerbuffer + sizeof(DltStorageHeader));

    /* Skip storage header field, fill this field with '0' */
    memset(file->msg.storageheader,0,sizeof(DltStorageHeader));

    /* Set storage header */
    dlt_set_storageheader(file->msg.storageheader,DLT_COMMON_DUMMY_ECUID);

    /* no check for storage header id*/

    /* calculate complete size of headers */
    file->msg.headersize = sizeof(DltStorageHeader) + sizeof(DltStandardHeader) +
                           DLT_STANDARD_HEADER_EXTRA_SIZE(file->msg.standardheader->htyp) + (DLT_IS_HTYP_UEH(file->msg.standardheader->htyp) ? sizeof(DltExtendedHeader) : 0);
    file->msg.datasize = DLT_BETOH_16(file->msg.standardheader->len) + sizeof(DltStorageHeader) - file->msg.headersize;
    if (verbose)
    {
        sprintf(str,"Header Size: %d\n",file->msg.headersize);
        dlt_log(LOG_INFO, str);
    }
    if (verbose)
    {
        sprintf(str,"Data Size: %d\n",file->msg.datasize);
        dlt_log(LOG_INFO, str);
    }

    /* check data size */
    if (file->msg.datasize < 0)
    {
        sprintf(str,"Plausibility check failed. Complete message size too short! (%d)\n", file->msg.datasize);
        dlt_log(LOG_ERR, str);
        return -1;
    }

    return 0;
}

int dlt_file_read_header_extended(DltFile *file, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file==0)
    {
        return -1;
    }

    /* load standard header extra parameters if used */
    if (DLT_STANDARD_HEADER_EXTRA_SIZE(file->msg.standardheader->htyp))
    {
        if (fread(file->msg.headerbuffer+sizeof(DltStorageHeader)+sizeof(DltStandardHeader),
                  DLT_STANDARD_HEADER_EXTRA_SIZE(file->msg.standardheader->htyp),
                  1,file->handle)!=1)
        {
            dlt_log(LOG_ERR, "Cannot read standard header extra parameters from file!\n");
            return -1;
        }

        dlt_message_get_extraparameters(&(file->msg),verbose);
    }

    /* load Extended header if used */
    if (DLT_IS_HTYP_UEH(file->msg.standardheader->htyp)==0)
    {
        /* there is nothing to be loaded */
        return 0;
    }

    if (fread(file->msg.headerbuffer+sizeof(DltStorageHeader)+sizeof(DltStandardHeader)+DLT_STANDARD_HEADER_EXTRA_SIZE(file->msg.standardheader->htyp),
              (DLT_IS_HTYP_UEH(file->msg.standardheader->htyp) ? sizeof(DltExtendedHeader) : 0),
              1,file->handle)!=1)
    {
        dlt_log(LOG_ERR, "Cannot read extended header from file!\n");
        return -1;
    }

    /* set extended header ptr */
    if (DLT_IS_HTYP_UEH(file->msg.standardheader->htyp))
    {
        file->msg.extendedheader = (DltExtendedHeader*) (file->msg.headerbuffer + sizeof(DltStorageHeader) + sizeof(DltStandardHeader) +
                                   DLT_STANDARD_HEADER_EXTRA_SIZE(file->msg.standardheader->htyp));
    }
    else
    {
        file->msg.extendedheader = 0;
    }

    return 0;
}

int dlt_file_read_data(DltFile *file, int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file==0)
    {
        return -1;
    }

    /* free last used memory for buffer */
    if (file->msg.databuffer)
    {
        free(file->msg.databuffer);
    }

    /* get new memory for buffer */
    file->msg.databuffer = (uint8_t *)malloc(file->msg.datasize);

    if (file->msg.databuffer == 0)
    {
        sprintf(str,"Cannot allocate memory for payload buffer of size %d!\n",file->msg.datasize);
        dlt_log(LOG_ERR, str);
        return -1;
    }

    /* load payload data from file */
    if (fread(file->msg.databuffer,file->msg.datasize,1,file->handle)!=1)
    {
        if (file->msg.datasize!=0)
        {
            sprintf(str,"Cannot read payload data from file of size %d!\n",file->msg.datasize);
            dlt_log(LOG_ERR, str);
            return -1;
        }
    }

    return 0;
}

int dlt_file_open(DltFile *file,const char *filename,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file==0)
    {
        return -1;
    }

    /* reset counters */
    file->counter = 0;
    file->counter_total = 0;
    file->position = 0;
    file->file_position = 0;
    file->file_length = 0;
    file->error_messages = 0;

    if (file->handle)
    {
        fclose(file->handle);
    }

    /* open dlt file */
    file->handle = fopen(filename,"rb");
    if (file->handle == 0)
    {
        sprintf(str,"File %s cannot be opened!\n",filename);
        dlt_log(LOG_ERR, str);
        return -1;
    }

    fseek(file->handle,0,SEEK_END);
    file->file_length = ftell(file->handle);
    fseek(file->handle,0,SEEK_SET);

    if (verbose)
    {
        /* print file length */
        sprintf(str,"File is %lu bytes long\n",file->file_length);
        dlt_log(LOG_INFO, str);
    }
    return 0;
}

int dlt_file_read(DltFile *file,int verbose)
{
    long *ptr;
    int found = 0;

    if (verbose)
    {
        sprintf(str,"%s: Message %d:\n",__func__, file->counter_total);
        dlt_log(LOG_INFO, str);
    }

    if (file==0)
    {
        return -1;
    }

    /* allocate new memory for index if number of messages exceeds a multiple of DLT_COMMON_INDEX_ALLOC (e.g.: 1000) */
    if (file->counter % DLT_COMMON_INDEX_ALLOC == 0)
    {
        ptr = (long *) malloc(((file->counter/DLT_COMMON_INDEX_ALLOC) + 1) * DLT_COMMON_INDEX_ALLOC * sizeof(long));

        if (ptr==0)
        {
            return -1;
        }

        if (file->index)
        {
            memcpy(ptr,file->index,file->counter * sizeof(long));
            free(file->index);
        }
        file->index = ptr;
    }

    /* set to end of last succesful read message, because of conflicting calls to dlt_file_read and dlt_file_message */
    fseek(file->handle,file->file_position,SEEK_SET);

    /* get file position at start of DLT message */
    if (verbose)
    {
        sprintf(str,"Position in file: %ld\n",file->file_position);
        dlt_log(LOG_INFO, str);
    }

    /* read header */
    if (dlt_file_read_header(file,verbose)<0)
    {
        /* go back to last position in file */
        fseek(file->handle,file->file_position,SEEK_SET);
        return -1;
    }

    if (file->filter)
    {
        /* read the extended header if filter is enabled and extended header exists */
        if (dlt_file_read_header_extended(file, verbose)<0)
        {
            /* go back to last position in file */
            fseek(file->handle,file->file_position,SEEK_SET);
            return-1;
        }

        /* check the filters if message is used */
        if (dlt_message_filter_check(&(file->msg),file->filter,verbose) == 1)
        {
            /* filter matched, consequently store current message */
            /* store index pointer to message position in DLT file */
            file->index[file->counter] = file->file_position;
            file->counter++;
            file->position = file->counter - 1;

            found = 1;
        }

        /* skip payload data */
        if (fseek(file->handle,file->msg.datasize,SEEK_CUR)!=0)
        {
            /* go back to last position in file */
            fseek(file->handle,file->file_position,SEEK_SET);
            sprintf(str,"Seek failed to skip payload data of size %d!\n",file->msg.datasize);
            dlt_log(LOG_ERR, str);
            return -1;
        }
    }
    else
    {
        /* filter is disabled */
        /* skip additional header parameters and payload data */
        if (fseek(file->handle,file->msg.headersize - sizeof(DltStorageHeader) - sizeof(DltStandardHeader) + file->msg.datasize,SEEK_CUR))
        {
            /* go back to last position in file */
            fseek(file->handle,file->file_position,SEEK_SET);
            sprintf(str,"Seek failed to skip extra header and payload data from file of size %zu!\n",
                    file->msg.headersize - sizeof(DltStorageHeader) - sizeof(DltStandardHeader) + file->msg.datasize);
            dlt_log(LOG_ERR, str);
            return -1;
        }

        /* store index pointer to message position in DLT file */
        file->index[file->counter] = file->file_position;
        file->counter++;
        file->position = file->counter - 1;

        found = 1;
    }

    /* increase total message counter */
    file->counter_total++;

    /* store position to next message */
    file->file_position = ftell(file->handle);

    return found;
}

int dlt_file_read_raw(DltFile *file,int resync, int verbose)
{
    int found = 0;
    long *ptr;

    if (verbose)
    {
        sprintf(str,"%s: Message %d:\n",__func__, file->counter_total);
        dlt_log(LOG_INFO, str);
    }

    if (file==0)
        return -1;

    /* allocate new memory for index if number of messages exceeds a multiple of DLT_COMMON_INDEX_ALLOC (e.g.: 1000) */
    if (file->counter % DLT_COMMON_INDEX_ALLOC == 0)
    {
        ptr = (long *) malloc(((file->counter/DLT_COMMON_INDEX_ALLOC) + 1) * DLT_COMMON_INDEX_ALLOC * sizeof(long));

        if (ptr==0)
        {
            return -1;
        }

        if (file->index)
        {
            memcpy(ptr,file->index,file->counter * sizeof(long));
            free(file->index);
        }
        file->index = ptr;
    }

    /* set to end of last succesful read message, because of conflicting calls to dlt_file_read and dlt_file_message */
    fseek(file->handle,file->file_position,SEEK_SET);

    /* get file position at start of DLT message */
    if (verbose)
    {
        sprintf(str,"Position in file: %ld\n",file->file_position);
        dlt_log(LOG_INFO, str);
    }

    /* read header */
    if (dlt_file_read_header_raw(file,resync,verbose)<0)
    {
        /* go back to last position in file */
        fseek(file->handle,file->file_position,SEEK_SET);
        return -1;
    }

    /* read the extended header if filter is enabled and extended header exists */
    if (dlt_file_read_header_extended(file, verbose)<0)
    {
        /* go back to last position in file */
        fseek(file->handle,file->file_position,SEEK_SET);
        return-1;
    }

    if (dlt_file_read_data(file,verbose)<0)
    {
        /* go back to last position in file */
        fseek(file->handle,file->file_position,SEEK_SET);
        return-1;
    }

    /* store index pointer to message position in DLT file */
    file->index[file->counter] = file->file_position;
    file->counter++;
    file->position = file->counter - 1;

    found = 1;

    /* increase total message counter */
    file->counter_total++;

    /* store position to next message */
    file->file_position = ftell(file->handle);

    return found;
}

int dlt_file_close(DltFile *file,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file==0)
    {
        return -1;
    }

    if (file->handle)
    {
        fclose(file->handle);
    }

    file->handle = 0;

    return 0;
}

int dlt_file_message(DltFile *file,int index,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file==0)
    {
        return -1;
    }

    /* check if message is in range */
    if (index >= file->counter)
    {
        sprintf(str,"Message %d out of range!\r\n",index);
        dlt_log(LOG_ERR, str);
        return -1;
    }

    /* seek to position in file */
    if (fseek(file->handle,file->index[index],SEEK_SET)!=0)
    {
        sprintf(str,"Seek to message %d to position %ld failed!\r\n",index,file->index[index]);
        dlt_log(LOG_ERR, str);
        return -1;
    }

    /* read all header and payload */
    if (dlt_file_read_header(file,verbose)<0)
    {
        return -1;
    }

    if (dlt_file_read_header_extended(file,verbose)<0)
    {
        return -1;
    }

    if (dlt_file_read_data(file,verbose)<0)
    {
        return -1;
    }

    /* set current position in file */
    file->position = index;

    return 0;
}

int dlt_file_free(DltFile *file,int verbose)
{
    PRINT_FUNCTION_VERBOSE(verbose);

    if (file==0)
    {
        return -1;
    }

    /* delete index lost if exists */
    if (file->index)
    {
        free(file->index);
    }
    file->index = 0;

    /* close file */
    if (file->handle)
    {
        fclose(file->handle);
    }
    file->handle = 0;

    return dlt_message_free(&(file->msg),verbose);
}

void dlt_log_init(int mode)
{
    log_as_daemon = mode;
}

void dlt_log_free(void)
{
    /* Nothing to be done yet */
}

int dlt_log(int prio, char *s)
{
    char logfmtstring[DLT_COMMON_BUFFER_LENGTH];

    if (s==0)
    {
        return -1;
    }

    switch (prio)
    {
        case	LOG_EMERG:
        {
            strcpy(logfmtstring,"DLT| EMERGENCY: %s");
            break;
        }
        case	LOG_ALERT:
        {
            strcpy(logfmtstring,"DLT| ALERT:     %s");
            break;
        }
        case	LOG_CRIT:
        {
            strcpy(logfmtstring,"DLT| CRITICAL:  %s");
            break;
        }
        case	LOG_ERR:
        {
            strcpy(logfmtstring,"DLT| ERROR:     %s");
            break;
        }
        case	LOG_WARNING:
        {
            strcpy(logfmtstring,"DLT| WARNING:   %s");
            break;
        }
        case	LOG_NOTICE:
        {
            strcpy(logfmtstring,"DLT| NOTICE:    %s");
            break;
        }
        case	LOG_INFO:
        {
            strcpy(logfmtstring,"DLT| INFO:      %s");
            break;
        }
        case	LOG_DEBUG:
        {
            strcpy(logfmtstring,"DLT| DEBUG:     %s");
            break;
        }
        default:
        {
            strcpy(logfmtstring,"DLT|            %s");
            break;
        }
    }

#if !defined (__WIN32__) && !defined(_MSC_VER)
    if (log_as_daemon)
    {
        openlog("DLT",LOG_PID,LOG_DAEMON);
        syslog(prio, logfmtstring, s);
        closelog();
    }
    else
#endif
    {
        printf(logfmtstring, s);
    }

    return 0;
}

int dlt_receiver_init(DltReceiver *receiver,int fd, int buffersize)
{
    if (receiver==0)
    {
        return -1;
    }

    receiver->lastBytesRcvd = 0;
    receiver->bytesRcvd = 0;
    receiver->totalBytesRcvd = 0;
    receiver->buffersize = buffersize;
    receiver->fd = fd;
    receiver->buffer = (char*)malloc(receiver->buffersize);

    if (receiver->buffer == 0)
    {
        receiver->buf = 0;
        return -1;
    }
    else
    {
        receiver->buf = receiver->buffer;
    }

    return 0;
}

int dlt_receiver_free(DltReceiver *receiver)
{

    if (receiver==0)
    {
        return -1;
    }

    if (receiver->buffer)
    {
        free(receiver->buffer);
    }

    receiver->buffer = 0;
    receiver->buf = 0;

    return 0;
}

#ifndef QT_VIEWER
int dlt_receiver_receive_socket(DltReceiver *receiver)
{
    if (receiver==0)
    {
        return -1;
    }

    if (receiver->buffer==0)
    {
        return -1;
    }

    receiver->buf = (char *)receiver->buffer;
    receiver->lastBytesRcvd = receiver->bytesRcvd;

    /* wait for data from socket */
    if ((receiver->bytesRcvd = recv(receiver->fd, receiver->buf + receiver->lastBytesRcvd, receiver->buffersize - receiver->lastBytesRcvd , 0)) <= 0)
    {
        receiver->bytesRcvd = 0;

        return receiver->bytesRcvd;
    } /* if */

    receiver->totalBytesRcvd += receiver->bytesRcvd;
    receiver->bytesRcvd += receiver->lastBytesRcvd;

    return receiver->bytesRcvd;
}
#endif

int dlt_receiver_receive_fd(DltReceiver *receiver)
{
    if (receiver==0)
    {
        return -1;
    }

    if (receiver->buffer==0)
    {
        return -1;
    }

    receiver->buf = (char *)receiver->buffer;
    receiver->lastBytesRcvd = receiver->bytesRcvd;

    /* wait for data from fd */
    if ((receiver->bytesRcvd = read(receiver->fd, receiver->buf + receiver->lastBytesRcvd, receiver->buffersize - receiver->lastBytesRcvd)) <= 0)
    {
        receiver->bytesRcvd = 0;

        return receiver->bytesRcvd;
    } /* if */

    receiver->totalBytesRcvd += receiver->bytesRcvd;
    receiver->bytesRcvd += receiver->lastBytesRcvd;

    return receiver->bytesRcvd;
}

int dlt_receiver_remove(DltReceiver *receiver,int size)
{
    if (receiver==0)
    {
        return -1;
    }

    if (receiver->buf==0)
    {
        return -1;
    }

    receiver->bytesRcvd = receiver->bytesRcvd - size;
    receiver->buf = receiver->buf + size;

    return 0;
}

int dlt_receiver_move_to_begin(DltReceiver *receiver)
{
    if (receiver==0)
    {
        return -1;
    }

    if ((receiver->buffer==0) || (receiver->buf==0))
    {
        return -1;
    }

    if ((receiver->buffer!=receiver->buf) && (receiver->bytesRcvd!=0))
    {
        memmove(receiver->buffer,receiver->buf,receiver->bytesRcvd);
    }

    return 0;
}

int dlt_set_storageheader(DltStorageHeader *storageheader, const char *ecu)
{

#if !defined(_MSC_VER)
    struct timeval tv;
#endif

    if (storageheader==0)
    {
        return -1;
    }

    /* get time of day */
#if defined(_MSC_VER)
    time((time_t*)&(storageheader->seconds));
#else
    gettimeofday(&tv, NULL);
#endif

    /* prepare storage header */
    storageheader->pattern[0] = 'D';
    storageheader->pattern[1] = 'L';
    storageheader->pattern[2] = 'T';
    storageheader->pattern[3] = 0x01;

    dlt_set_id(storageheader->ecu,ecu);

    /* Set current time */
#if defined(_MSC_VER)
    storageheader->microseconds = 0;
#else
    storageheader->seconds = (time_t)tv.tv_sec; /* value is long */
    storageheader->microseconds = (int32_t)tv.tv_usec; /* value is long */
#endif

    return 0;
}

int dlt_check_storageheader(DltStorageHeader *storageheader)
{
    if (storageheader==0)
    {
        return -1;
    }

    return  ((storageheader->pattern[0] == 'D') &&
             (storageheader->pattern[1] == 'L') &&
             (storageheader->pattern[2] == 'T') &&
             (storageheader->pattern[3] == 1));
}

int dlt_ringbuffer_init(DltRingBuffer *dltbuf, uint32_t size)
{

    if (dltbuf==0)
    {
        return -1;
    }

    if (size<=sizeof(uint32_t))
    {
        return -1;
    }

    dltbuf->buffer=(char*)malloc(size);
    if (dltbuf->buffer==0)
    {
        return -1;
    }

    dltbuf->size=size;

    dltbuf->pos_write=0;
    dltbuf->pos_read=0;

    dltbuf->count=0;

    return 0;
}

int dlt_ringbuffer_free(DltRingBuffer *dltbuf)
{

    if (dltbuf==0)
    {
        return -1;
    }

    if (dltbuf->buffer)
    {
        free(dltbuf->buffer);
    }

    dltbuf->buffer=0;

    dltbuf->size=0;

    dltbuf->pos_write=0;
    dltbuf->pos_read=0;

    dltbuf->count=0;

    return 0;
}

int dlt_ringbuffer_put(DltRingBuffer *dltbuf, void *data, uint32_t size)
{
    uint32_t sui, part1, part2;

    if (dltbuf==0)
    {
        return -1;
    }

    if (dltbuf->buffer==0)
    {
        return -1;
    }

    if (data==0)
    {
        return -1;
    }

    sui = sizeof(uint32_t);

    if ((size+sui)>dltbuf->size)
    {
        return -1;
    }

    dlt_ringbuffer_checkandfreespace(dltbuf, (size+sui));

    if (dltbuf->pos_write >= dltbuf->size)
    {
        dltbuf->pos_write = 0;
    }

    /* Not enough space for one uint available before end of linear buffer */
    /* Start at begin of linear buffer */
    if ((dltbuf->size - dltbuf->pos_write) < sui)
    {
        dltbuf->pos_write = 0;
    }

    /* Write length of following data to buffer */
    memcpy(&(dltbuf->buffer[dltbuf->pos_write]), &size, sui);
    dltbuf->pos_write+=sui;

    if (dltbuf->pos_write >= dltbuf->size)
    {
        dltbuf->pos_write = 0;
    }

    if ((dltbuf->size - dltbuf->pos_write) < size)
    {
        /* Not enough space til end of linear buffer, */
        /* split up write call */
        part1 = dltbuf->size - dltbuf->pos_write;
        part2 = size - part1;

        memcpy(dltbuf->buffer + dltbuf->pos_write, data, part1);
        memcpy(dltbuf->buffer, ((char*)data) + part1, part2);
        dltbuf->pos_write = part2;

    }
    else
    {
        /* Enough space til end of linear buffer */
        memcpy(&(dltbuf->buffer[dltbuf->pos_write]), data, size);
        dltbuf->pos_write+=size;
    }

    dltbuf->count++;

    return 0;
}


int dlt_ringbuffer_put3(DltRingBuffer *dltbuf, void *data1, uint32_t size1, void *data2, uint32_t size2, void *data3, uint32_t size3)
{
    uint32_t sui, part1, part2;
    uint32_t total_size;

    if (dltbuf==0)
    {
        return -1;
    }

    if (dltbuf->buffer==0)
    {
        return -1;
    }

    sui = sizeof(uint32_t);

    total_size = size1+size2+size3;

    if ((total_size+sui)>dltbuf->size)
    {
        return -1;
    }

    dlt_ringbuffer_checkandfreespace(dltbuf, (total_size+sui));

    if (dltbuf->pos_write >= dltbuf->size)
    {
        dltbuf->pos_write = 0;
    }

    /* Not enough space for one uint available before end of linear buffer */
    /* Start at begin of linear buffer */
    if ((dltbuf->size - dltbuf->pos_write) < sui)
    {
        dltbuf->pos_write = 0;
    }

    /* Write length of following data to buffer */
    memcpy(&(dltbuf->buffer[dltbuf->pos_write]), &total_size, sui);
    dltbuf->pos_write+=sui;

    if (dltbuf->pos_write >= dltbuf->size)
    {
        dltbuf->pos_write = 0;
    }

    /* First chunk of data (data1, size1) */
    if ((dltbuf->size - dltbuf->pos_write) < size1)
    {
        /* Not enough space til end of linear buffer, */
        /* split up write call */
        part1 = dltbuf->size - dltbuf->pos_write;
        part2 = size1 - part1;

        memcpy(dltbuf->buffer + dltbuf->pos_write, data1, part1);
        memcpy(dltbuf->buffer, ((char*)data1) + part1, part2);
        dltbuf->pos_write = part2;

    }
    else
    {
        /* Enough space til end of linear buffer */
        memcpy(&(dltbuf->buffer[dltbuf->pos_write]), data1, size1);
        dltbuf->pos_write+=size1;
    }

    if (dltbuf->pos_write >= dltbuf->size)
    {
        dltbuf->pos_write = 0;
    }

    /* Second chunk of data (data2, size2) */
    if ((dltbuf->size - dltbuf->pos_write) < size2)
    {
        /* Not enough space til end of linear buffer, */
        /* split up write call */
        part1 = dltbuf->size - dltbuf->pos_write;
        part2 = size2 - part1;

        memcpy(dltbuf->buffer + dltbuf->pos_write, data2, part1);
        memcpy(dltbuf->buffer, ((char*)data2) + part1, part2);
        dltbuf->pos_write = part2;

    }
    else
    {
        /* Enough space til end of linear buffer */
        memcpy(&(dltbuf->buffer[dltbuf->pos_write]), data2, size2);
        dltbuf->pos_write+=size2;
    }

    if (dltbuf->pos_write >= dltbuf->size)
    {
        dltbuf->pos_write = 0;
    }

    /* Third chunk of data (data3, size3) */
    if ((dltbuf->size - dltbuf->pos_write) < size3)
    {
        /* Not enough space til end of linear buffer, */
        /* split up write call */
        part1 = dltbuf->size - dltbuf->pos_write;
        part2 = size3 - part1;

        memcpy(dltbuf->buffer + dltbuf->pos_write, data3, part1);
        memcpy(dltbuf->buffer, ((char*)data3) + part1, part2);
        dltbuf->pos_write = part2;

    }
    else
    {
        /* Enough space til end of linear buffer */
        memcpy(dltbuf->buffer + dltbuf->pos_write, data3, size3);
        dltbuf->pos_write+=size3;
    }

    dltbuf->count++;

    return 0;
}

int dlt_ringbuffer_get(DltRingBuffer *dltbuf, void *data, size_t *size)
{
    uint32_t tmpsize=0;
    uint32_t sui;

    uint32_t part1, part2;

    if (dltbuf==0)
    {
        return -1;
    }

    if (dltbuf->buffer==0)
    {
        return -1;
    }

    if (dltbuf->count==0)
    {
        return -1;
    }

    sui = sizeof(uint32_t);

    if (dltbuf->pos_read >= dltbuf->size)
    {
        dltbuf->pos_read = 0;
    }

    if ((dltbuf->size - dltbuf->pos_read) < sui)
    {
        dltbuf->pos_read = 0;
    }

    /* printf("Reading at offset: %d\n", dltbuf->pos_read); */

    memcpy(&tmpsize,&(dltbuf->buffer[dltbuf->pos_read]), sui);
    dltbuf->pos_read += sui;

    if (dltbuf->pos_read >= dltbuf->size)
    {
        dltbuf->pos_read = 0;
    }

    if ((tmpsize>0) && ((tmpsize+sizeof(uint32_t))<=dltbuf->size))
    {
        if ((dltbuf->size - dltbuf->pos_read) < tmpsize)
        {
            /* Not enough space til end of linear buffer, */
            /* split up read call */
            part1 = dltbuf->size - dltbuf->pos_read;
            part2 = tmpsize - part1;

            memcpy(data, dltbuf->buffer + dltbuf->pos_read, part1);
            memcpy(((char*)data)+part1, dltbuf->buffer, part2);
            dltbuf->pos_read = part2;
        }
        else
        {
            /* Enough space til end of linear buffer */
            /* no split up read call */
            memcpy(data, &(dltbuf->buffer[dltbuf->pos_read]), tmpsize);
            dltbuf->pos_read+=tmpsize;
        }
        *size = tmpsize;
    }
    else
    {
        data=0;
        *size=0;
    }

    dltbuf->count--;

    return 0;
}

int dlt_ringbuffer_get_skip(DltRingBuffer *dltbuf)
{
    uint32_t tmpsize=0;
    uint32_t sui;

    uint32_t part1, part2;

    if (dltbuf==0)
    {
        return -1;
    }

    if (dltbuf->buffer==0)
    {
        return -1;
    }

    if (dltbuf->count==0)
    {
        return -1;
    }

    sui = sizeof(uint32_t);

    if (dltbuf->pos_read >= dltbuf->size)
    {
        dltbuf->pos_read = 0;
    }

    if ((dltbuf->size - dltbuf->pos_read) < sui)
    {
        dltbuf->pos_read = 0;
    }

    memcpy(&tmpsize,&(dltbuf->buffer[dltbuf->pos_read]), sui);
    dltbuf->pos_read += sui;

    if (dltbuf->pos_read >= dltbuf->size)
    {
        dltbuf->pos_read = 0;
    }

    if ((tmpsize>0) && ((tmpsize+sui)<=dltbuf->size))
    {
        if ((dltbuf->size - dltbuf->pos_read) < tmpsize)
        {
            /* Not enough space til end of linear buffer */
            part1 = dltbuf->size - dltbuf->pos_read;
            part2 = tmpsize - part1;

            dltbuf->pos_read = part2;
        }
        else
        {
            /* Enough space til end of linear buffer */
            dltbuf->pos_read+=tmpsize;
        }
    }

    dltbuf->count--;

    return 0;
}

int dlt_ringbuffer_freespacewrite(DltRingBuffer *dltbuf, uint32_t *freespace)
{
    if ((dltbuf==0) || (freespace==0))
    {
        return -1;
    }

    *freespace=0;

    /* Space til pos_read */
    if (dltbuf->pos_read > dltbuf->pos_write)
    {
        *freespace=(dltbuf->pos_read - dltbuf->pos_write);
        return 0;
    }
    else if (dltbuf->pos_read < dltbuf->pos_write)
    {
        *freespace=(dltbuf->size - dltbuf->pos_write + dltbuf->pos_read );
        return 0;
    }
    else
    {
        if (dltbuf->count)
        {
            return 0;
        }
        else
        {
            *freespace=dltbuf->size;
            return 0;
        }
    }
    return 0;
}

int dlt_ringbuffer_checkandfreespace(DltRingBuffer *dltbuf, uint32_t reqspace)
{
    uint32_t space_left;

    if (dltbuf==0)
    {
        return -1;
    }

    if (dlt_ringbuffer_freespacewrite(dltbuf,&space_left) == -1)
    {
        return -1;
    }

    /* printf("Now reading at: %d, space_left = %d, req = %d, r=%d, w=%d, count=%d \n",
              dltbuf->pos_read,space_left, reqspace, dltbuf->pos_read, dltbuf->pos_write, dltbuf->count); */

    while (space_left<reqspace)
    {
        /* Overwrite, correct read position */

        /* Read and skip one element */
        dlt_ringbuffer_get_skip(dltbuf);

        /* Space until pos_read */
        if (dlt_ringbuffer_freespacewrite(dltbuf,&space_left) == -1)
	    {
		    return -1;
	    }

        /* printf("Overwrite: Now reading at: %d, space_left = %d, req = %d, r=%d, w=%d, count=%d \n",
                  dltbuf->pos_read,space_left, reqspace, dltbuf->pos_read, dltbuf->pos_write, dltbuf->count); */
    }

    return 0;
}

#if !defined (__WIN32__)

int dlt_setup_serial(int fd, speed_t speed)
{
#if !defined (__WIN32__) && !defined(_MSC_VER)
    struct termios config;

    if (isatty(fd)==0)
    {
        return -1;
    }

    if (tcgetattr(fd, &config) < 0)
    {
        return -1;
    }

    /* Input flags - Turn off input processing
       convert break to null byte, no CR to NL translation,
       no NL to CR translation, don't mark parity errors or breaks
       no input parity check, don't strip high bit off,
       no XON/XOFF software flow control
    */
    config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                        INLCR | PARMRK | INPCK | ISTRIP | IXON);

    /* Output flags - Turn off output processing
       no CR to NL translation, no NL to CR-NL translation,
       no NL to CR translation, no column 0 CR suppression,
       no Ctrl-D suppression, no fill characters, no case mapping,
       no local output processing

       config.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
                           ONOCR | ONOEOT| OFILL | OLCUC | OPOST);
    */
    config.c_oflag = 0;

    /* No line processing:
       echo off, echo newline off, canonical mode off,
       extended input processing off, signal chars off
    */
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    /* Turn off character processing
       clear current char size mask, no parity checking,
       no output processing, force 8 bit input
    */
    config.c_cflag &= ~(CSIZE | PARENB);
    config.c_cflag |= CS8;

    /* One input byte is enough to return from read()
       Inter-character timer off
    */
    config.c_cc[VMIN]  = 1;
    config.c_cc[VTIME] = 0;

    /* Communication speed (simple version, using the predefined
       constants)
    */
    if (cfsetispeed(&config, speed) < 0 || cfsetospeed(&config, speed) < 0)
    {
        return -1;
    }

    /* Finally, apply the configuration
    */
    if (tcsetattr(fd, TCSAFLUSH, &config) < 0)
    {
        return -1;
    }

    return 0;
#else
    return -1;
#endif
}

speed_t dlt_convert_serial_speed(int baudrate)
{
#if !defined (__WIN32__) && !defined(_MSC_VER) && !defined(__APPLE__)
    speed_t ret;

    switch (baudrate)
    {
    case  50:
    {
        ret = B50;
        break;
    }
    case  75:
    {
        ret = B75;
        break;
    }
    case  110:
    {
        ret = B110;
        break;
    }
    case  134:
    {
        ret = B134;
        break;
    }
    case  150:
    {
        ret = B150;
        break;
    }
    case  200:
    {
        ret = B200;
        break;
    }
    case  300:
    {
        ret = B300;
        break;
    }
    case  600:
    {
        ret = B600;
        break;
    }
    case  1200:
    {
        ret = B1200;
        break;
    }
    case  1800:
    {
        ret = B1800;
        break;
    }
    case  2400:
    {
        ret = B2400;
        break;
    }
    case  4800:
    {
        ret = B4800;
        break;
    }
    case  9600:
    {
        ret = B9600;
        break;
    }
    case  19200:
    {
        ret = B19200;
        break;
    }
    case  38400:
    {
        ret = B38400;
        break;
    }
    case  57600:
    {
        ret = B57600;
        break;
    }
    case  115200:
    {
        ret = B115200;
        break;
    }
    case 230400:
    {
        ret = B230400;
        break;
    }
    case 460800:
    {
        ret = B460800;
        break;
    }
    case  500000:
    {
        ret = B500000;
        break;
    }
    case  576000:
    {
        ret = B576000;
        break;
    }
    case  921600:
    {
        ret = B921600;
        break;
    }
    case  1000000:
    {
        ret = B1000000;
        break;
    }
    case  1152000:
    {
        ret = B1152000;
        break;
    }
    case  1500000:
    {
        ret = B1500000;
        break;
    }
    case  2000000:
    {
        ret = B2000000;
        break;
    }
    case  2500000:
    {
        ret = B2500000;
        break;
    }
    case  3000000:
    {
        ret = B3000000;
        break;
    }
    case  3500000:
    {
        ret = B3500000;
        break;
    }
    case  4000000:
    {
        ret = B4000000;
        break;
    }
    default:
    {
        ret = B115200;
        break;
    }
    }

    return ret;
#else
    (void) baudrate;
    return 0;
#endif
}

#endif

void dlt_get_version(char *buf)
{
    sprintf(buf,"DLT Package Version: %s %s, Package Revision: %s, build on %s %s\n",
            PACKAGE_VERSION, PACKAGE_VERSION_STATE, PACKAGE_REVISION, __DATE__ , __TIME__ );
}

uint32_t dlt_uptime(void)
{

#if defined (__WIN32__) || defined(_MSC_VER)

    return (uint32_t)(GetTickCount()*10); /* GetTickCount() return DWORD */

#elif defined (__APPLE__)
    static mach_timebase_info_data_t sTimebaseInfo = { .numer = 0, .denom = 0 };
    if (0 == sTimebaseInfo.denom) {
        mach_timebase_info(&sTimebaseInfo);
    }
    uint64_t now = mach_absolute_time();
    uint64_t nano = now * sTimebaseInfo.numer / sTimebaseInfo.denom;

    return (uint32_t)(nano/100000);
#else
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC,&ts)==0)
    {
        return (uint32_t)((((ts.tv_sec*1000000)+(ts.tv_nsec/1000)))/100); // in 0.1 ms = 100 us
    }
    else
    {
        return 0;
    }

#endif

}

int dlt_message_print_header(DltMessage *message, char *text, uint32_t size, int verbose)
{
    if ((message==0) || (text==0))
    {
        return -1;
    }

    dlt_message_header(message,text,size,verbose);
    printf("%s\n",text);

    return 0;
}

int dlt_message_print_hex(DltMessage *message, char *text, uint32_t size, int verbose)
{
    if ((message==0) || (text==0))
    {
        return -1;
    }

    dlt_message_header(message,text,size,verbose);
    printf("%s ",text);
    dlt_message_payload(message,text,size,DLT_OUTPUT_HEX,verbose);
    printf("[%s]\n",text);

    return 0;
}

int dlt_message_print_ascii(DltMessage *message, char *text, uint32_t size, int verbose)
{
    if ((message==0) || (text==0))
    {
        return -1;
    }

    dlt_message_header(message,text,size,verbose);
    printf("%s ",text);
    dlt_message_payload(message,text,size,DLT_OUTPUT_ASCII,verbose);
    printf("[%s]\n",text);

    return 0;
}

int dlt_message_print_mixed_plain(DltMessage *message, char *text, uint32_t size, int verbose)
{
    if ((message==0) || (text==0))
    {
        return -1;
    }

    dlt_message_header(message,text,size,verbose);
    printf("%s \n",text);
    dlt_message_payload(message,text,size,DLT_OUTPUT_MIXED_FOR_PLAIN,verbose);
    printf("[%s]\n",text);

    return 0;
}

int dlt_message_print_mixed_html(DltMessage *message, char *text, uint32_t size, int verbose)
{
    if ((message==0) || (text==0))
    {
        return -1;
    }

    dlt_message_header(message,text,size,verbose);
    printf("%s \n",text);
    dlt_message_payload(message,text,size,DLT_OUTPUT_MIXED_FOR_HTML,verbose);
    printf("[%s]\n",text);

    return 0;
}

int dlt_message_argument_print(DltMessage *msg,uint32_t type_info,uint8_t **ptr,int32_t *datalength,char *text,int textlength,int byteLength,int verbose)
{
    (void) verbose; // unused

    int16_t length=0,length_tmp=0; /* the macro can set this variable to -1 */
    uint16_t length2=0,length2_tmp=0,length3=0,length3_tmp=0;

    uint8_t value8u=0;
    uint16_t value16u=0,value16u_tmp=0;
    uint32_t value32u=0,value32u_tmp=0;
    uint64_t value64u=0,value64u_tmp=0;

    int8_t  value8i=0;
    int16_t value16i=0,value16i_tmp=0;
    int32_t value32i=0,value32i_tmp=0;
    int64_t value64i=0,value64i_tmp=0;

    float32_t value32f=0,value32f_tmp=0;
    int32_t value32f_tmp_int32i=0,value32f_tmp_int32i_swaped=0;
    float64_t value64f=0,value64f_tmp=0;
    int64_t value64f_tmp_int64i=0,value64f_tmp_int64i_swaped=0;

    if (type_info & DLT_TYPE_INFO_STRG)
    {

        /* string type */
        if (byteLength<0)
        {
            DLT_MSG_READ_VALUE(length_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return -1;
            length=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length_tmp);
        }
        else
        {
            length=(int16_t)byteLength;
        }

        if (type_info & DLT_TYPE_INFO_VARI)
        {
            DLT_MSG_READ_VALUE(length2_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return -1;
            length2=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length2_tmp);
            if((*datalength)<length2)
                return -1;
            *ptr += length2;
            *datalength-=length2;
        }

        DLT_MSG_READ_STRING((text+strlen(text)),*ptr,*datalength,length);
        if((*datalength)<0)
            return -1;

    }
    else if (type_info & DLT_TYPE_INFO_BOOL)
    {
        /* Boolean type */
        if (type_info & DLT_TYPE_INFO_VARI)
        {
            DLT_MSG_READ_VALUE(length2_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return -1;
            length2=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length2_tmp);
            if((*datalength)<length2)
                return -1;
            *ptr += length2;
            *datalength-=length2;
        }
        value8u=0;
        DLT_MSG_READ_VALUE(value8u,*ptr,*datalength,uint8_t); /* No endian conversion necessary */
        if((*datalength)<0)
            return -1;
        sprintf(text+strlen(text),"%d",value8u);
    }
    else if (type_info & DLT_TYPE_INFO_SINT || type_info & DLT_TYPE_INFO_UINT)
    {
        /* signed or unsigned argument received */
        if (type_info & DLT_TYPE_INFO_VARI)
        {
            DLT_MSG_READ_VALUE(length2_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return -1;
            length2=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length2_tmp);
            DLT_MSG_READ_VALUE(length3_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return -1;
            length3=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length3_tmp);
            if((*datalength)<length2)
                return -1;
            *ptr += length2;
            *datalength-=length2;
            if((*datalength)<length3)
                return -1;
            *ptr += length3;
            *datalength-=length3;
        }
        if (type_info & DLT_TYPE_INFO_FIXP)
        {
            if((*datalength)<0)
                return -1;

            switch (	type_info & DLT_TYPE_INFO_TYLE)
            {
				case DLT_TYLE_8BIT:
				case DLT_TYLE_16BIT:
				case DLT_TYLE_32BIT:
				{
                    if((*datalength)<4)
                        return -1;
                    *ptr += 4;
					*datalength-=4;
					break;
				}
				case DLT_TYLE_64BIT:
				{
                    if((*datalength)<8)
                        return -1;
                    *ptr += 8;
					*datalength-=8;
					break;
				}
				case DLT_TYLE_128BIT:
				{
                    if((*datalength)<16)
                        return -1;
                    *ptr += 16;
					*datalength-=16;
					break;
				}
				default:
				{
					return -1;
				}
            }
        }
        switch (	type_info & DLT_TYPE_INFO_TYLE)
        {
			case DLT_TYLE_8BIT:
			{
				if (type_info & DLT_TYPE_INFO_SINT)
				{
					value8i=0;
					DLT_MSG_READ_VALUE(value8i,*ptr,*datalength,int8_t); /* No endian conversion necessary */
                    if((*datalength)<0)
                        return -1;
                    sprintf(text+strlen(text),"%d",value8i);
				}
				else
				{
					value8u=0;
					DLT_MSG_READ_VALUE(value8u,*ptr,*datalength,uint8_t); /* No endian conversion necessary */
                    if((*datalength)<0)
                        return -1;
                    sprintf(text+strlen(text),"%d",value8u);
				}
				break;
			}
			case DLT_TYLE_16BIT:
			{
				if (type_info & DLT_TYPE_INFO_SINT)
				{
                    // value16i=0;
					value16i_tmp=0;
					DLT_MSG_READ_VALUE(value16i_tmp,*ptr,*datalength,int16_t);
                    if((*datalength)<0)
                        return -1;
                    value16i=DLT_ENDIAN_GET_16(msg->standardheader->htyp, value16i_tmp);
					sprintf(text+strlen(text),"%hd",value16i);
				}
				else
				{
                    // value16u=0;
					value16u_tmp=0;
					DLT_MSG_READ_VALUE(value16u_tmp,*ptr,*datalength,uint16_t);
                    if((*datalength)<0)
                        return -1;
                    value16u=DLT_ENDIAN_GET_16(msg->standardheader->htyp, value16u_tmp);
					sprintf(text+strlen(text),"%hu",value16u);
				}
				break;
			}
			case DLT_TYLE_32BIT:
			{
				if (type_info & DLT_TYPE_INFO_SINT)
				{
                    // value32i=0;
					value32i_tmp=0;
					DLT_MSG_READ_VALUE(value32i_tmp,*ptr,*datalength,int32_t);
                    if((*datalength)<0)
                        return -1;
                    value32i=DLT_ENDIAN_GET_32(msg->standardheader->htyp, (uint32_t)value32i_tmp);
					sprintf(text+strlen(text),"%d",value32i);
				}
				else
				{
                    // value32u=0;
					value32u_tmp=0;
					DLT_MSG_READ_VALUE(value32u_tmp,*ptr,*datalength,uint32_t);
                    if((*datalength)<0)
                        return -1;
                    value32u=DLT_ENDIAN_GET_32(msg->standardheader->htyp, value32u_tmp);
					sprintf(text+strlen(text),"%u",value32u);
				}
				break;
			}
			case DLT_TYLE_64BIT:
			{
				if (type_info & DLT_TYPE_INFO_SINT)
				{
                    // value64i=0;
					value64i_tmp=0;
					DLT_MSG_READ_VALUE(value64i_tmp,*ptr,*datalength,int64_t);
                    if((*datalength)<0)
                        return -1;
                    value64i=DLT_ENDIAN_GET_64(msg->standardheader->htyp, (uint64_t)value64i_tmp);
	#if defined (__WIN32__) && !defined(_MSC_VER)
					sprintf(text+strlen(text),"%I64d",value64i);
	#else
					sprintf(text+strlen(text),"%" PRId64,value64i);
	#endif
				}
				else
				{
                    // value64u=0;
					value64u_tmp=0;
					DLT_MSG_READ_VALUE(value64u_tmp,*ptr,*datalength,uint64_t);
                    if((*datalength)<0)
                        return -1;
                    value64u=DLT_ENDIAN_GET_64(msg->standardheader->htyp, value64u_tmp);
	#if defined (__WIN32__) && !defined(_MSC_VER)
					sprintf(text+strlen(text),"%I64u",value64u);
	#else
					sprintf(text+strlen(text),"%" PRIu64,value64u);
	#endif
				}
				break;
			}
			case DLT_TYLE_128BIT:
			{
				if (*datalength>=16)
					dlt_print_hex_string(text+strlen(text),textlength,*ptr,16);
                if((*datalength)<16)
                    return -1;
                *ptr += 16;
				*datalength-=16;
				break;
			}
			default:
			{
				return -1;
			}
        }
    }
    else if (type_info & DLT_TYPE_INFO_FLOA)
    {
        /* float data argument */
        if (type_info & DLT_TYPE_INFO_VARI)
        {
            DLT_MSG_READ_VALUE(length2_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return -1;
            length2=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length2_tmp);
            DLT_MSG_READ_VALUE(length3_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return -1;
            length3=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length3_tmp);
            if((*datalength)<length2)
                return -1;
            *ptr += length2;
            *datalength-=length2;
            if((*datalength)<length3)
                return -1;
            *ptr += length3;
            *datalength-=length3;
        }
        switch (	type_info & DLT_TYPE_INFO_TYLE)
        {
			case DLT_TYLE_8BIT:
			{
				if (*datalength>=1)
					dlt_print_hex_string(text+strlen(text),textlength,*ptr,1);
                if((*datalength)<1)
                    return -1;
                *ptr += 1;
				*datalength-=1;
				break;
			}
			case DLT_TYLE_16BIT:
			{
				if (*datalength>=2)
					dlt_print_hex_string(text+strlen(text),textlength,*ptr,2);
                if((*datalength)<2)
                    return -1;
                *ptr += 2;
				*datalength-=2;
				break;
			}
			case DLT_TYLE_32BIT:
			{
				if (sizeof(float32_t)==4)
				{
					value32f=0;
					value32f_tmp=0;
					value32f_tmp_int32i=0;
					value32f_tmp_int32i_swaped=0;
					DLT_MSG_READ_VALUE(value32f_tmp,*ptr,*datalength,float32_t);
                    if((*datalength)<0)
                        return -1;
                    memcpy(&value32f_tmp_int32i,&value32f_tmp,sizeof(float32_t));
					value32f_tmp_int32i_swaped=DLT_ENDIAN_GET_32(msg->standardheader->htyp, (uint32_t)value32f_tmp_int32i);
					memcpy(&value32f,&value32f_tmp_int32i_swaped,sizeof(float32_t));
					sprintf(text+strlen(text),"%g",value32f);
				}
				else
				{
					dlt_log(LOG_ERR, "Invalid size of float32_t\n");
					return -1;
				}
				break;
			}
			case DLT_TYLE_64BIT:
			{
				if (sizeof(float64_t)==8)
				{
					value64f=0;
					value64f_tmp=0;
					value64f_tmp_int64i=0;
					value64f_tmp_int64i_swaped=0;
					DLT_MSG_READ_VALUE(value64f_tmp,*ptr,*datalength,float64_t);
                    if((*datalength)<0)
                        return -1;
                    memcpy(&value64f_tmp_int64i,&value64f_tmp,sizeof(float64_t));
					value64f_tmp_int64i_swaped=DLT_ENDIAN_GET_64(msg->standardheader->htyp, (uint64_t)value64f_tmp_int64i);
					memcpy(&value64f,&value64f_tmp_int64i_swaped,sizeof(float64_t));
					sprintf(text+strlen(text),"%g",value64f);
				}
				else
				{
					dlt_log(LOG_ERR, "Invalid size of float64_t\n");
					return -1;
				}
				break;
			}
			case DLT_TYLE_128BIT:
			{
				if (*datalength>=16)
					dlt_print_hex_string(text+strlen(text),textlength,*ptr,16);
                if((*datalength)<16)
                    return -1;
                *ptr += 16;
				*datalength-=16;
				break;
			}
			default:
			{
				return -1;
			}
        }

    }
    else if (type_info & DLT_TYPE_INFO_RAWD)
    {
        /* raw data argument */
        DLT_MSG_READ_VALUE(length_tmp,*ptr,*datalength,uint16_t);
        if((*datalength)<0)
            return -1;
        length=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length_tmp);
        if (type_info & DLT_TYPE_INFO_VARI)
        {
            DLT_MSG_READ_VALUE(length2_tmp,*ptr,*datalength,uint16_t);
            if((*datalength)<0)
                return -1;
            length2=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length2_tmp);
            if((*datalength)<length2)
                return -1;
            *ptr += length2;
            *datalength-=length2;
        }
        if((*datalength)<length)
            return -1;
        dlt_print_hex_string(text+strlen(text),textlength,*ptr,length);
        *ptr+=length;
        *datalength-=length;
    }
    else if (type_info & DLT_TYPE_INFO_TRAI)
    {
        /* trace info argument */
        DLT_MSG_READ_VALUE(length_tmp,*ptr,*datalength,uint16_t);
        if((*datalength)<0)
            return -1;
        length=DLT_ENDIAN_GET_16(msg->standardheader->htyp, length_tmp);
        DLT_MSG_READ_STRING((text+strlen(text)),*ptr,*datalength,length);
        if((*datalength)<0)
            return -1;
    }
    else
    {
        return -1;
    }

    if (*datalength<0)
    {
        dlt_log(LOG_ERR, "Payload of DLT message corrupted\n");
        return -1;
    }

    return 0;
}

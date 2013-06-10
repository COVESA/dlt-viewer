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
 * \file qdlt.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtDebug>

#include "qdlt.h"

extern "C"
{
#include "dlt_common.h"
}

const char *qDltMessageType[] = {"log","app_trace","nw_trace","control","","","",""};
const char *qDltLogInfo[] = {"","fatal","error","warn","info","debug","verbose","","","","","","","","",""};
const char *qDltTraceType[] = {"","variable","func_in","func_out","state","vfb","","","","","","","","","",""};
const char *qDltNwTraceType[] = {"","ipc","can","flexray","most","vfb","","","","","","","","","",""};
const char *qDltControlType[] = {"","request","response","time","","","","","","","","","","","",""};
const char *qDltMode[] = {"non-verbose","verbose"};
const char *qDltEndianness[] = {"little-endian","big-endian"};
const char *qDltTypeInfo[] = {"String","Bool","SignedInteger","UnsignedInteger","Float","RawData","TraceInfo"};
const char *qDltCtrlServiceId[] = {"","set_log_level","set_trace_status","get_log_info","get_default_log_level","store_config","reset_to_factory_default",
                             "set_com_interface_status","set_com_interface_max_bandwidth","set_verbose_mode","set_message_filtering","set_timing_packets",
                             "get_local_time","use_ecu_id","use_session_id","use_timestamp","use_extended_header","set_default_log_level","set_default_trace_status",
                             "get_software_version","message_buffer_overflow"};
const char *qDltCtrlReturnType [] = {"ok","not_supported","error","3","4","5","6","7","no_matching_context_id"};

QDlt::QDlt()
{

}

QDlt::~QDlt()
{

}

bool QDlt::swap(QByteArray &bytes,int size, int offset)
{
    char tmp;

    if( (offset < 0)  || (offset >= bytes.size()) )
        return false;

    if(size == -1)
        size = bytes.size()-offset;

    if((size+offset) > bytes.size())
        return false;

    for(int num = 0;num<(size/2);num++)
    {
        tmp = bytes[offset+num];
        bytes[offset+num] = bytes[offset+size-1-num];
        bytes[offset+size-1-num] = tmp;
    }

    return true;
}

QString QDlt::toAsciiTable(QByteArray &bytes, bool withLineNumber, bool withBinary, bool withAscii, int blocksize, int linesize, bool toHtml)
{
    QString text;
    text.reserve(1024+bytes.size());

    /* create HTML text to show */
    if(toHtml)
        text+=QString("<html><body>");

    if(toHtml)
        text += QString("<pre>");

    int lines = (bytes.size()+linesize-1)/linesize;
    for(int line=0;line<lines;line++)
    {
        if(withLineNumber)
            text += QString("%1: ").arg(line*linesize,4,linesize,QLatin1Char('0'));
        if(withBinary)
        {
            for(int num=0;num<linesize;num++)
            {
                int bufpos = line*linesize+num;
                if(bufpos >= bytes.size())
                    break;
                char ch = (bytes.constData())[bufpos];
                if(num==blocksize)
                    text += QString("  ");
                else if(num!=0)
                    text += QString(" ");
                if((line*linesize+num) < bytes.size())
                    text += QString("%1").arg((unsigned char)ch,2,linesize,QLatin1Char('0'));
                else
                    text += QString("--");
            }
        }
        if(withAscii)
        {
            text += QString(" ");
            for(int num=0;num<linesize;num++)
            {
                int bufpos = line*linesize+num;
                if(bufpos >= bytes.size())
                    break;
                char ch = (bytes.constData())[bufpos];
                if((ch >= ' ') && (ch <= '~') )
                {
                    // Necessary to display < and > as characters in a HTML context.
                    // Otherwise < and > would be handled as HTML tags and not the complete payload would be displayed.
                    if(toHtml && ch == '<')
                    {
                        text += "&lt;";
                    }else if(toHtml && ch == '>'){
                        text += "&gt;";
                    }else{
                        text += QString(QChar(ch));
                    }
                }
                else
                {
                    text += QString("-");
                }
            }
        }
        if(line!=lines-1)
        {
            if(toHtml)
                text += QString("<BR>");
            else
                text += QString("\n");
        }
    }
    if(toHtml)
        text += QString("</pre>");

    /* finish HTML text */
    if(toHtml)
        text+=QString("</body></html>");

    return text;
}

QString QDlt::toAscii(QByteArray &bytes, bool ascii)
{
    static const char hexmap[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    if (ascii)
    {
        // Could also use QString::QString ( const QByteArray & ba )
        return QString::fromLatin1(bytes.data(), bytes.size() );
    }
    else
    {
        int size = bytes.size();
        if (!size)
        {
            //Let's return an empty string.
            return QString("");
        }

        std::vector<char> str( size*3, ' ' );

        char* strData = &str[0];
        char* byteData = bytes.data();
        for(int num=0;num<size;++num)
        {
            *strData = hexmap[ (*byteData & 0xF0) >> 4 ];
            ++strData;
            *strData = hexmap[ *byteData & 0x0F ];
            strData += 2;
            ++byteData;
        }
        // Remove trailing space at the end
        if (size>0)
        {
            *(strData-1) = 0;
        }
        return QString( &str[0] );
    }
}


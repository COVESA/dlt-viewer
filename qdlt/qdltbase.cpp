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
 * \file qdlt.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "qdltbase.h"

#include <vector>

QString QDlt::toAsciiTable(const QByteArray &bytes, bool withLineNumber, bool withBinary, bool withAscii, int blocksize, int linesize, bool toHtml)
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

QString QDlt::toAscii(const QByteArray &bytes, int type,int size_bytes)
{
    if (type==1)
    {
        // ascii
        // Could also use QString::QString ( const QByteArray & ba )
        return QString::fromLatin1(bytes.data(), bytes.size() );
    }
    else if (type==2)
    {
        // binary
        int size = bytes.size();
        if (!size)
        {
            //Let's return an empty string.
            return QString("");
        }

        if (1 == size_bytes)
        {
            uint8_t toEncode = bytes.data()[0];
            QString encoded = QString("0b%1").arg(toEncode, 8, 2, QChar('0'));
            return encoded.insert(6, ' '); // insert a space after 0bxxxx
        }
        else // has to be: (2 == size_bytes)
        {
            uint16_t toEncode = (uint8_t)(bytes.data()[0]) | (((uint8_t)(bytes.data()[1])) << 8);
            QString encoded = QString("0b%1").arg(toEncode, 16, 2, QChar('0'));
            encoded.insert(14, ' '); // insert spaces
            encoded.insert(10, ' ');
            encoded.insert(6, ' ');
            return encoded;
        }

        return QString("");
    }
    else
    {
        // hex
        int size = bytes.size();
        if (!size)
        {
            //Let's return an empty string.
            return QString("");
        }

        if (1 == size_bytes)
        {
            uint8_t toEncode = bytes.data()[0];
            return QString("0x%1").arg(toEncode, 2, 16, QChar('0'));
        }
        if (2 == size_bytes)
        {
            uint16_t toEncode = (uint8_t)(bytes.data()[0]) | (((uint8_t)(bytes.data()[1])) << 8);
            return QString("0x%1").arg(toEncode, 4, 16, QChar('0'));
        }
        if (4 == size_bytes)
        {
            uint32_t toEncode = (uint8_t)(bytes.data()[0]) | (((uint8_t)(bytes.data()[1])) << 8) | (((uint8_t)(bytes.data()[2])) << 16) | (((uint8_t)(bytes.data()[3])) << 24);
            return QString("0x%1").arg(toEncode, 8, 16, QChar('0'));
        }
        if (8 == size_bytes)
        {
            uint32_t toEncodeLo = (uint8_t)(bytes.data()[0]) | (((uint8_t)(bytes.data()[1])) << 8) | (((uint8_t)(bytes.data()[2])) << 16) | (((uint8_t)(bytes.data()[3])) << 24);
            uint32_t toEncodeHi = (uint8_t)(bytes.data()[4]) | (((uint8_t)(bytes.data()[5])) << 8) | (((uint8_t)(bytes.data()[6])) << 16) | (((uint8_t)(bytes.data()[7])) << 24);
            return QString("0x%1").arg(toEncodeHi, 8, 16, QChar('0')) + QString("%1").arg(toEncodeLo, 8, 16, QChar('0'));
        }

        if (0xff == size_bytes)
        {
            std::vector<char> str( size*3, ' ' );

            char* strData = &str[0];
            const char* byteData = bytes.data();
            static const char hexmap[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
            for(int num=0;num<size;++num)
            {
                *strData = hexmap[ (*byteData & 0xF0) >> 4 ];
                ++strData;
                *strData = hexmap[ *byteData & 0x0F ];
                if((((num+1)%1)==0) && (num!=size-1))
                    strData += 2;
                else
                    strData += 1;
                ++byteData;
            }
            // add termination
            if (size>0)
            {
                *(strData) = 0;
            }
            return QString( &str[0] );
        }
    }
    return QString("");
}


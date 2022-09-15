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
 * \file qdltparser.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "qdltparser.h"

#include <QByteArray>
#include <QDebug>
#include <QStringList>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QCryptographicHash>

QDltParser::QDltParser()
{
    clear();
}

void QDltParser::clear()
{
    applications.clear();
    contexts.clear();
    messages.clear();
    messageIds.clear();
}

bool QDltParser::parseFile(QString fileName)
{
    QByteArray data;
    QString text;
    QFile file(fileName);
    int linecounter = 0;

    qDebug() << "parseFile" << fileName;

    // open file
    if(!file.open(QFile::ReadOnly))
    {
        errorString = "Cannot open file " + fileName;
        return false;
    }

    // run through whole file
    while(file.bytesAvailable())
    {
        data = file.readLine();
        linecounter++;
        text = QString(data);
        if(text.contains("DLT_REGISTER_APP"))
        {
            qDebug() << "parseFile" << "DLT_REGISTER_APP";
            if(!parseContextsRegisterApp(text))
            {
                return false;
            }
        }
        else if(text.contains("DLT_REGISTER_CONTEXT"))
        {
            qDebug() << "parseFile" << "DLT_REGISTER_CONTEXT";
            if(!parseContextsRegisterContext(text))
            {
                return false;
            }
        }
        else if(text.contains("#define") && text.contains("DLT_MSG_ID_"))
        {
            qDebug() << "parseFile" << "#define DLT_MSG_ID_";
            if(!parseMessageId(text))
            {
                return false;
            }
        }
        else if(text.contains("DLT_LOG_ID") || (text.contains("DLT_LOG")))
        {
            QFile empty;
            if(!parseMessage(file,empty,linecounter,text))
            {
                return false;
            }
        }
    }

    // close file
    file.close();

    // everything is ok
    return true;
}

bool QDltParser::converteFile(QString fileName)
{
    QByteArray data;
    QString text;
    QFile file(fileName);
    QFile fileWrite(fileName);
    int linecounter = 0;

    qDebug() << "converteFile" << fileName;

    QFile::remove(fileName+QString(".bak_dlt"));
    file.rename(fileName+QString(".bak_dlt"));

    // open write file
    if(!fileWrite.open(QFile::WriteOnly))
    {
        errorString = "Cannot open file " + fileName;
        return false;
    }

    // open read file
    if(!file.open(QFile::ReadOnly))
    {
        errorString = "Cannot open file " + fileName;
        return false;
    }

    // run through whole file
    while(file.bytesAvailable())
    {
        data = file.readLine();
        linecounter++;
        text = QString(data);
        if(text.contains("DLT_LOG_ID") || text.contains("DLT_LOG"))
        {
            if(!parseMessage(file,fileWrite,linecounter,text))
            {
                return false;
            }
        }
        else
        {
            /* write output file */
            fileWrite.write(text.toLatin1());
        }
    }

    // close file
    file.close();
    fileWrite.close();

    // everything is ok
    return true;
}

bool QDltParser::parseMessage(QFile &file,QFile &fileWrite,int &linecounter,QString text)
{
    bool withid = false;
    QDltFibexFrame *frame = new QDltFibexFrame();
    QStringList list;

    if(text.contains("DLT_LOG_ID"))
    {
        withid = true;
    }

    frame->filename = file.fileName();
    frame->lineNumber = linecounter;

    list = parseMessageLine(file,text,linecounter);

    if(fileWrite.isOpen() && withid)
    {
        /* write output file */
        fileWrite.write(text.toLatin1());
    }

    if(fileWrite.isOpen() && !withid)
    {
        /* write output file */
        fileWrite.write(QString("DLT_LOG_ID%1(").arg(withid? list.size()-3 : list.size()-2).toLatin1());
    }

    parseMessageStringList(frame,fileWrite,list,withid);

    if(!frame || frame->context.isEmpty() || frame->messageInfo.isEmpty())
    {
        errorString = "Missing parameters in message: " + text;

        if(frame)
            delete frame;

        return false;
    }

    if(fileWrite.isOpen() && !withid)
    {
        if(list.size()==2)
        {
            QString hashString = fileWrite.fileName()+"_"+QString("%1").arg(frame->lineNumber)+"_"+frame->context;
            QByteArray hashByteArray = hashString.toLatin1();
            QByteArray md5 = QCryptographicHash::hash(hashByteArray, QCryptographicHash::Md5);

            frame->idString = QString("DLT_MSG_ID_%1_%2_%3").arg(frame->appid).arg(frame->ctid).arg(QString(md5.toHex()).left(8));
            fileWrite.write(QString("%1);\n").arg(frame->idString).toLatin1());
        }
        /* write output lines to do not change line number order */
        for(int i=0;i<(linecounter-frame->lineNumber);i++)
        {
            fileWrite.write(QString("\n").toLatin1());
        }
    }

    // only if not coverting
    if(!fileWrite.isOpen())
        messages.append(frame);

    qDebug() << "parseMessage" << frame->filename << frame->lineNumber;

    return true;
}

void QDltParser::parseMessageStringList(QDltFibexFrame *frame,QFile &fileWrite,QStringList &list,bool withid)
{
    for(int i = 0; i < list.size();i++)
    {
        QString argtext = list.at(i);
        argtext = argtext.trimmed();

        qDebug() << "parseMessageStringList" << argtext;

        if(i==0)
        {
            frame->context = argtext;

            QDltCon *con = contexts[frame->context];
            if(con)
            {
                frame->ctid = con->conid;
                frame->appid = con->appid;
            }

            if(fileWrite.isOpen() && !withid)
            {
                /* write output file */
                fileWrite.write(QString("%1,").arg(list.at(i)).toLatin1());
            }
        }
        else if(i==1)
        {
            /* message info */
            frame->messageInfo = argtext;

            if(fileWrite.isOpen() && !withid)
            {
                /* write output file */
                fileWrite.write(QString("%1,").arg(list.at(i)).toLatin1());
            }
         }
        else if(withid && i==2)
        {
            frame->idString = argtext;
            frame->id = argtext.toUInt();
        }
        else
        {
            /* argument */
            parseMessageParameter(argtext,frame);

            if(fileWrite.isOpen() && !withid)
            {
                if(i==2)
                {
                    QString hashString = fileWrite.fileName()+"_"+QString("%1").arg(frame->lineNumber)+"_"+frame->context;
                    QByteArray hashByteArray = hashString.toLatin1();
                    QByteArray md5 = QCryptographicHash::hash(hashByteArray, QCryptographicHash::Md5);

                    frame->idString = QString("DLT_MSG_ID_%1_%2_%3").arg(frame->appid).arg(frame->ctid).arg(QString(md5.toHex()).left(8));

                    /* write output file */
                    fileWrite.write(QString("%1,").arg(frame->idString).toLatin1());
                }
                if(i < (list.size()-1))
                {
                    fileWrite.write(QString("%1,").arg(argtext).toLatin1());
                }
                else
                {
                    fileWrite.write(QString("%1);\n").arg(argtext).toLatin1());
                }
            }

        }
    }
}

void QDltParser::parseMessageParameter(QString argtext,QDltFibexFrame *frame)
{
    int begin,end;
    QDltFibexPdu *pdu;

    pdu=new QDltFibexPdu();
    frame->pdureflist.append(pdu);

    if(argtext.contains("DLT_CSTRING"))
    {
        begin = argtext.indexOf('"');
        end = argtext.lastIndexOf('"');
        pdu->description = argtext.mid(begin+1,end-begin-1);
    }
    else if(argtext.contains("DLT_STRING"))
    {
          pdu->typeInfo = "S_STRG_ASCII";
    }
    else if(argtext.contains("DLT_RAW"))
    {
          pdu->typeInfo = "S_RAWD";
          pdu->byteLength = 0;
    }
    else
    {
        begin = argtext.indexOf('(');
        pdu->typeInfo = QString("S_") + argtext.mid(4,begin-4);
        if (pdu->typeInfo == "S_BOOL")
        {
            pdu->byteLength = 1;
        }
        else if (pdu->typeInfo == "S_INT8")
        {
            pdu->byteLength = 1;
            pdu->typeInfo = "S_SINT8";
        }
        else if (pdu->typeInfo == "S_UINT8")
        {
            pdu->byteLength = 1;
        }
        else if (pdu->typeInfo == "S_INT16")
        {
            pdu->byteLength = 2;
            pdu->typeInfo = "S_SINT16";
        }
        else if (pdu->typeInfo == "S_UINT16")
        {
            pdu->byteLength = 2;
        }
        else if (pdu->typeInfo == "S_INT32" || pdu->typeInfo == "S_INT")
        {
            pdu->byteLength = 4;
            pdu->typeInfo = "S_SINT32";
        }
        else if (pdu->typeInfo == "S_UINT32" || pdu->typeInfo == "S_UINT")
        {
            pdu->byteLength = 4;
            pdu->typeInfo = "S_UINT32";
        }
        else if (pdu->typeInfo == "S_INT64")
        {
            pdu->byteLength = 8;
            pdu->typeInfo = "S_SINT64";
        }
        else if (pdu->typeInfo == "S_UINT64")
        {
            pdu->byteLength = 8;
        }
        else if (pdu->typeInfo == "S_FLOAT16")
        {
            pdu->byteLength = 2;
            pdu->typeInfo = "S_FLOA16";
        }
        else if (pdu->typeInfo == "S_FLOAT32")
        {
            pdu->byteLength = 4;
            pdu->typeInfo = "S_FLOA32";
        }
        else if (pdu->typeInfo == "S_FLOAT64")
        {
            pdu->byteLength = 8;
            pdu->typeInfo = "S_FLOA64";
        }
        else if (pdu->typeInfo == "S_STRG_ASCII")
        {
            pdu->byteLength = 0;
        }
    }

    qDebug() << "parseMessageParameter" <<  pdu->typeInfo << pdu->byteLength << pdu->description;
}

QStringList QDltParser::parseMessageLine(QFile &file,QString &line,int &linecounter)
{
    QStringList list;
    int level = 0;
    int pos = 0;
    int start = 0;
    QString text;
    QByteArray data;
    QChar character;
    bool quotation = false; /* always set when inside of a string */
    bool backslash = false; /* is set to true, if the char before was a backslash */

    do
    {
        for(;pos<line.length();pos++)
        {
            character = line.at(pos);
            if(!backslash && character == '"')
            {
                if(quotation)
                    quotation = false;
                else
                    quotation = true;
                backslash = false;
            }
            else if(quotation && character == '\\')
            {
                backslash = true;
            }
            else if(!quotation && character == '(')
            {
                if(level == 0)
                {
                    start = pos+1;
                }
                level++;
                backslash = false;
            }
            else if(!quotation && character == ',')
            {
                if(level==1)
                {
                    text = line.mid(start,pos-start);
                    list.append(text);
                    start = pos+1;
                }
                backslash = false;
            }
            else if(!quotation && character == ')')
            {
                if(level==1)
                {
                    text = line.mid(start,pos-start);
                    list.append(text);
                }
                level--;
                backslash = false;
            }
            else
            {
                backslash = false;
            }
        }

        if(level>0)
        {
            data = file.readLine();
            linecounter++;
            line += QString(data);
        }

    } while(level>0);

    return list;
}

bool  QDltParser::parseMessageId(QString text)
{
    QStringList list;

    list = text.split(" ",QString::SkipEmptyParts);

    for (int i = 0; i < list.size(); ++i)
    {
        if((list[i] == QString("#define")) && (list.size() > (i+2)))
        {
            messageIds[list[i+1]]=list[i+2].toUInt();

            qDebug() << "parseMessageId" << list[i+1] << list[i+2];

            return true;
        }
        else
        {
            errorString = "Define Message Id wrong format: " + text;
            return false;
        }
    }

    errorString = "Define Message Id wrong format: " + text;
    return false;
}

bool QDltParser::parseContextsRegisterApp(QString text)
{
    int begin,end;
    QStringList list;
    QString argtext;
    QString appid,appdesc;

    begin = text.indexOf('(');
    end = text.lastIndexOf(')');
    if(begin>0 && end>0 && end>begin)
    {
        text.remove(end,text.size()-end+1);
        text.remove(0,begin+1);
        text = text.trimmed();

        list = text.split(',');
        for(int i = 0; i < list.size();i++)
        {
            argtext = list.at(i);
            argtext = argtext.trimmed();

            if(i == 0)
            {
                /* app id */
                begin = argtext.indexOf('"');
                end = argtext.lastIndexOf('"');
                appid = argtext.mid(begin+1,end-begin-1);
            }
            else if (i == 1)
            {
                /* app description */
                begin = argtext.indexOf('"');
                end = argtext.lastIndexOf('"');
                appdesc = argtext.mid(begin+1,end-begin-1);
            }
        }
    }

    if(appid.isEmpty())
    {
        errorString = "Application Id is empty: " + text;
        return false;
    }

    qDebug() << "parseContextsRegisterApp" << appid << appdesc;

    applications[appid] = appdesc;

    return true;
}

bool QDltParser::parseContextsRegisterContext(QString text)
{
    int begin,end;
    QDltCon *con = 0;
    QStringList list;
    int i;
    QString argtext;
    bool withapp = false;

    if(text.contains("DLT_REGISTER_CONTEXT_APP"))
    {
        withapp = true;
    }

    begin = text.indexOf('(');
    end = text.lastIndexOf(')');
    if(begin>0 && end>0 && end>begin)
    {
        con = new QDltCon();

        text.remove(end,text.size()-end+1);
        text.remove(0,begin+1);
        text = text.trimmed();

        list = text.split(',');
        for(i = 0; i < list.size();i++)
        {
            argtext = list.at(i);
            argtext = argtext.trimmed();

            /* DLT Embedded mode */
            if(i == 0)
            {
                /* context */
                con->context = argtext;
            }
            else if (i == 1)
            {
                /* con id */
                begin = argtext.indexOf('"');
                end = argtext.lastIndexOf('"');
                con->conid = argtext.mid(begin+1,end-begin-1);
            }
            else if (withapp && i == 2)
            {
                /* app id */
                begin = argtext.indexOf('"');
                end = argtext.lastIndexOf('"');
                con->appid = argtext.mid(begin+1,end-begin-1);
            }
            else if ((!withapp && i == 2) || (withapp && i == 3))
            {
                /* con description */
                begin = argtext.indexOf('"');
                end = argtext.lastIndexOf('"');
                con->description = argtext.mid(begin+1,end-begin-1);
            }
        }
    }

    if(!con || con->conid.isEmpty())
    {
        errorString = "Context Id is empty: " + text;

        delete con;

        return false;
    }

    qDebug() << "parseContextsRegisterContext" << con->context << con->appid << con->conid << con->description;

    contexts[con->context] = con;

    return true;
}

bool QDltParser::parseCheck()
{
    QDltFibexFrame *frame;

    // check application ids
    if(applications.size()<1)
    {
        errorString = "No application is registered!";
        return false;
    }

    // find an application id
    QString appid;
    QMapIterator<QString,QString> i(applications);
    while (i.hasNext()) {
        i.next();
        appid = i.key();
    }

    // check and set context ids and application ids in frames
    for(int i = 0;i<messages.size();i++)
    {
        frame = messages.at(i);

        // search context
        QDltCon *con = contexts[frame->context];
        if(!con)
        {
            errorString = "Context not found: " + frame->context;
            return false;
        }
        if(con->conid.isEmpty())
        {
            errorString = "Context " + frame->context + " has no context id!";
            return false;
        }
        frame->ctid = con->conid;
        if(con->appid.isEmpty())
        {
            if(applications.size()>1)
            {
                errorString = "Context " + frame->context + " has no application id and more than one application is registered!";
                return false;
            }
            con->appid = appid;
            frame->appid = appid;  // when common api is not used, the register context macro does not contain the appid; use global one
        }
        else
        {
            frame->appid = con->appid;
        }

        // search id
        uint32_t id = messageIds[frame->idString];
        if(id!=0)
        {
            frame->id = id;
        }

    }

    return true;
}

bool QDltParser::writeFibex(QString &fileName)
{
    QFile file;

    qDebug() << "writeFibex" << fileName;

    /* open selected file */
    file.setFileName(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
    {
        /* file could not be opened */
        errorString = "Cannot open file: " + fileName;
        return false;
    }

    /* open xml writer */
    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);

    /* write start XML document */
    xml.writeStartDocument();

    xml.writeNamespace("http://www.asam.net/xml","ho");
    xml.writeNamespace("http://www.asam.net/xml/fbx","fx");

    xml.writeStartElement("http://www.asam.net/xml/fbx","FIBEX");

    xml.writeStartElement("http://www.asam.net/xml/fbx","PROJECT");
    xml.writeAttribute("ID","projectEntry");
    xml.writeTextElement("http://www.asam.net/xml","SHORT-NAME","projectEntry");
    xml.writeEndElement(); // PROJECT

    xml.writeStartElement("http://www.asam.net/xml/fbx","ELEMENTS");
    xml.writeStartElement("http://www.asam.net/xml/fbx","ECUS");
    xml.writeStartElement("http://www.asam.net/xml/fbx","ECU");
    xml.writeAttribute("ID","Entry");
    xml.writeTextElement("http://www.asam.net/xml","SHORT-NAME","Entry");

    xml.writeStartElement("http://www.asam.net/xml/fbx","MANUFACTURER-EXTENSION");

    xml.writeTextElement("SW_VERSION","unknown");

    /* Applications */
    xml.writeStartElement("APPLICATIONS");

    QMapIterator<QString,QString> i(applications);
    while (i.hasNext()) {
        i.next();

        /* Application */
        xml.writeStartElement("APPLICATION");
        xml.writeTextElement("APPLICATION_ID",i.key());
        xml.writeTextElement("APPLICATION_DESCRIPTION",i.value());

        /* Contexts */
        xml.writeStartElement("CONTEXTS");
        QMapIterator<QString,QDltCon*> j(contexts);
        while (j.hasNext()) {
            j.next();
            QDltCon* con = j.value();

            if(con->appid == i.key())
            {
                xml.writeStartElement("CONTEXT");
                xml.writeTextElement("CONTEXT_ID",con->conid);
                xml.writeTextElement("CONTEXT_DESCRIPTION",con->description);
                xml.writeEndElement(); // CONTEXT
            }
        }
        xml.writeEndElement(); // CONTEXTS

        xml.writeEndElement(); // APPLICATION
    }

    xml.writeEndElement(); // APPLICATIONS

    xml.writeEndElement(); // MANUFACTURER-EXTENSION
    xml.writeEndElement(); // ECU
    xml.writeEndElement(); // ECUS

    xml.writeStartElement("http://www.asam.net/xml/fbx","PDUS");

    for(int i = 0;i<messages.size();i++)
    {
        QDltFibexFrame *frame = messages.at(i);

        int length = 0;
        for(int k = 0;k<frame->pdureflist.size();k++)
        {
            QDltFibexPdu *pduc= frame->pdureflist.at(k);
            if(!pduc->description.isEmpty())
                length += 0;
            else
                length += pduc->byteLength;
         }

        for(int j = 0;j<frame->pdureflist.size();j++)
        {

            QDltFibexPdu *pdu= frame->pdureflist.at(j);

            xml.writeStartElement("http://www.asam.net/xml/fbx","PDU");
            xml.writeAttribute("ID",QString("PDU_%1_%2").arg(frame->id).arg(j));
            xml.writeTextElement("http://www.asam.net/xml","SHORT-NAME",QString("PDU_%1_%2").arg(frame->id).arg(j));
            if(pdu->description.isEmpty())
            {
                /* all parameters except constant strings */
                xml.writeTextElement("http://www.asam.net/xml/fbx","BYTE-LENGTH",QString("%1").arg(pdu->byteLength));
                xml.writeTextElement("http://www.asam.net/xml/fbx","PDU-TYPE","OTHER");
                xml.writeStartElement("http://www.asam.net/xml/fbx","SIGNAL-INSTANCES");
                xml.writeStartElement("http://www.asam.net/xml/fbx","SIGNAL-INSTANCE");
                xml.writeAttribute("ID",QString("S_%1_0").arg(frame->id));
                xml.writeTextElement("http://www.asam.net/xml/fbx","SEQUENCE-NUMBER","0");
                xml.writeStartElement("http://www.asam.net/xml/fbx","SIGNAL-REF");
                xml.writeAttribute("ID-REF",pdu->typeInfo);
                xml.writeEndElement(); // SIGNAL-REF
                xml.writeEndElement(); // SIGNAL-INSTANCE
                xml.writeEndElement(); // SIGNAL-INSTANCES
            }
            else
            {
                /* constant string only */
                xml.writeTextElement("http://www.asam.net/xml","DESC",pdu->description);
                xml.writeTextElement("http://www.asam.net/xml/fbx","BYTE-LENGTH","0");
                xml.writeTextElement("http://www.asam.net/xml/fbx","PDU-TYPE","OTHER");
            }
            xml.writeEndElement(); // PDU
        }
    }

    xml.writeEndElement(); // PDUS

    xml.writeStartElement("http://www.asam.net/xml/fbx","FRAMES");

    for(int i = 0;i<messages.size();i++)
    {
        QDltFibexFrame *frame = messages.at(i);

        xml.writeStartElement("http://www.asam.net/xml/fbx","FRAME");
        xml.writeAttribute("ID",QString("ID_%1").arg(frame->id));
        xml.writeTextElement("http://www.asam.net/xml","SHORT-NAME",QString("ID_%1").arg(frame->id));

        int length = 0;
        for(int k = 0;k<frame->pdureflist.size();k++)
        {
            QDltFibexPdu *pduc= frame->pdureflist.at(k);
            if(!pduc->description.isEmpty())
                length += 0;
            else
                length += pduc->byteLength;
        }

        xml.writeTextElement("http://www.asam.net/xml/fbx","BYTE-LENGTH",QString("%1").arg(length));

        xml.writeTextElement("http://www.asam.net/xml/fbx","FRAME-TYPE","OTHER");

        xml.writeStartElement("http://www.asam.net/xml/fbx","PDU-INSTANCES");

        for(int j = 0;j<frame->pdureflist.size();j++)
        {

            //DltFibexPdu *pdu= frame->pdureflist.at(j);

            xml.writeStartElement("http://www.asam.net/xml/fbx","PDU-INSTANCE");
            xml.writeAttribute("ID",QString("P_%1_%2").arg(frame->id).arg(j));
            xml.writeStartElement("http://www.asam.net/xml/fbx","PDU-REF");
            xml.writeAttribute("ID-REF",QString("PDU_%1_%2").arg(frame->id).arg(j));
            xml.writeEndElement(); // PDU-REF
            xml.writeTextElement("http://www.asam.net/xml/fbx","SEQUENCE-NUMBER",QString("%1").arg(j));
            xml.writeEndElement(); // PDU-INSTANCE
        }

        xml.writeEndElement(); // PDU-INSTANCES

        xml.writeStartElement("http://www.asam.net/xml/fbx","MANUFACTURER-EXTENSION");
        xml.writeTextElement("MESSAGE_TYPE",QString("DLT_TYPE_LOG"));
        xml.writeTextElement("MESSAGE_INFO",frame->messageInfo);
        xml.writeTextElement("APPLICATION_ID",frame->appid);
        xml.writeTextElement("CONTEXT_ID",frame->ctid);
        xml.writeTextElement("MESSAGE_SOURCE_FILE",QString("%1").arg(frame->filename));
        xml.writeTextElement("MESSAGE_LINE_NUMBER",QString("%1").arg(frame->lineNumber));
        xml.writeEndElement(); // MANUFACTURER-EXTENSION

        xml.writeEndElement(); // FRAME
    }

    xml.writeEndElement(); // FRAMES
    xml.writeEndElement(); // ELEMENTS
    xml.writeEndElement(); // FIBEX

    /* write end XML document */
    xml.writeEndDocument();

    /* close CML file */
    file.close();

    return true;
}

bool QDltParser::writeCsv(QString &fileName)
{
    QFile file(fileName);

    qDebug() << "writeCsv" << fileName;

    /* open selected file */
    if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
    {
        /* file could not be opened */
        errorString = "Cannot open file: " + fileName;
        return false;
    }

    //file.write("ID;BYTE-LENGTH;MESSAGE_TYPE;MESSAGE_INFO;APPLICATION_ID;CONTEXT_ID;FILENAME;LINENUMBER;ARG0;LENGTH0;ARG1;LENGTH1;ARG2;LENGTH2");

    for(int i = 0;i<messages.size();i++)
    {
        QDltFibexFrame *frame = messages.at(i);

        /* write message ID */
        file.write(QString("\"%1\",").arg(frame->id).toLatin1());

        int length = 0;
        for(int k = 0;k<frame->pdureflist.size();k++)
        {
            QDltFibexPdu *pduc= frame->pdureflist.at(k);
            if(!pduc->description.isEmpty())
                length += 0;
            else
                length += pduc->byteLength;
         }

        /* write length in bytes */
        file.write(QString("\"%1\",").arg(length).toLatin1());

        /* write message type */
        file.write(QString("\"%1\",").arg("DLT_TYPE_LOG").toLatin1());

        /* write message info */
        file.write(QString("\"%1\",").arg(frame->messageInfo).toLatin1());
        /* write app id */
        file.write(QString("\"%1\",").arg(frame->appid).toLatin1());
        /* write context id */
        file.write(QString("\"%1\",").arg(frame->ctid).toLatin1());
        /* write file name */
        file.write(QString("\"%1\",").arg(frame->filename).toLatin1());
        /* write line number */
        file.write(QString("\"%1\"").arg(frame->lineNumber).toLatin1());

        for(int j = 0;j<frame->pdureflist.size();j++)
        {

            QDltFibexPdu *pdu= frame->pdureflist.at(j);

            if(!pdu->description.isEmpty())
            {
                /* write message argument */
                file.write(QString(",\"%1\"").arg(pdu->description).toLatin1());
                file.write(QString(",\"%1\"").arg(0).toLatin1());
            }
            else
            {
                /* write message argument */
                file.write(QString(",\"%1\"").arg(pdu->typeInfo).toLatin1());
                file.write(QString(",\"%1\"").arg(pdu->byteLength).toLatin1());
            }

        }
        file.write("\n");
    }

    /* close CML file */
    file.close();

    return true;
}

bool QDltParser::writeIdHeader(QString &directory,bool perApplication)
{
    QFile file;

    QMapIterator<QString,QString> i(applications);

    if(!perApplication)
    {
        file.setFileName(directory+QString("/dlt_id.h"));

        qDebug() << "writeIdHeader" << directory+QString("/dlt_id.h");

        /* open selected file */
        if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
        {
            /* file could not be opened */
            errorString = "Cannot open file: " + directory+QString("/dlt_id.h");
            return false;
        }

        file.write("// generated file, do not edit\n");
        file.write("\n");
        file.write(QString("#ifndef DLT_ID_H\n").toLatin1());
        file.write(QString("#define DLT_ID_H\n").toLatin1());
        file.write("\n");
    }

    while(i.hasNext())
    {
        i.next();

        if(perApplication)
        {
            file.setFileName(directory+QString("/dlt_id_%1.h").arg(i.key()));

            qDebug() << "writeIdHeader" << directory+QString("/dlt_id_%1.h").arg(i.key());

            /* open selected file */
            if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
            {
                /* file could not be opened */
                errorString = QString("Cannot open file: ") + directory+QString("/dlt_id_%1.h").arg(i.key());
                return false;
            }

            file.write("// generated file, do not edit");
            file.write("\n");
            file.write(QString("#ifndef DLT_ID_%1_H\n").arg(i.key()).toLatin1());
            file.write(QString("#define DLT_ID_%1_H\n").arg(i.key()).toLatin1());
            file.write("\n");
        }

        /* add all DLT messages to list */
        for(int j = 0;j<messages.size();j++)
        {
            QDltFibexFrame *frame = messages.at(j);

            if(frame->appid==i.key())
                file.write(QString("#define %1 %2\n").arg(frame->idString).arg(frame->id).toLatin1());
        }

        if(perApplication)
        {
            file.write("\n");
            file.write(QString("#endif /* DLT_ID_%1_H */\n").arg(i.key()).toLatin1());

            /* close CML file */
            file.close();
        }
    }

    if(!perApplication)
    {
        file.write("\n");
        file.write(QString("#endif /* DLT_ID_H */\n").toLatin1());

        /* close CML file */
        file.close();
    }

    return true;
}

bool QDltParser::checkId(uint32_t startId,uint32_t endId,uint32_t &id, bool perApplication,QString appId)
{
    bool ok = true;

    if(id<startId || id>endId)
        ok = false;
    else
    {
        int found = 0;

        // if found more than once set false
        for(int j = 0;j<messages.size();j++)
        {
            QDltFibexFrame *frame = messages.at(j);

            if(!perApplication || appId==frame->appid)
            {
                if(id==frame->id)
                    found++;
            }
        }
        if(found>1)
            ok = false;
    }

    if(!ok)
    {
        // find new free message id
        bool found = false;
        id = startId;
        do
        {
            if(found)
            {
                id++;
                found = false;
            }

            if(id>endId)
                return false;

            // generate new id
            for(int j = 0;j<messages.size();j++)
            {
                QDltFibexFrame *frame = messages.at(j);

                if(!perApplication || appId==frame->appid)
                {
                    if(id==frame->id)
                        found=true;
                }
            }

        } while(found);

    }

    return true;
}

bool QDltParser::generateId(uint32_t startId,uint32_t endId, bool perApplication)
{

    qDebug() << "generateId" << startId << endId << perApplication;

    /* go through all messages */
    for(int j = 0;j<messages.size();j++)
    {
        QDltFibexFrame *frame = messages.at(j);
        uint32_t newId = frame->id;

        if(!checkId(startId,endId,newId,perApplication,frame->appid))
        {
            errorString = QString("Error: Failed to check and generate new id for context %1 in range %2 - %3").arg(frame->idString).arg(startId).arg(endId);
            return false;
        }
        else
        {
            if(frame->id!=newId)
            {
                frame->id=newId;
                messageIds[frame->idString]= newId;
            }
        }
    }

    return true;
}


bool QDltParser::readFibex(QString &fileName)
{
   QFile file(fileName);

   qDebug() << "readFibex" << fileName;

   if (!file.open(QFile::ReadOnly | QFile::Text))
   {
       return false;
   }

   QDltFibexPdu *pdu = 0;
   QDltFibexFrame *frame = 0;
   QList<QDltFibexPdu*> pdulist;

   QXmlStreamReader xml(&file);
   while (!xml.atEnd()) {
         xml.readNext();

         if(xml.isStartElement())
         {
             if(xml.name() == QString("PDU"))
             {
                 if(!pdu)
                 {
                   pdu = new QDltFibexPdu();
                   pdu->id = xml.attributes().value(QString("ID")).toString();
                 }
             }
             if(xml.name() == QString("DESC"))
             {
                 if(pdu)
                     pdu->description = xml.readElementText();
             }
             if(xml.name() == QString("BYTE-LENGTH"))
             {
                 if(frame)
                     frame->byteLength = xml.readElementText().toInt();
                 if(pdu)
                     pdu->byteLength = xml.readElementText().toInt();

             }
             if(xml.name() == QString("SIGNAL-INSTANCE"))
             {
                 // nothing todo
             }
             if(xml.name() == QString("SIGNAL-REF"))
             {
                 if(pdu)
                 {
                     QString text = xml.attributes().value("ID-REF").toString();
                     pdu->typeInfo = text;
#if 0
                     if (text == "S_BOOL")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_BOOL;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoBool;
                     }
                     else if (text == "S_SINT8")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_SINT | DLT_TYLE_8BIT;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoSInt;
                     }
                     else if (text == "S_UINT8")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_UINT | DLT_TYLE_8BIT;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoUInt;
                     }
                     else if (text == "S_SINT16")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_SINT | DLT_TYLE_16BIT;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoSInt;
                     }
                     else if (text == "S_UINT16")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_UINT | DLT_TYLE_16BIT;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoUInt;
                     }
                     else if (text == "S_SINT32")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_SINT | DLT_TYLE_32BIT;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoSInt;
                     }
                     else if (text == "S_UINT32")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_UINT | DLT_TYLE_32BIT;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoUInt;
                     }
                     else if (text == "S_SINT64")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_SINT | DLT_TYLE_64BIT;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoSInt;
                     }
                     else if (text == "S_UINT64")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_UINT | DLT_TYLE_64BIT;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoUInt;
                     }
                     else if (text == "S_FLOA16")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_FLOA | DLT_TYLE_16BIT;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoFloa;
                     }
                     else if (text == "S_FLOA32")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_FLOA | DLT_TYLE_32BIT;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoFloa;
                     }
                     else if (text == "S_FLOA64")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_FLOA | DLT_TYLE_64BIT;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoFloa;
                     }
                     else if (text == "S_STRG_ASCII")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_STRG | DLT_SCOD_ASCII;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoStrg;
                     }
                     else if (text == "S_STRG_UTF8")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_STRG | DLT_SCOD_UTF8;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoStrg;
                     }
                     else if (text == "S_RAWD")
                     {
                         pdu->typeInfo = DLT_TYPE_INFO_RAWD;
                         //pdu->typeInfo = QDltArgument::DltTypeInfoRawd;
                     }
                     else
                     {
                         pdu->typeInfo = 0;
                     }
#endif
                 }
             }
             if(xml.name() == QString("FRAME"))
             {
                 if(!frame)
                 {
                   frame = new QDltFibexFrame();
                   frame->id = xml.attributes().value(QString("ID")).toString().mid(3).toInt();
                 }
             }
             if(xml.name() == QString("MANUFACTURER-EXTENSION"))
             {
                 // nothing todo
             }
             if(xml.name() == QString("MESSAGE_TYPE"))
             {
                 if(frame)
                 {
                     QString text = xml.readElementText();
#if 0
                     if (text == QString("DLT_TYPE_LOG"))
                     {
                           frame->messageType = DLT_TYPE_LOG;
                     }
                     else if (text == QString("DLT_TYPE_APP_TRACE"))
                     {
                         frame->messageType = DLT_TYPE_APP_TRACE;
                     }
                     else if (text == QString("DLT_TYPE_NW_TRACE"))
                     {
                         frame->messageType = DLT_TYPE_NW_TRACE;
                     }
                     else if (text == QString("DLT_TYPE_CONTROL"))
                     {
                         frame->messageType = DLT_TYPE_CONTROL;
                     }
                     else
                     {
                         frame->messageType = 0;
                     }
#endif
                 }
             }
             if(xml.name() == QString("MESSAGE_INFO"))
             {
                 if(frame)
                 {
                     QString text = xml.readElementText();
                     frame->messageInfo = text;
#if 0
                     if (text == QString("DLT_LOG_DEFAULT"))
                     {
                         frame->messageInfo = DLT_LOG_DEFAULT;
                     }
                     else if (text == QString("DLT_LOG_OFF"))
                     {
                         frame->messageInfo = DLT_LOG_OFF;
                     }
                     else if (text == QString("DLT_LOG_FATAL"))
                     {
                         frame->messageInfo = DLT_LOG_FATAL;
                     }
                     else if (text == QString("DLT_LOG_ERROR"))
                     {
                         frame->messageInfo = DLT_LOG_ERROR;
                     }
                     else if (text == QString("DLT_LOG_WARN"))
                     {
                         frame->messageInfo = DLT_LOG_WARN;
                     }
                     else if (text == QString("DLT_LOG_INFO"))
                     {
                         frame->messageInfo = DLT_LOG_INFO;
                     }
                     else if (text == QString("DLT_LOG_DEBUG"))
                     {
                         frame->messageInfo = DLT_LOG_DEBUG;
                     }
                     else if (text == QString("DLT_LOG_VERBOSE"))
                     {
                         frame->messageInfo = DLT_LOG_VERBOSE;
                     }
                     else
                     {
                         frame->messageInfo = 0;
                     }
#endif
                 }
             }
             if(xml.name() == QString("APPLICATION_ID"))
             {
                 if(frame)
                 {
                     frame->appid = xml.readElementText();
                 }
             }
             if(xml.name() == QString("CONTEXT_ID"))
             {
                 if(frame)
                 {
                     frame->ctid = xml.readElementText();
                 }
             }
             if(xml.name() == QString("MESSAGE_SOURCE_FILE"))
             {
                 if(frame)
                 {
                     frame->filename = xml.readElementText();
                 }
             }
             if(xml.name() == QString("MESSAGE_LINE_NUMBER"))
             {
                 if(frame)
                 {
                     frame->lineNumber = xml.readElementText().toInt();
                 }
             }
             if(xml.name() == QString("PDU-INSTANCE"))
             {
             }
             if(xml.name() == QString("PDU-REF"))
             {
                 if(frame)
                 {
                     QDltFibexPduRef *ref = new QDltFibexPduRef();
                     ref->id = xml.attributes().value(QString("ID-REF")).toString();
                     frame->pdurefreflist.append(ref);
                     frame->pduRefCounter++;
                 }
             }
         }
         if(xml.isEndElement())
         {
             if(xml.name() == QString("PDU"))
             {
                 if(pdu)
                 {
                     pdulist.append(pdu);
                     pdu = 0;
                 }
             }
             if(xml.name() == QString("DESC"))
             {
             }
             if(xml.name() == QString("BYTE-LENGTH"))
             {
             }
             if(xml.name() == QString("SIGNAL-INSTANCE"))
             {
             }
             if(xml.name() == QString("SIGNAL-REF"))
             {
             }
             if(xml.name() == QString("FRAME"))
             {
                 if(frame)
                 {
                     messages.append(frame);
                     frame = 0;
                 }
             }
             if(xml.name() == QString("MANUFACTURER-EXTENSION"))
             {
             }
             if(xml.name() == QString("MESSAGE_TYPE"))
             {
             }
             if(xml.name() == QString("MESSAGE_INFO"))
             {
             }
             if(xml.name() == QString("APPLICATION_ID"))
             {
             }
             if(xml.name() == QString("CONTEXT_ID"))
             {
             }
             if(xml.name() == QString("PDU-INSTANCE"))
             {
             }
             if(xml.name() == QString("PDU-REF"))
             {
             }

         }
   }
   if (xml.hasError()) {
       errorString = "XML Parser error: " + xml.errorString();
       return false;
   }

   file.close();

   /* create PDU Ref links */
   foreach(QDltFibexFrame *frame, messages)
   {
       foreach(QDltFibexPduRef *ref, frame->pdurefreflist)
       {
           foreach(QDltFibexPdu *pdu, pdulist)
           {
               if(pdu->id == ref->id)
               {
                   QDltFibexPdu *pdunew = new QDltFibexPdu();
                   frame->pdureflist.append(pdunew);

                   pdunew->byteLength = pdu->byteLength;
                   pdunew->description = pdu->description;
                   pdunew->typeInfo = pdu->typeInfo;

                   break;
               }
           }
       }
   }

   return true;
}

bool QDltParser::checkDoubleIds(QString &text,bool perApplication)
{
    int i,j;

    qDebug() << "checkDoubleIds" << perApplication;

    /* empty text first */
    text.clear();

    /* check if double Ids are found */
    /* compare each Id against all following Ids */
    for(i = 0;i<messages.size();i++)
    {
        QDltFibexFrame *frame = messages.at(i);

        for(j = i+1;j<messages.size();j++)
        {
            QDltFibexFrame *cmpframe = messages.at(j);

            if(perApplication)
            {
                if( frame->idString == cmpframe->idString && frame->appid == cmpframe->appid)
                {
                    /* double id found and add it to list */
                    text += QString("Error: Double Id Text: %1 (%2:%3,%4:%5)\n").arg(frame->idString).arg(frame->filename).arg(frame->lineNumber).arg(cmpframe->filename).arg(cmpframe->lineNumber);
                }
            }
            else
            {
                if( frame->idString == cmpframe->idString )
                {
                    /* double id found and add id to list */
                    text += QString("Error: Double Id Text: %1 (%2:%3,%4:%5)\n").arg(frame->idString).arg(frame->filename).arg(frame->lineNumber).arg(cmpframe->filename).arg(cmpframe->lineNumber);
                }
            }
        }
    }

    /* check if double Ids are found */
    /* compare each Id against all following Ids */
    for(i = 0;i<messages.size();i++)
    {
        QDltFibexFrame *frame = messages.at(i);

        for(j = i+1;j<messages.size();j++)
        {
            QDltFibexFrame *cmpframe = messages.at(j);

            if(perApplication)
            {
                if( frame->id == cmpframe->id && frame->appid == cmpframe->appid)
                {
                    /* double id found and add id to list */
                    text += QString("Error: Double Id: %1 (%2:%3,%4:%5)\n").arg(frame->id).arg(frame->filename).arg(frame->lineNumber).arg(cmpframe->filename).arg(cmpframe->lineNumber);
                }
            }
            else
            {
                if( frame->id == cmpframe->id )
                {
                    /* double id found and add id to list */
                    text += QString("Error: Double Id: %1 (%2:%3,%4:%5)\n").arg(frame->id).arg(frame->filename).arg(frame->lineNumber).arg(cmpframe->filename).arg(cmpframe->lineNumber);
                }
            }
        }
    }

    if(text.isEmpty())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool QDltParser::parseConfiguration(QString fileName)
{
    /* parse configuration file */
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorString = "Cannot open file " + fileName;
        return false;
    }

    while (!file.atEnd()) {
        QByteArray line = file.readLine();

        QString text(line);

        text.remove(QChar('\n'));
        text.remove(QChar('\r'));

        QStringList list = text.split(" ");

        if(list.size()<1)
        {
            // do nothing
        }
        else if(list[0]=="clear")
        {
            clear();
        }
        else if(list[0]=="read-fibex")
        {
            if(list.size()<2)
            {
                errorString = "Missing parameters in line " + text;
                return false;
            }
            QString filename = list[1];
            if(!readFibex(filename))
            {
                file.close();
                return false;
            }
        }
        else if(list[0]=="write-fibex")
        {
            if(list.size()<2)
            {
                errorString = "Missing parameters in line " + text;
                return false;
            }
            QString filename = list[1];
            if(!writeFibex(filename))
            {
                file.close();
                return false;
            }
        }
        else if(list[0]=="write-csv")
        {
            if(list.size()<2)
            {
                errorString = "Missing parameters in line " + text;
                return false;
            }
            QString filename = list[1];
            if(!writeCsv(filename))
            {
                file.close();
                return false;
            }
        }
        else if(list[0]=="write-id")
        {
            if(list.size()<2)
            {
                errorString = "Missing parameters in line " + text;
                return false;
            }
            QString filename = list[1];
            if(!writeIdHeader(filename,false))
            {
                file.close();
                 return false;
            }
        }
        else if(list[0]=="write-id-app")
        {
            if(list.size()<2)
            {
                errorString = "Missing parameters in line " + text;
                return false;
            }
            QString filename = list[1];
            if(!writeIdHeader(filename,true))
            {
                file.close();
                return false;
            }
        }
        else if(list[0]=="check-double")
        {
            QString text;
            if(!checkDoubleIds(text,false))
            {
                file.close();
                return false;
            }
        }
        else if(list[0]=="check-double-app")
        {
            QString text;
            if(!checkDoubleIds(text,true))
            {
                file.close();
                return false;
            }
        }
        else if(list[0]=="parse-file")
        {
            if(list.size()<2)
            {
                errorString = "Missing parameters in line " + text;
                return false;
            }
            QString filename = list[1];
            // parse file
            if(!parseFile(filename))
            {
                file.close();
                return false;
            }

            // update message ids and application/context ids
            if(!parseCheck())
            {
                file.close();
                return false;
            }
        }
        else if(list[0]=="converte-file")
        {
            if(list.size()<2)
            {
                errorString = "Missing parameters in line " + text;
                return false;
            }
            QString filename = list[1];
            // parse file
            if(!converteFile(filename))
            {
                file.close();
                return false;
            }
        }
        else if(list[0]=="update-id")
        {
            if(list.size()<3)
            {
                errorString = "Missing parameters in line " + text;
                return false;
            }
            uint32_t startId = list[1].toUInt();
            uint32_t endId = list[2].toUInt();
            // parse file
            if(!generateId(startId,endId,false))
            {
                file.close();
                return false;
            }
        }
        else if(list[0]=="update-id-app")
        {
            if(list.size()<3)
            {
                errorString = "Missing parameters in line " + text;
                return false;
            }
            uint32_t startId = list[1].toUInt();
            uint32_t endId = list[2].toUInt();
            // parse file
            if(!generateId(startId,endId,true))
            {
                file.close();
                return false;
            }
        }
        else
        {
            if(!list[0].isEmpty())
            {
                errorString = "Unknown command " + list[0];
                return false;
            }
        }
    }

    file.close();

    return true;
}

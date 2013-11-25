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
 * \file nonverboseplugin.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtGui>
#include <QMessageBox>

#include "nonverboseplugin.h"
#include "dlt_protocol.h"
#include "dlt_user.h"

extern char *message_type[];
extern const char *log_info[];
extern const char *trace_type[];
extern const char *nw_trace_type[];
extern const char *control_type[];
extern const char *service_id[];
extern const char *return_type[];

QString NonverbosePlugin::name()
{
    return QString("Non Verbose Mode Plugin");
}

QString NonverbosePlugin::pluginVersion(){
    return NON_VERBOSE_PLUGIN_VERSION;
}

QString NonverbosePlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString NonverbosePlugin::description()
{
    return QString();
}

QString NonverbosePlugin::error()
{
    return m_error_string;
}

bool NonverbosePlugin::loadConfig(QString filename)
{

   bool ret = true;
  /* remove all stored items */
    m_error_string ="";
    foreach(DltFibexPdu *pdu, pdumap)
        delete pdu;
    pdumap.clear();    

    foreach(DltFibexFrame *frame, framemap)
        delete frame;
    framemap.clear();


    if ( filename.length() <= 0 )
    {
        m_error_string = "No XML specified. Plugin only works with valid configuration file";
        return false;
    }

    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
            m_error_string = "Could not open File: ";
            m_error_string.append(filename).append(" for configuration.");

            return false;
    }

    QString warning_text;

    DltFibexPdu *pdu = 0;
    DltFibexFrame *frame = 0;

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
          xml.readNext();

          if(xml.isStartElement())
          {
              if(xml.name() == QString("PDU"))
              {
                  if(!pdu)
                  {
                    pdu = new DltFibexPdu();
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
                      if (text == "S_BOOL")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_BOOL;
                          pdu->typeInfo = QDltArgument::DltTypeInfoBool;
                      }
                      else if (text == "S_SINT8")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_SINT | DLT_TYLE_8BIT;
                          pdu->typeInfo = QDltArgument::DltTypeInfoSInt;
                      }
                      else if (text == "S_UINT8")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_UINT | DLT_TYLE_8BIT;
                          pdu->typeInfo = QDltArgument::DltTypeInfoUInt;
                      }
                      else if (text == "S_SINT16")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_SINT | DLT_TYLE_16BIT;
                          pdu->typeInfo = QDltArgument::DltTypeInfoSInt;
                      }
                      else if (text == "S_UINT16")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_UINT | DLT_TYLE_16BIT;
                          pdu->typeInfo = QDltArgument::DltTypeInfoUInt;
                      }
                      else if (text == "S_SINT32")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_SINT | DLT_TYLE_32BIT;
                          pdu->typeInfo = QDltArgument::DltTypeInfoSInt;
                      }
                      else if (text == "S_UINT32")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_UINT | DLT_TYLE_32BIT;
                          pdu->typeInfo = QDltArgument::DltTypeInfoUInt;
                      }
                      else if (text == "S_SINT64")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_SINT | DLT_TYLE_64BIT;
                          pdu->typeInfo = QDltArgument::DltTypeInfoSInt;
                      }
                      else if (text == "S_UINT64")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_UINT | DLT_TYLE_64BIT;
                          pdu->typeInfo = QDltArgument::DltTypeInfoUInt;
                      }
                      else if (text == "S_FLOA16")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_FLOA | DLT_TYLE_16BIT;
                          pdu->typeInfo = QDltArgument::DltTypeInfoFloa;
                      }
                      else if (text == "S_FLOA32")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_FLOA | DLT_TYLE_32BIT;
                          pdu->typeInfo = QDltArgument::DltTypeInfoFloa;
                      }
                      else if (text == "S_FLOA64")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_FLOA | DLT_TYLE_64BIT;
                          pdu->typeInfo = QDltArgument::DltTypeInfoFloa;
                      }
                      else if (text == "S_STRG_ASCII")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_STRG | DLT_SCOD_ASCII;
                          pdu->typeInfo = QDltArgument::DltTypeInfoStrg;
                      }
                      else if (text == "S_STRG_UTF8")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_STRG | DLT_SCOD_UTF8;
                          pdu->typeInfo = QDltArgument::DltTypeInfoUtf8;
                      }
                      else if (text == "S_RAWD" || text == "S_RAW")
                      {
                          //pdu->typeInfo = DLT_TYPE_INFO_RAWD;
                          pdu->typeInfo = QDltArgument::DltTypeInfoRawd;
                      }
                      else
                      {
                          pdu->typeInfo = 0;
                      }
                  }
              }
              if(xml.name() == QString("FRAME"))
              {
                  if(!frame)
                  {
                    frame = new DltFibexFrame();
                    frame->id = xml.attributes().value(QString("ID")).toString();
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
                  }
              }
              if(xml.name() == QString("MESSAGE_INFO"))
              {
                  if(frame)
                  {
                      QString text = xml.readElementText();
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
              if(xml.name() == QString("PDU-INSTANCE"))
              {
              }
              if(xml.name() == QString("PDU-REF"))
              {
                  if(frame)
                  {
                      DltFibexPduRef *ref = new DltFibexPduRef();
                      ref->id = xml.attributes().value(QString("ID-REF")).toString();
                      frame->pdureflist.append(ref);
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
                      pdumap[pdu->id] = pdu;
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
                      if (framemap.contains(frame->id)){
                          // we don't add another instance but add a warning msgbox.
                          warning_text+=frame->id + ", ";
                          delete frame;
                      }else{
                        framemap[frame->id] = frame;
                      }
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
        m_error_string.append("\nXML Parser error: ").append(xml.errorString()).append("\n");
        ret = false;
    }

    file.close();

    if (warning_text.length()){
        warning_text.chop(2); // remove last ", "
        m_error_string.append("Duplicated FRAMES ignored: \n").append(warning_text);
        ret = true;//it is not breaking the plugin functionality, but could cause wrong decoding.
    }

    /* create PDU Ref links */
    foreach(DltFibexFrame *frame, framemap)
    {
        foreach(DltFibexPduRef *ref, frame->pdureflist)
        {
            foreach(DltFibexPdu *pdu, pdumap)
            {
                if(pdu->id == ref->id)
                {
                    ref->ref = pdu;
                    break;
                }
            }
        }
    }

    return ret;
}

bool NonverbosePlugin::saveConfig(QString /*filename*/)
{
    return true;
}

QStringList NonverbosePlugin::infoConfig()
{
    QStringList list;

    foreach(DltFibexFrame *frame, framemap)
    {
        QString text;
        text += frame->id + QString(" AppI:%1 CtI:%2 Len:%3 MT:%4 MI:%5").arg(frame->appid).arg(frame->ctid).arg(frame->byteLength).arg(frame->messageType).arg(frame->messageInfo);
        int c = 0;
        foreach(DltFibexPduRef *ref, frame->pdureflist)
        {
            if(c == 0)
                text += " (";
            text += ref->id;
            if(ref->ref)
                text += QString(" Des:%1 TI:%2 Len:%3").arg(ref->ref->description).arg(ref->ref->typeInfo).arg(ref->ref->byteLength);
            if(c == (frame->pdureflist.size()-1))
                text += ")";
            else
                text += ", ";
        }
        list.append(text);
    }
    return list;
}

bool NonverbosePlugin::isMsg(QDltMsg &msg, int triggeredByUser)
{
    Q_UNUSED(triggeredByUser)

    if((msg.getMode() != QDltMsg::DltModeNonVerbose))
    {
        /* message is not a non-verbose message */
        return false;
    }
    if((msg.getType() == QDltMsg::DltTypeControl))
    {
        /* message is a control message */
        return false;
    }

    QString idtext = QString("ID_%1").arg(msg.getMessageId());
    return framemap.contains(idtext);
}

bool NonverbosePlugin::decodeMsg(QDltMsg &msg, int triggeredByUser)
{
    Q_UNUSED(triggeredByUser)
    int offset = 4;

    if((msg.getMode() != QDltMsg::DltModeNonVerbose))
    {
        /* message is not a non-verbose message */
        return false;
    }
    if((msg.getType() == QDltMsg::DltTypeControl))
    {
        /* message is a control message */
        return false;
    }

    QString idtext = QString("ID_%1").arg(msg.getMessageId());
    if(!framemap.contains(idtext))
        return false;

    DltFibexFrame *frame = framemap[idtext];

    /* set message data */
    msg.setApid(frame->appid);
    msg.setCtid(frame->ctid);
    msg.setNumberOfArguments(frame->pdureflist.size());
    msg.setType((QDltMsg::DltTypeDef)(frame->messageType));
    msg.setSubtype(frame->messageInfo);
    QByteArray payload = msg.getPayload();

    /* Look for all PDUs for this message */
    for (int i=0;i < frame->pdureflist.size();i++)
    {
        QDltArgument argument;
        QByteArray data;
        unsigned short length;
        DltFibexPdu *pdu = frame->pdureflist[i]->ref;
        if(pdu)
        {
            if(!pdu->description.isEmpty()) {
                argument.setTypeInfo(QDltArgument::DltTypeInfoStrg);
                argument.setEndianness(msg.getEndianness());
                argument.setOffsetPayload(offset);
                data.append(pdu->description);
                argument.setData(data);
            }
            else {
                argument.setTypeInfo((QDltArgument::DltTypeInfoDef)(pdu->typeInfo));
                argument.setEndianness(msg.getEndianness());
                argument.setOffsetPayload(offset);

                if( (pdu->typeInfo == QDltArgument::DltTypeInfoStrg) || (pdu->typeInfo == QDltArgument::DltTypeInfoRawd) || (pdu->typeInfo == QDltArgument::DltTypeInfoUtf8))
                {
                    if((unsigned int)payload.size()<(offset+sizeof(unsigned short)))
                        break;
                    if(argument.getEndianness() == QDltMsg::DltEndiannessLittleEndian)
                        length = *((unsigned short*) (payload.constData()+offset));
                    else
                        length = DLT_SWAP_16(*((unsigned short*) (payload.constData()+offset)));
                    offset += sizeof(unsigned short);
                    argument.setData(payload.mid(offset,length));
                    offset += length;
                }
                else
                {
                    argument.setData(payload.mid(offset,pdu->byteLength));
                    offset += pdu->byteLength;
                }

            }

            msg.addArgument(argument);
        }
    }

    return true;
}

#ifndef QT5
Q_EXPORT_PLUGIN2(nonverboseplugin, NonverbosePlugin);
#endif

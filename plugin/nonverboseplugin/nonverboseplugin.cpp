#include <QtGui>

#include "nonverboseplugin.h"

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

QString NonverbosePlugin::description()
{
    return QString();
}

QString NonverbosePlugin::error()
{
    return QString();
}

bool NonverbosePlugin::loadConfig(QString filename)
{
    /* remove all stored items */
    for(int num=0;num<pdulist.count();num++)
        delete pdulist.at(num);
    pdulist.clear();
    for(int num=0;num<framelist.count();num++)
        delete framelist.at(num);
    framelist.clear();

    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
             return false;
    }

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
                          pdu->typeInfo = QDltArgument::DltTypeInfoStrg;
                      }
                      else if (text == "S_RAWD")
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
                      framelist.append(frame);
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
        QMessageBox::warning(0, QString("XML Parser error"),
                             xml.errorString());
    }

    file.close();

    /* create PDU Ref links */
    for(int numframe=0;numframe<framelist.size();numframe++)
    {
        DltFibexFrame *frame = framelist[numframe];
        for(int numref=0;numref<frame->pdureflist.size();numref++)
        {
               DltFibexPduRef *ref = frame->pdureflist[numref];
               for(int numpdu=0;numpdu<pdulist.size();numpdu++)
               {
                   DltFibexPdu *pdu = pdulist[numpdu];
                   if(pdu->id == ref->id)
                   {
                       ref->ref = pdu;
                       break;
                   }
               }
        }
    }

    return true;
}

bool NonverbosePlugin::saveConfig(QString /*filename*/)
{
    return true;
}

QStringList NonverbosePlugin::infoConfig()
{
    QStringList list;

    for(int pos=0;pos<framelist.size();pos++)
    {
        QString text;
        DltFibexFrame *frame = framelist[pos];
        text += frame->id;
        for(int numref=0;numref<frame->pdureflist.size();numref++)
        {
            DltFibexPduRef *ref = frame->pdureflist[numref];
            if(numref == 0)
                text += " (";
            text += ref->id;
            if(ref->ref)
                text += QString(":%1:%2").arg(ref->ref->description).arg(ref->ref->typeInfo);
            if(numref == (frame->pdureflist.size()-1))
                text += ")";
            else
                text += ", ";
        }
        list.append(text);
    }
    return list;
}

bool NonverbosePlugin::isMsg(QDltMsg &msg)
{
    bool found = false;

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
    /* Look for id in frames */
    DltFibexFrame *frame=0;
    for(int num=0;num<framelist.size();num++)
    {
        frame = framelist[num];
        if(idtext == frame->id)
        {
            found = true;
            break;
        }
    }

    return found;
}

bool NonverbosePlugin::decodeMsg(QDltMsg &msg)
{
    bool found = false;
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
    /* Look for id in frames */
    DltFibexFrame *frame=0;
    for(int num=0;num<framelist.size();num++)
    {
        frame = framelist[num];
        if(idtext == frame->id)
        {
            found = true;
            break;
        }
    }

    if(!found) {
        /* message not found in list */
        return false;
    }

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

                if( (pdu->typeInfo == QDltArgument::DltTypeInfoStrg) || (pdu->typeInfo == QDltArgument::DltTypeInfoRawd))
                {
                    if((unsigned int)payload.size()<(offset+sizeof(unsigned short)))
                        break;
                    length = *((unsigned short*) (payload.data()+offset));
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

Q_EXPORT_PLUGIN2(nonverboseplugin, NonverbosePlugin);

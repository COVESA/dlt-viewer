/**
 * @licence app begin@
 * Copyright (C) 2011-2014  BMW AG
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
 * \file dltdbusplugin.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtGui>

#include "dltdbusplugin.h"
#include "dbus.h"
#include <dlt_user.h>

DltDBusPlugin::DltDBusPlugin() {
    dltFile = 0;
}

DltDBusPlugin::~DltDBusPlugin() {

}

QString DltDBusPlugin::name() {
    return QString("DLT DBus Plugin");
}

QString DltDBusPlugin::pluginVersion(){
    return DLT_DBUS_PLUGIN_VERSION;
}

QString DltDBusPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DltDBusPlugin::description() {
    return QString();
}

QString DltDBusPlugin::error() {
    return QString();
}

bool DltDBusPlugin::loadConfig(QString /*filename*/) {
    return true;
}

bool DltDBusPlugin::saveConfig(QString /*filename*/) {
    return true;
}

QStringList DltDBusPlugin::infoConfig() {
    return QStringList();
}

QWidget* DltDBusPlugin::initViewer() {
    form = new Form();
    return form;
}

void DltDBusPlugin::selectedIdxMsgDecoded(int , QDltMsg &/*msg*/){
    /* Show Decoded output */
}

void DltDBusPlugin::selectedIdxMsg(int /*index*/, QDltMsg &msg) {
    QString text;
    QDltArgument argument;

    if(!checkIfDBusMsg(msg))
    {
        form->setTextBrowserDBus("no DBus message!");
        form->setTextBrowserHeader("no DBus message!");
        form->setTextBrowserPayload("no DBus message!");
        form->setTextBrowserPayloadHex("no DBus message!");
        return;
    }

    /* decode DBus message */
    msg.getArgument(1,argument);
    QByteArray data = argument.getData();
    DltDBusDecoder dbusMsg;
    if(!dbusMsg.decodeHeader(data))
    {
        form->setTextBrowserDBus(QString("Decoder error: ") + dbusMsg.getLastError());
        form->setTextBrowserHeader(QString("Decoder error: ") + dbusMsg.getLastError());
        form->setTextBrowserPayload(QString("Decoder error: ") + dbusMsg.getLastError());
        form->setTextBrowserPayloadHex(QString("Decoder error: ") + dbusMsg.getLastError());
        return;
    }

    /* DBus message header */
    text += QString("<table border=\"1\" cellspacing=\"0\" cellheader=\"0\">");
    text += QString("<tr><th>Header</th><th>Value</th></tr>");

    text += QString("<tr><td>Endianess</td><td>%1</td></tr>").arg(dbusMsg.getEndianessString());
    text += QString("<tr><td>Message Type</td><td>%1</td></tr>").arg(dbusMsg.getMessageTypeString());
    text += QString("<tr><td>Flags</td><td>%1</td></tr>").arg(dbusMsg.getFlagsString());
    text += QString("<tr><td>Version</td><td>%1</td></tr>").arg(dbusMsg.getVersion());
    text += QString("<tr><td>Length</td><td>%1</td></tr>").arg(dbusMsg.getPayloadLength());
    text += QString("<tr><td>Serial</td><td>%1</td></tr>").arg(dbusMsg.getSerial());
    text += QString("<tr><td>ArrayLength</td><td>%1</td></tr>").arg(dbusMsg.getArrayLength());
    text += QString("<tr><td>Path</td><td>%1</td></tr>").arg(dbusMsg.getPath());
    text += QString("<tr><td>Interface</td><td>%1</td></tr>").arg(dbusMsg.getInterface());
    text += QString("<tr><td>Member</td><td>%1</td></tr>").arg(dbusMsg.getMember());
    text += QString("<tr><td>ErrorName</td><td>%1</td></tr>").arg(dbusMsg.getErrorName());
    text += QString("<tr><td>ReplySerial</td><td>%1</td></tr>").arg(dbusMsg.getReplySerial());
    text += QString("<tr><td>Destination</td><td>%1</td></tr>").arg(dbusMsg.getDestination());
    text += QString("<tr><td>Sender</td><td>%1</td></tr>").arg(dbusMsg.getSender());
    QByteArray signature = dbusMsg.getSignature();
    text += QString("<tr><td>Signature</td><td>%1</td></tr>").arg(msg.toAsciiTable(signature,false,false,true,256,256,false));
    text += QString("<tr><td>UnixFds</td><td>%1</td></tr>").arg(dbusMsg.getUnixFds());

    text += QString("</table>");

    form->setTextBrowserHeader(text);

    /* DBus message payload Hex*/
    QByteArray payload = dbusMsg.getPayload();
    text = QString("<h3>Size: %1</h3>").arg(payload.size());
    text += msg.toAsciiTable(payload,true,true,false);
    form->setTextBrowserPayloadHex(text);

    /* decode DBus payload */
    if(!dbusMsg.decodePayload())
    {
        form->setTextBrowserDBus(QString("Decoder error: ") + dbusMsg.getLastError());
        form->setTextBrowserPayload(QString("Decoder error: ") + dbusMsg.getLastError());
        return;
    }

    /* DBus message payload */
    text = QString("<table border=\"1\" cellspacing=\"0\" cellheader=\"0\">");
    text += QString("<tr><th>Type</th><th>Value</th></tr>");

    QList<DltDBusParameter> parameters = dbusMsg.getParameters();
    for(int num=0;num<parameters.size();num++)
    {
        DltDBusParameter parameter = parameters[num];
        text += QString("<tr><td>%1</td><td>%2</td></tr>").arg(parameter.getTypeString()).arg(parameter.getValue().toString());
    }

    text += QString("</table>");

    form->setTextBrowserPayload(text);

    /* DBus message overview */
    text = decodeMessageToString(dbusMsg);
    form->setTextBrowserDBus(text);
}


void DltDBusPlugin::initFileStart(QDltFile *file){
    dltFile = file;
    methods.clear();

    // clear old map
    QMapIterator<uint32_t, QDltSegmentedMsg*> i(segmentedMessages);
    while (i.hasNext()) {
        i.next();
        delete i.value();
    }
    segmentedMessages.clear();
}

void DltDBusPlugin::methodsAddMsg(QDltMsg &msg)
{
    QDltArgument argument1,argument2;

    msg.getArgument(0,argument1);
    msg.getArgument(1,argument2);

    // check if single network message
    if(msg.getNumberOfArguments()!=2 ||
       argument1.getTypeInfo()!=QDltArgument::DltTypeInfoRawd ||
       argument2.getTypeInfo()!=QDltArgument::DltTypeInfoRawd)
        return;

    /* Show DBus message Decoding */
    QByteArray data = argument1.getData()+argument2.getData();
    DltDBusDecoder dbusMsg;
    QString text;
    if(dbusMsg.decode(data))
    {
        if(dbusMsg.getMessageType()==DBUS_MESSAGE_TYPE_METHOD_CALL)
        {
           methods[DltDbusMethodKey(dbusMsg.getSender(),dbusMsg.getSerial())] = dbusMsg.getInterface() + "." + dbusMsg.getMember();
        }
    }
    else
    {

    }
}

void DltDBusPlugin::segmentedMsg(QDltMsg &msg)
{
    QDltArgument argument1,argument2;
    uint32_t handle;

    msg.getArgument(0,argument1);
    msg.getArgument(1,argument2);

    // get handle
    if(argument2.getTypeInfo()!=QDltArgument::DltTypeInfoUInt)
    {
         return;
    }
    handle = argument2.getValue().toUInt();

    // check if this is a segmented network message
    if(argument1.getTypeInfo()!=QDltArgument::DltTypeInfoStrg)
    {
        return;
    }

    //qDebug() << "segmentedMsg" << handle << argument1.getValue().toString();

    if(argument1.getValue().toString()=="NWST")
    {
      if(segmentedMessages.contains(handle))
      {
          // message already exists, this should not happen
          qDebug() << "Segment" << handle << "already exists!";
          return;
      }

      // add new segmented message
      QDltSegmentedMsg *seg = new QDltSegmentedMsg();
      segmentedMessages[handle] = seg;

      if(seg->add(msg))
      {
          // something went wrong
          qDebug() << seg->getError();
          return;
      }

    }
    else if(argument1.getValue().toString()=="NWCH")
    {
        if(segmentedMessages.contains(handle))
        {
            if(segmentedMessages[handle]->add(msg))
            {
                // something went wrong
                qDebug() << segmentedMessages[handle]->getError();
                return;
            }
        }
    }
    else if(argument1.getValue().toString()=="NWEN")
    {
       if(segmentedMessages.contains(handle))
       {
           if(segmentedMessages[handle]->add(msg))
           {
               // something went wrong
               qDebug() << segmentedMessages[handle]->getError();
               return;
           }
           if(segmentedMessages[handle]->complete())
           {
                //qDebug() << "Complete segemented message" << segmentedMessages[handle]->getPayload().size();
           }
           else
           {
                qDebug() << "Incomplete segemented message" << handle;
           }
       }
    }

}

void DltDBusPlugin::initMsg(int /*index*/, QDltMsg &msg){

    if(!checkIfDBusMsg(msg))
        return;

    // add method call
    methodsAddMsg(msg);

    // add segment
    segmentedMsg(msg);

}

void DltDBusPlugin::initMsgDecoded(int , QDltMsg &){
//empty. Implemented because derived plugin interface functions are virtual.
}

void DltDBusPlugin::initFileFinish(){
}

void DltDBusPlugin::updateFileStart(){
//empty. Implemented because derived plugin interface functions are virtual.
}

void DltDBusPlugin::updateMsg(int /*index*/, QDltMsg &msg){
    if(!checkIfDBusMsg(msg))
        return;

    // add method call
    methodsAddMsg(msg);

    // add segment
    segmentedMsg(msg);
}

void DltDBusPlugin::updateMsgDecoded(int , QDltMsg &){
//empty. Implemented because derived plugin interface functions are virtual.
}

void DltDBusPlugin::updateFileFinish(){
}

bool DltDBusPlugin::isMsg(QDltMsg & msg, int triggeredByUser)
{
    Q_UNUSED(triggeredByUser);

    return checkIfDBusMsg(msg);
}

bool DltDBusPlugin::decodeMsg(QDltMsg &msg, int triggeredByUser)
{
    QDltArgument argument1,argument2,argument;
    QString text;
    Q_UNUSED(triggeredByUser);

    if(!checkIfDBusMsg(msg))
        return false;

    msg.getArgument(0,argument1);
    msg.getArgument(1,argument2);

    // check if it is a common network message
    if(msg.getNumberOfArguments()==2 &&
       argument1.getTypeInfo()==QDltArgument::DltTypeInfoRawd &&
       argument2.getTypeInfo()==QDltArgument::DltTypeInfoRawd)
    {
        // this is a single network message
        QByteArray data = argument1.getData() + argument2.getData();
        DltDBusDecoder dbusMsg;
        if(dbusMsg.decode(data))
        {
            text += decodeMessageToString(dbusMsg);
        }
        else
        {
            text += "DBus Decoder error: " + dbusMsg.getLastError();
        }
        msg.removeArgument(1);
        msg.removeArgument(0);
    }
    else if(argument1.getTypeInfo()==QDltArgument::DltTypeInfoStrg &&
       argument1.getValue().toString()=="NWTR")
    {
        // this is a truncated network message
        // decode header only
        // this is a single network message
        QByteArray data = argument1.getData() + argument2.getData();
        DltDBusDecoder dbusMsg;
        if(dbusMsg.decode(data,true))
        {
            text += decodeMessageToString(dbusMsg,true);
        }
        else
        {
            text += "DBus Decoder error: " + dbusMsg.getLastError();
        }
        msg.removeArgument(1);
        msg.removeArgument(0);
    }
    else if(argument1.getTypeInfo()==QDltArgument::DltTypeInfoStrg &&
            argument1.getValue().toString()=="NWEN")
    {
        // this is the end of a segmented message
        // decode now the whole message
        // get handle
        if(argument2.getTypeInfo()==QDltArgument::DltTypeInfoUInt)
        {
            uint32_t handle = argument2.getValue().toUInt();
            if(segmentedMessages.contains(handle))
            {
                if(segmentedMessages[handle]->complete())
                {
                    // show decoded message
                    QByteArray data = segmentedMessages[handle]->getHeader() + segmentedMessages[handle]->getPayload() ;
                    DltDBusDecoder dbusMsg;
                    if(dbusMsg.decode(data))
                    {
                        text += decodeMessageToString(dbusMsg);
                    }
                    else
                    {
                        text += "DBus Decoder error: " + dbusMsg.getLastError();
                    }
                }
                else
                {
                    text += "Incomplete segmented message " + QString("%1").arg(handle);
                }
            }
            else
            {
                text += "Unknown segmented message " + QString("%1").arg(handle);
            }
        }
        else
        {
            text += "Type error in segmented message";
        }
        msg.removeArgument(1);
        msg.removeArgument(0);
    }
    else
    {
        text = "DBus decode error!";
    }

    /* Add decoded parameter*/
    argument.setTypeInfo(QDltArgument::DltTypeInfoStrg);
    argument.setEndianness(msg.getEndianness());
    argument.setOffsetPayload(0);
    QByteArray dataText;
    dataText.append(text);
    argument.setData(dataText);
    msg.addArgument(argument);

    return true;
}

QString DltDBusPlugin::stringToHtml(QString str)
{
    str = str.replace("<","&#60;");
    str = str.replace(">","&#62;");

    return str;
}

bool DltDBusPlugin::checkIfDBusMsg(QDltMsg &msg)
{
    QDltArgument argument1,argument2;

    // at least two arguments in network message
    if(msg.getType()!=QDltMsg::DltTypeNwTrace || msg.getSubtype()!=QDltMsg::DltNetworkTraceIpc || msg.getNumberOfArguments()<2)
        return false;

    msg.getArgument(0,argument1);
    msg.getArgument(1,argument2);

    // check if this is a segmented network message
    if(argument1.getTypeInfo()==QDltArgument::DltTypeInfoStrg && (
       argument1.getValue().toString()=="NWCH" ||
       argument1.getValue().toString()=="NWST" ||
       argument1.getValue().toString()=="NWEN" ||
       argument1.getValue().toString()=="NWTR"))
        return true;

    // check if it is a common network message
    if(msg.getNumberOfArguments()==2 &&
       argument1.getTypeInfo()==QDltArgument::DltTypeInfoRawd &&
       argument2.getTypeInfo()==QDltArgument::DltTypeInfoRawd)
        return true;

    // no known network message
    return false;
}

QString DltDBusPlugin::decodeMessageToString(DltDBusDecoder &dbusMsg,bool headerOnly)
{
    QList<DltDBusParameter> parameters = dbusMsg.getParameters();
    QString text;
    QString method;
    switch(dbusMsg.getMessageType())
    {
        case DBUS_MESSAGE_TYPE_METHOD_CALL:
            text = QString("C [%1,%2] ").arg(dbusMsg.getSender()).arg(dbusMsg.getSerial()) + dbusMsg.getInterface()+"."+dbusMsg.getMember()+" (";
            break;
        case DBUS_MESSAGE_TYPE_METHOD_RETURN:
            method = methods[DltDbusMethodKey(dbusMsg.getDestination(),dbusMsg.getReplySerial())];
            text = QString("R [%1,%2] ").arg(dbusMsg.getDestination()).arg(dbusMsg.getReplySerial()) + method + " (";
            break;
        case DBUS_MESSAGE_TYPE_SIGNAL:
            text = QString("S [%1] ").arg(dbusMsg.getSender()) + dbusMsg.getInterface() + "." + dbusMsg.getMember() + " (";
            break;
        default:
            text = dbusMsg.getMessageTypeStringShort() + " " + dbusMsg.getInterface() + "." + dbusMsg.getMember();
    }

    if(headerOnly)
    {
        return text;
    }

    text += " (";

    bool start = true;
    char lastType = 0;
    for(int num=0;num<parameters.size();num++)
    {
        DltDBusParameter parameter = parameters[num];
        if(start)
        {
            start = false;
        }
        else
        {
            if(parameter.getType()==DBUS_TYPE_ARRAY || parameter.getType()==DBUS_TYPE_STRUCT || parameter.getType()==DBUS_TYPE_STRUCT_BEGIN || parameter.getType()==DBUS_TYPE_STRUCT_END ||
               parameter.getType()==DBUS_TYPE_DICT_ENTRY || parameter.getType()==DBUS_TYPE_DICT_ENTRY_BEGIN || parameter.getType()==DBUS_TYPE_DICT_ENTRY_END)
                ;
            else if(lastType==DBUS_TYPE_ARRAY || lastType==DBUS_TYPE_STRUCT || lastType==DBUS_TYPE_STRUCT_BEGIN || lastType==DBUS_TYPE_STRUCT_END ||
                    lastType==DBUS_TYPE_DICT_ENTRY || lastType==DBUS_TYPE_DICT_ENTRY_BEGIN || lastType==DBUS_TYPE_DICT_ENTRY_END)
                ;
            else
                text += ",";
        }
        if(parameter.getType()==DBUS_TYPE_STRING ||parameter.getType()==DBUS_TYPE_OBJECT_PATH || parameter.getType()==DBUS_TYPE_SIGNATURE )
            text += "\"";
        text += parameter.getValue().toString();
        if(parameter.getType()==DBUS_TYPE_STRING ||parameter.getType()==DBUS_TYPE_OBJECT_PATH || parameter.getType()==DBUS_TYPE_SIGNATURE )
            text += "\"";
        lastType = parameter.getType();
    }
    text += ")";

    return text;
}

#ifndef QT5
Q_EXPORT_PLUGIN2(dltdbusplugin, DltDBusPlugin);
#endif

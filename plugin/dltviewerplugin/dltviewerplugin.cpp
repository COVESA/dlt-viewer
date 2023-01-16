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
 * \file dltviewerplugin.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QtGui>

#include "dltviewerplugin.h"

DltViewerPlugin::DltViewerPlugin() {
    form = NULL;
    dltFile = 0;
}

DltViewerPlugin::~DltViewerPlugin() {

}

QString DltViewerPlugin::name()
{
    return plugin_name_displayed;
}

QString DltViewerPlugin::pluginVersion(){
    return DLT_VIEWER_PLUGIN_VERSION;
}

QString DltViewerPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DltViewerPlugin::description() {
    return QString();
}

QString DltViewerPlugin::error() {
    return QString();
}

bool DltViewerPlugin::loadConfig(QString /*filename*/) {
    return true;
}

bool DltViewerPlugin::saveConfig(QString /*filename*/) {
    return true;
}

QStringList DltViewerPlugin::infoConfig() {
    return QStringList();
}

QWidget* DltViewerPlugin::initViewer() {
    form = new DltViewer::Form();
    return form;
}

void DltViewerPlugin::selectedIdxMsgDecoded(int , QDltMsg &msg){
    /* Show Decoded output */
    form->setTextBrowserMessage(msg.toStringHeader()+"<br><br>"+stringToHtml(msg.toStringPayload()));
}

void DltViewerPlugin::selectedIdxMsg(int index, QDltMsg &msg) {
    QString text;
    QDltArgument argument;

    //if(!dltFile)
    //    return;

    /* Show Payload*/
    form->setTextBrowserUncoded(msg.toStringHeader()+"<br><br>"+stringToHtml(msg.toStringPayload()));

    /* Show details */
    text = QString("<html><body>");

    text += QString("<h3>Header</h3>");
    text += QString("<table border=\"1\" cellspacing=\"0\" cellheader=\"0\">");
    //qDebug() << text;
    text += QString("<tr><th>Index</th><th>Time</th><th>Timestamp</th><th>Count</th><th>Ecuid</th><th>Apid</th><th>Ctid</th><th>SessionId</th>");
    text += QString("<th>Type</th><th>Subtype</th><th>Mode</th><th>Endianness</th><th>#Args</th></tr>");
    //text += QString("<tr><td>%1</td>").arg(dltFile->getMsgFilterPos(index));
    text += QString("<tr><td>%1</td>").arg(index);
    text += QString("<td>%1.%2</td>").arg(msg.getTimeString()).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'));
    text += QString("<td>%1.%2</td>").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0'));
    text += QString("<td>%1</td>").arg(msg.getMessageCounter());
    text += QString("<td>%1</td>").arg(msg.getEcuid());
    text += QString("<td>%1</td>").arg(msg.getApid());
    text += QString("<td>%1</td>").arg(msg.getCtid());
    text += QString("<td>%1</td>").arg(msg.getSessionid());
    text += QString("<td>%1</td>").arg(msg.getTypeString());
    text += QString("<td>%1</td>").arg(msg.getSubtypeString());
    text += QString("<td>%1</td>").arg(msg.getModeString());
    text += QString("<td>%1</td>").arg(msg.getEndiannessString());
    text += QString("<td>%1</td>").arg(msg.getNumberOfArguments());
    text += QString("</tr></table>");

    if(msg.getType()==QDltMsg::DltTypeControl) {
        text += QString("<h3>Control Message</h3>");
        text += QString("<table border=\"1\" cellspacing=\"0\" cellheader=\"0\">");
        text += QString("<tr><th>MessageId</th><th>CtrlServiceId</th><th>CtrlReturnType</th></tr>");
        text += QString("<tr>");
        text += QString("<td>%1</td>").arg(msg.getMessageId());
        text += QString("<td>%1</td>").arg(msg.getCtrlServiceIdString());
        text += QString("<td>%1</td>").arg(msg.getCtrlReturnTypeString());
        text += QString("</tr></table>");
    }
    else {
        text += QString("<h3>Payload</h3>");
        text += QString("<table border=\"1\" cellspacing=\"0\" cellheader=\"0\">");
        text += QString("<tr><th>Index</th><th>Offest</th><th>Size</th><th>TypeInfo</th><th>Name</th><th>Unit</th><th>Text</th><th>Binary</th></tr>");
        for(int num=0;num<msg.getNumberOfArguments();num++) {
            text += QString("<tr>");
            if(msg.getArgument(num,argument)) {
                text += QString("<td>%1</td>").arg(num+1);
                text += QString("<td>%1</td>").arg(argument.getOffsetPayload());
                text += QString("<td>%1</td>").arg(argument.getDataSize());
                text += QString("<td>%1</td>").arg(argument.getTypeInfoString());
                text += QString("<td>%1</td>").arg(argument.getName());
                text += QString("<td>%1</td>").arg(argument.getUnit());
                text += QString("<td>");

                // Necessary to display < and > as characters in a HTML context.
                // Otherwise < and > would be handled as HTML tags and not the complete payload would be displayed.
                text += stringToHtml(argument.toString());
                text += QString("</td>");
                text += QString("<td>");
                text += argument.toString(true); /* Binary output */
                text += QString("</td>");
            }
            text += QString("</tr>");
        }
        text += QString("</table>");
    }

    text += QString("</body></html>");

    form->setTextBrowserDetails(text);

    /* Show DLTv2 Details */
    text = QString("<html><body>");
    if(msg.getVersionNumber()==2)
    {
        text += QString("<h4>Base Header</h4>");
        text += QString("With Session Id: %1<br>").arg(msg.getWithSessionId());
        text += QString("With App Context Id: %1<br>").arg(msg.getWithAppContextId());
        text += QString("With Ecu Id: %1<br>").arg(msg.getWithEcuId());
        text += QString("With Message Info: %1<br>").arg(msg.getWithHFMessageInfo());
        text += QString("With Number Of Arguments: %1<br>").arg(msg.getWithHFNumberOfArguments());
        text += QString("With Timestamp: %1<br>").arg(msg.getWithHFTimestamp());
        text += QString("With Message Id: %1<br>").arg(msg.getWithHFMessageId());
        if(msg.getContentInformation()==0)
        {
            text += QString("Content Information: verbose<br>");
        }
        else if(msg.getContentInformation()==1)
        {
            text += QString("Content Information: none verbose<br>");
        }
        else if(msg.getContentInformation()==2)
        {
            text += QString("Content Information: control<br>");
        }
        else
        {
            text += QString("Content Information: unknown value 3<br>");
        }
        text += QString("With Segementation: %1<br>").arg(msg.getWithSegementation());
        text += QString("With PrivacyLevel: %1<br>").arg(msg.getWithPrivacyLevel());
        text += QString("With Tags: %1<br>").arg(msg.getWithTags());
        text += QString("With Source File Name and Line Number: %1<br>").arg(msg.getWithSourceFileNameLineNumber());
        text += QString("Message Counter: %1<br>").arg(msg.getMessageCounter());
        text += QString("Message Info: %1<br>").arg(msg.getTypeString());
        text += QString("Message Type: %1<br>").arg(msg.getSubtypeString());
        if(msg.getWithHFNumberOfArguments())
        {
            text += QString("Number Of Arguments: %1<br>").arg(msg.getNumberOfArguments());
        }
        if(msg.getWithHFTimestamp())
        {
            text += QString("Timestamp Nanoseconds: %1<br>").arg(msg.getTimestampNanoseconds());
            text += QString("Timestamp Seconds: %1<br>").arg(msg.getTimestampSeconds());
        }
        if(msg.getWithHFMessageId())
        {
            text += QString("Message Id: %1<br>").arg(msg.getMessageId());
        }
        text += QString("<h4>Extension Header</h4>");
        if(msg.getWithEcuId())
        {
            text += QString("ECU Id: %1<br>").arg(msg.getEcuid());
        }
        if(msg.getWithAppContextId())
        {
            text += QString("App Id: %1<br>").arg(msg.getApid());
            text += QString("Ctx Id: %1<br>").arg(msg.getCtid());
        }
        if(msg.getWithSessionId())
        {
            text += QString("Session Id: %1<br>").arg(msg.getSessionid());
        }
        if(msg.getWithSourceFileNameLineNumber())
        {
            text += QString("Source File Name: %1<br>").arg(msg.getSourceFileName());
            text += QString("Line Number: %1<br>").arg(msg.getLineNumber());
        }
        if(msg.getWithTags())
        {
            text += QString("Tags: %1<br>").arg(msg.getTags().join(','));
        }
        if(msg.getWithPrivacyLevel())
        {
            text += QString("Privacy Level: %1<br>").arg(msg.getPrivacyLevel());
        }
        if(msg.getWithSegementation())
        {
            if(msg.getSegmentationFrameType()==0)
            {
                text += QString("Segmentation Frame Type: First Frame<br>");
                text += QString("Segmentation Total Length: %1<br>").arg(msg.getSegmentationTotalLength());
            }
            else if(msg.getSegmentationFrameType()==1)
            {
                text += QString("Segmentation Frame Type: Consecutive Frame<br>");
                text += QString("Segmentation Consecutive Frame: %1<br>").arg(msg.getSegmentationConsecutiveFrame());
            }
            else if(msg.getSegmentationFrameType()==2)
            {
                text += QString("Segmentation Frame Type: Las tFrame<br>");
            }
            else if(msg.getSegmentationFrameType()==3)
            {
                text += QString("Segmentation Frame Type: Abort Frame<br>");
                text += QString("Segmentation Abort Reason: %1<br>").arg(msg.getSegmentationAbortReason());
            }
            else
            {
                text += QString("Segmentation Frame Type: Unknown value %1<br>").arg(msg.getSegmentationFrameType());
            }
        }
        text += QString("<h4>Payload</h4>");
        text += QString("<table border=\"1\" cellspacing=\"0\" cellheader=\"0\">");
        text += QString("<tr><th>Index</th><th>Offest</th><th>Size</th><th>TypeInfo</th><th>Name</th><th>Unit</th><th>Text</th><th>Binary</th></tr>");
        for(int num=0;num<msg.getNumberOfArguments();num++) {
            text += QString("<tr>");
            if(msg.getArgument(num,argument)) {
                text += QString("<td>%1</td>").arg(num+1);
                text += QString("<td>%1</td>").arg(argument.getOffsetPayload());
                text += QString("<td>%1</td>").arg(argument.getDataSize());
                text += QString("<td>%1</td>").arg(argument.getTypeInfoString());
                text += QString("<td>%1</td>").arg(argument.getName());
                text += QString("<td>%1</td>").arg(argument.getUnit());
                text += QString("<td>");

                // Necessary to display < and > as characters in a HTML context.
                // Otherwise < and > would be handled as HTML tags and not the complete payload would be displayed.
                text += stringToHtml(argument.toString());
                text += QString("</td>");
                text += QString("<td>");
                text += argument.toString(true); /* Binary output */
                text += QString("</td>");
            }
            text += QString("</tr>");
        }
        text += QString("</table>");
    }
    else
    {
        text += "No DLTv2 message!";
    }
    text += QString("</body></html>");
    form->setTextBrowserDltV2(text);

    /* get binary payload */
    QByteArray bytes_header = msg.getHeader();
    QByteArray bytes_payload = msg.getPayload();

    /* Show Ascii output */
    form->setTextBrowserAscii(msg.toAsciiTable(bytes_header,false,false,true,8,64)+msg.toAsciiTable(bytes_payload,false,false,true,8,64));

    /* Show Binary output */
    form->setTextBrowserBinary(msg.toAsciiTable(bytes_header,true,true,false)+msg.toAsciiTable(bytes_payload,true,true,false));

    /* Show Mixed output */
    form->setTextBrowserMixed(msg.toAsciiTable(bytes_header,true,true,true)+msg.toAsciiTable(bytes_payload,true,true,true));
}


void DltViewerPlugin::initFileStart(QDltFile *file)
{
    dltFile = file;
    qDebug() << "Activate plugin" << plugin_name_displayed <<  DLT_VIEWER_PLUGIN_VERSION;
}

void DltViewerPlugin::initMsg(int /*index*/, QDltMsg & /*msg*/){
}

void DltViewerPlugin::initMsgDecoded(int , QDltMsg &){
//empty. Implemented because derived plugin interface functions are virtual.
}

void DltViewerPlugin::initFileFinish(){
}

void DltViewerPlugin::updateFileStart(){
//empty. Implemented because derived plugin interface functions are virtual.
}

void DltViewerPlugin::updateMsg(int /*index*/, QDltMsg &/*msg*/){
}

void DltViewerPlugin::updateMsgDecoded(int , QDltMsg &){
//empty. Implemented because derived plugin interface functions are virtual.
}

void DltViewerPlugin::updateFileFinish(){
}

QString DltViewerPlugin::stringToHtml(QString str)
{
    str = str.replace("<","&#60;");
    str = str.replace(">","&#62;");

    return str;
}


#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(dltviewerplugin, DltViewerPlugin);
#endif

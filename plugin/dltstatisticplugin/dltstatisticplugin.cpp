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
 * \file dummyviewerplugin.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtGui>

#include "dltstatisticplugin.h"

DltStatisticPlugin::DltStatisticPlugin()
{
    dltFile = 0;
    dltControl = 0;

    clear();

    resetStatistics();
}

DltStatisticPlugin::~DltStatisticPlugin()
{

}

QString DltStatisticPlugin::name()
{
    return QString("DLT Statistic Plugin");
}

QString DltStatisticPlugin::pluginVersion(){
    return DLT_STATISTIC_PLUGIN_VERSION;
}

QString DltStatisticPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DltStatisticPlugin::description()
{
    return QString();
}

QString DltStatisticPlugin::error()
{
    return QString();
}

bool DltStatisticPlugin::loadConfig(QString /* filename */)
{
    return true;
}

bool DltStatisticPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList DltStatisticPlugin::infoConfig()
{
    return QStringList();
}

QWidget* DltStatisticPlugin::initViewer()
{
    form = new Form();

    connect(form->timelineWidget,SIGNAL(cursorTime1Changed(time_t)),this,SLOT(cursorTime1Changed(time_t)));
    connect(form->timelineWidget,SIGNAL(cursorTime2Changed(time_t)),this,SLOT(cursorTime2Changed(time_t)));

    connect(form->bandwidthWidget,SIGNAL(cursorTime1Changed(time_t)),this,SLOT(cursorTime1Changed(time_t)));
    connect(form->bandwidthWidget,SIGNAL(cursorTime2Changed(time_t)),this,SLOT(cursorTime2Changed(time_t)));

    updateData();
    updateWidget();

    return form;
}


void DltStatisticPlugin::updateCounters(int index, QDltMsg &msg)
{
    Q_UNUSED(index);
    Q_UNUSED(msg);

    if(!dltFile)
        return;
}


void DltStatisticPlugin::selectedIdxMsg(int /*index*/, QDltMsg &/*msg*/) {
    if(!dltFile)
        return;

    //qDebug() << "undecoded: " << msg.toStringPayload();

}

void DltStatisticPlugin::selectedIdxMsgDecoded(int , QDltMsg &/*msg*/){

    //qDebug() << "decoded: " << msg.toStringPayload();

}

void DltStatisticPlugin::initFileStart(QDltFile *file){

    dltFile = file;

    clear();
    updateData();
    updateWidget();

    resetStatistics();

    counterMessages = dltFile->size();

    form->enableUpdateButton(false);
}

void DltStatisticPlugin::addToTimeline(int /*index*/,QDltMsg &msg)
{
    if(msg.getTime()!=0)
    {
        if(timeline.contains(msg.getTime()))
            timeline[msg.getTime()] = timeline[msg.getTime()] + 1;
        else
            timeline[msg.getTime()] = 1;
    }

    int size = msg.getPayloadSize()+msg.getHeaderSize();

    if(msg.getTime()!=0)
    {
        if(bandwidth.contains(msg.getTime()))
            bandwidth[msg.getTime()] = bandwidth[msg.getTime()] + size;
        else
            bandwidth[msg.getTime()] = size;
    }

    // calculate min and max time
    if(msg.getTime()!=0)
    {
        if(minimumTime==0 && maximumTime ==0)
        {
            minimumTime = msg.getTime();
            maximumTime = msg.getTime();
        }
        else
        {
            if( msg.getTime() < minimumTime)
                minimumTime = msg.getTime();
            if( msg.getTime() > maximumTime)
                maximumTime = msg.getTime();
        }
    }
}

void DltStatisticPlugin::initMsg(int index, QDltMsg &msg){

    updateCounters(index, msg);

    addToTimeline(index,msg);

    updateStatistics(index, msg);
}

void DltStatisticPlugin::initMsgDecoded(int , QDltMsg &){
//empty. Implemented because derived plugin interface functions are virtual.
}

void DltStatisticPlugin::initFileFinish(){
    if(!dltFile)
        return;

    counterMessages = dltFile->size();
    printStatistics();

    updateData();
    updateWidget();
}

void DltStatisticPlugin::updateFileStart(){

}

void DltStatisticPlugin::updateMsg(int index, QDltMsg &msg){
        if(!dltFile)
            return;

        updateCounters(index,msg);

        addToTimeline(index,msg);

        updateStatistics(index, msg);
}

void DltStatisticPlugin::updateMsgDecoded(int , QDltMsg &){
}

void DltStatisticPlugin::updateFileFinish(){
    if(!dltFile)
        return;

    counterMessages = dltFile->size();
    printStatistics();

    form->enableUpdateButton(true);
    updateData();
}

bool DltStatisticPlugin::initControl(QDltControl *control)
{
    dltControl = control;

    return true;
}

bool DltStatisticPlugin::initConnections(QStringList /*list*/)
{
    return true;
}

bool DltStatisticPlugin::controlMsg(int , QDltMsg &)
{
    return true;
}

bool DltStatisticPlugin::stateChanged(int /*index*/, QDltConnection::QDltConnectionState /*connectionState*/){

    return true;
}

bool DltStatisticPlugin::autoscrollStateChanged(bool /*enabled*/)
{
    return true;
}

void DltStatisticPlugin::clear()
{
    timeline.clear();
    bandwidth.clear();

    minimumTime = 0;
    maximumTime = 0;
}

void DltStatisticPlugin::updateData()
{
    form->updateTimelinesData(minimumTime,maximumTime,timeline,bandwidth);
}

void DltStatisticPlugin::updateWidget()
{
    form->update();
}

void DltStatisticPlugin::resetStatistics() {

    counterMessages = 0;
    counterVerboseLogs = 0;
    counterVerboseTraces = 0;
    counterNonVerboseControl = 0;

    for(int num=0; num <= QDltMsg::DltLogVerbose; num++) {
        countersVerboseLogs[num] = 0;
    }

    for(int num=0; num <= QDltMsg::DltNetworkTraceMost; num++) {
        countersVerboseTraces[num] = 0;
    }

    for(int num=0; num <= QDltMsg::DltControlTime; num++) {
        countersNonVerboseControl[num] = 0;
    }
}

void DltStatisticPlugin::updateStatistics(int , QDltMsg &msg) {

            if(msg.getMode()==QDltMsg::DltModeVerbose)
            {

                if(msg.getType() == QDltMsg::DltTypeLog)
                {
                    counterVerboseLogs++;

                    if((msg.getSubtype() >= QDltMsg::DltLogOff) && (msg.getSubtype() <= QDltMsg::DltLogVerbose))
                    {
                        countersVerboseLogs[msg.getSubtype()]++;
                    }
                }
                if(msg.getType() == QDltMsg::DltTypeNwTrace)
                {
                    counterVerboseTraces++;

                    if((msg.getSubtype() >= QDltMsg::DltNetworkTraceIpc) && (msg.getSubtype() <= QDltMsg::DltNetworkTraceMost))
                    {
                        countersVerboseTraces[msg.getSubtype()]++;
                    }
                }
            }
            if(msg.getMode() == QDltMsg::DltModeNonVerbose )
            {
                if(msg.getType() == QDltMsg::DltTypeControl )
                {
                    counterNonVerboseControl++;

                    if((msg.getSubtype() >= QDltMsg::DltControlRequest) && (msg.getSubtype() <= QDltMsg::DltControlTime))
                    {
                        countersNonVerboseControl[msg.getSubtype()]++;
                    }
                }

            }
}

void DltStatisticPlugin::printStatistics() {
    QString text;

    text = QString("<html><body>");

    text += QString("<h3>Total messages: %1</h3>").arg(counterMessages);

    text += QString("<h3>Verbose log</h3>");

    text += QString("<table border=\"1\" cellspacing=\"0\" cellheader=\"0\">");
    text += QString("<tr><th>Total</th><th>Log</th><th>NwTrace</th></tr>");
    text += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(counterVerboseLogs+counterVerboseTraces).arg(counterVerboseLogs).arg(counterVerboseTraces);
    text += QString("</table><br>");

    text += QString("<table border=\"1\" cellspacing=\"0\" cellheader=\"0\">");
    text += QString("<tr>");
    for(int num=1;num<=QDltMsg::DltLogVerbose;num++) {
        text += QString("<th>%1</th>").arg(qDltLogInfo[num]);
    }
    text += QString("</tr>");
    text += QString("<tr>");
    for(int num=1;num<=QDltMsg::DltLogVerbose;num++) {
        text += QString("<td>%1</td>").arg(countersVerboseLogs[num]);
    }
    text += QString("</tr>");
    text += QString("</table><br>");

    text += QString("<table border=\"1\" cellspacing=\"0\" cellheader=\"0\">");
    text += QString("<tr>");
    for(int num=1;num<=QDltMsg::DltNetworkTraceMost;num++) {
        text += QString("<th>%1</th>").arg(qDltNwTraceType[num]);
    }
    text += QString("</tr>");
    text += QString("<tr>");
    for(int num=1;num<=QDltMsg::DltNetworkTraceMost;num++) {
        text += QString("<td>%1</td>").arg(countersVerboseTraces[num]);
    }
    text += QString("</tr>");
    text += QString("</table>");

    text += QString("<h3>Control</h3>");

    text += QString("<table border=\"1\" cellspacing=\"0\" cellheader=\"0\">");
    text += QString("<tr>");
    text += QString("<th>Total</th>");
    for(int num=1;num<=QDltMsg::DltControlTime;num++) {
        text += QString("<th>%1</th>").arg(qDltControlType[num]);
    }
    text += QString("</tr>");
    text += QString("<tr>");
    text += QString("<td>%1</td>").arg(counterNonVerboseControl);
    for(int num=1;num<=QDltMsg::DltControlTime;num++) {
        text += QString("<td>%1</td>").arg(countersNonVerboseControl[num]);
    }
    text += QString("</tr>");
    text += QString("</table>");

    text += QString("</body></html>");

    form->setTextBrowserOverview(text);
}

QString DltStatisticPlugin::stringToHtml(QString str)
{
    str = str.replace("<","&#60;");
    str = str.replace(">","&#62;");

    return str;
}

void DltStatisticPlugin::cursorTime1Changed(time_t time)
{
    if(!dltControl)
        return;
    if(!dltFile)
        return;

    QDltMsg msg;
    int numbers = dltFile->size();
    for(int num=0;num<numbers;num++)
    {
        dltFile->getMsg(num, msg);
        if(msg.getTime()>=time)
        {
            dltControl->jumpToMsg(num);
            return;
        }
    }
    dltControl->jumpToMsg(dltFile->size()-1);
}

void DltStatisticPlugin::cursorTime2Changed(time_t time)
{
    cursorTime1Changed(time);
}


#ifndef QT5
Q_EXPORT_PLUGIN2(dltstatisticplugin, DltStatisticPlugin);
#endif

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
 * \file dummycontrolplugin.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtGui>

#include "dlttestrobotplugin.h"

DltTestRobotPlugin::DltTestRobotPlugin()
{
    form = NULL;
    counterMessages = 0;
    counterNonVerboseMessages = 0;
    counterVerboseMessages = 0;
    dltFile = 0;
    dltControl = 0;
    ecuList = 0;

    tcpSocket = 0;

    port = 4490;
}

DltTestRobotPlugin::~DltTestRobotPlugin()
{

}

QString DltTestRobotPlugin::name()
{
    return QString("DLT Test Robot Plugin");
}

QString DltTestRobotPlugin::pluginVersion(){
    return DLT_TEST_ROBOT_PLUGIN_VERSION;
}

QString DltTestRobotPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DltTestRobotPlugin::description()
{
    return QString();
}

QString DltTestRobotPlugin::error()
{
    return QString();
}

bool DltTestRobotPlugin::loadConfig(QString /* filename */)
{
    return true;
}

bool DltTestRobotPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList DltTestRobotPlugin::infoConfig()
{
    return QStringList();
}

QWidget* DltTestRobotPlugin::initViewer()
{
    form = new DltTestRobot::Form(this);

    start();

    return form;
}

bool DltTestRobotPlugin::initControl(QDltControl *control)
{
    dltControl = control;

    return true;
}

bool DltTestRobotPlugin::initConnections(QStringList list)
{
    ecuList = new QStringList(list);

    return false;
}

bool DltTestRobotPlugin::controlMsg(int , QDltMsg &)
{
    return false;
}

bool DltTestRobotPlugin::stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname){

    qDebug() << ecuList->at(index) << "ConnectionState:" << connectionState << "Hostname:" << hostname << endl;

    return true;
}

bool DltTestRobotPlugin::autoscrollStateChanged(bool enabled)
{

    return true;
}

void DltTestRobotPlugin::initMessageDecoder(QDltMessageDecoder* pMessageDecoder)
{
    Q_UNUSED(pMessageDecoder);
}

void DltTestRobotPlugin::initMainTableView(QTableView* pTableView)
{
    Q_UNUSED(pTableView);
}

void DltTestRobotPlugin::configurationChanged()
{}

void DltTestRobotPlugin::selectedIdxMsg(int , QDltMsg &) {

}

void DltTestRobotPlugin::selectedIdxMsgDecoded(int , QDltMsg &){

}

void DltTestRobotPlugin::initFileStart(QDltFile *file){
    dltFile = file;
}

void DltTestRobotPlugin::initMsg(int , QDltMsg &){

}
void DltTestRobotPlugin::initMsgDecoded(int , QDltMsg &){

}

void DltTestRobotPlugin::initFileFinish(){

}

void DltTestRobotPlugin::updateFileStart(){

}

void DltTestRobotPlugin::updateMsg(int , QDltMsg &){

}

void DltTestRobotPlugin::updateMsgDecoded(int , QDltMsg &msg)
{
    for(int num=0; num<filterEcuId.size(); num++)
    {
        if(filterEcuId[num]==msg.getEcuid() && filterAppId[num]==msg.getApid() && filterCtxId[num]==msg.getCtid() && tcpSocket)
        {
            QString text = msg.getEcuid() + " " + msg.getApid() + " " + msg.getCtid() + " " + msg.toStringPayload();
            qDebug() << "DltTestRobot: send message" << text;
            text += "\n";
            tcpSocket->write(text.toLatin1());
        }
    }

}
void DltTestRobotPlugin::updateFileFinish(){

}

void DltTestRobotPlugin::readyRead()
{
    // data on was received
    while (tcpSocket && tcpSocket->canReadLine())
    {
        QString text = QString(tcpSocket->readLine());

        if(text.size()>0)
        {
            text.chop(1);

            // line is not empty
            qDebug() << "DltTestRobot: readLine" << text;

            QStringList list = text.split(' ');

            if(list[0]=="injection" && ecuList)
            {
                QString ecuId = list[1];
                for(int num=0;num<ecuList->length();num++)
                {
                    if(ecuList->at(num).contains(ecuId) && dltControl)
                    {
                        list.removeAt(0);
                        list.removeAt(0);
                        dltControl->sendInjection(num,"DLT","Test",4096,list.join(' ').toLatin1());
                        qDebug() << "DltTestRobot: send injection" << list.join(' ');
                    }
                }

            }
            else if(list[0]=="filter")
            {
                if(list[1]=="clear")
                {
                    filterEcuId.clear();
                    filterAppId.clear();
                    filterCtxId.clear();
                }
                else if(list[1]=="add")
                {
                    filterEcuId.append(list[2]);
                    filterAppId.append(list[3]);
                    filterCtxId.append(list[4]);
                }

            }
        }
    }
}

void DltTestRobotPlugin::newConnection()
{
    tcpSocket = tcpServer.nextPendingConnection();
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    tcpServer.pauseAccepting();

    //readData.clear();
    qDebug() << "DltTestRobot: connected";

    form->status("connected");

}

void DltTestRobotPlugin::connected()
{

}

void DltTestRobotPlugin::disconnected()
{
    tcpSocket->close();
    disconnect(tcpSocket, SIGNAL(connected()), this, SLOT(connected()));
    disconnect(tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    disconnect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    //delete tcpSocket;
    tcpSocket = 0;
    tcpServer.resumeAccepting();

    qDebug() << "DltTestRobot: disconnected";

    form->status("listening");

}

unsigned int DltTestRobotPlugin::getPort() const
{
    return port;
}

void DltTestRobotPlugin::setPort(unsigned int value)
{
    port = value;
}

void DltTestRobotPlugin::start()
{
    if(tcpServer.isListening())
        return ;

    tcpServer.setMaxPendingConnections(1);
    if(tcpServer.listen(QHostAddress::Any,port)==true)
    {
        connect(&tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));

        form->status("listening");

        qDebug() << "DltTestRobot: listening" << port;
    }
    else
    {

        form->status("error");

        qDebug() << "DltTestRobot: error" << port;
    }

    form->start();
}

void DltTestRobotPlugin::stop()
{
    if(tcpSocket && tcpSocket->isOpen())
    {
        disconnect(tcpSocket, SIGNAL(connected()), this, SLOT(connected()));
        disconnect(tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
        tcpSocket->close();
    }

    disconnect(&tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
    tcpServer.close();

    form->status("stopped");
    qDebug() << "DltTestRobot: stopped";

    form->stop();
}


#ifndef QT5
Q_EXPORT_PLUGIN2(dlttestrobotplugin, DltTestRobotPlugin);
#endif

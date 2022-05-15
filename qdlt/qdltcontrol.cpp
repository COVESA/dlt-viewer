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

QDltControl::QDltControl(QObject *_server)
{
    server = _server;

    connect(this, SIGNAL(sendInjectionSignal(int,QString,QString,int,QByteArray)),
            server, SLOT(sendInjection(int,QString,QString,int,QByteArray)));
    connect(this, SIGNAL(jumpToMsgSignal(int)),
            server, SLOT(jumpToMsgSignal(int)));
    connect(this, SIGNAL(markerSignal()),
            server, SLOT(markerSignal()));
    connect(this, SIGNAL(connectEcuSignal(int)),
            server, SLOT(connectEcuSignal(int)));
    connect(this, SIGNAL(disconnectEcuSignal(int)),
            server, SLOT(disconnectEcuSignal(int)));
    connect(this, SIGNAL(connectAllEcuSignal()),
            server, SLOT(connectAllEcuSignal()));
    connect(this, SIGNAL(disconnectAllEcuSignal()),
            server, SLOT(disconnectAllEcuSignal()));
    connect(this, SIGNAL(openFileSignal(QStringList)),
            server, SLOT(onOpenTriggered(QStringList)));
    connect(this, SIGNAL(newFileSignal(QString)),
            server, SLOT(onNewTriggered(QString)));
    connect(this, SIGNAL(saveAsFileSignal(QString)),
            server, SLOT(onSaveAsTriggered(QString)));
    connect(this, SIGNAL(clearFileSignal()),
            server, SLOT(on_action_menuFile_Clear_triggered()));
    connect(this, SIGNAL(quitDltViewerSignal()),
            server, SLOT(on_action_menuFile_Quit_triggered()));
    connect(this, SIGNAL(reopenFileSignal()),
            server, SLOT(reopenFileSignal()));
}

QDltControl::~QDltControl()
{

}

void QDltControl::sendInjection(int index,QString applicationId,QString contextId,int serviceId,QByteArray data)
{
    emit sendInjectionSignal(index,applicationId,contextId,serviceId,data);
}

void QDltControl::jumpToMsg(int index)
{
    emit jumpToMsgSignal(index);
}

void QDltControl::newFile(QString filename)
{
    emit newFileSignal(filename);
}

void QDltControl::openFile(QStringList filenames)
{
    emit openFileSignal(filenames);
}

void QDltControl::saveAsFile(QString filename)
{
    emit saveAsFileSignal(filename);
}

void QDltControl::clearFile()
{
    emit clearFileSignal();
}

void QDltControl::quitDltViewer()
{
    emit quitDltViewerSignal();
}

void QDltControl::marker()
{
    emit markerSignal();
}

void QDltControl::connectEcu(int index)
{
    emit connectEcuSignal(index);
}

void QDltControl::disconnectEcu(int index)
{
    emit disconnectEcuSignal(index);
}

void QDltControl::connectAllEcu()
{
    emit connectAllEcuSignal();
}

void QDltControl::disconnectAllEcu()
{
    emit disconnectAllEcuSignal();
}

void QDltControl::reopenFile()
{
    emit reopenFileSignal();
}

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
 * \file qdltcontrol.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef QDLT_CONTROL_H
#define QDLT_CONTROL_H

#include <QStringList>
#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
#include <QMutex>
#include <time.h>

#include "export_rules.h"

class QDLT_EXPORT QDltControl : public QObject
{
      Q_OBJECT
public:
    QDltControl(QObject *server);
    ~QDltControl();

    void sendInjection(int index,QString applicationId,QString contextId,int serviceId,QByteArray data);
    void jumpToMsg(int index);

    void newFile(QString filename);
    void openFile(QStringList filenames);
    void saveAsFile(QString filename);
    void reopenFile();
    void clearFile();
    void quitDltViewer();
    void marker();
    void connectEcu(int index);
    void disconnectEcu(int index);
    void connectAllEcu();
    void disconnectAllEcu();
    bool silentmode;
    bool commandlinemode;

signals:
    void sendInjectionSignal(int index,QString applicationId,QString contextId,int serviceId,QByteArray data);
    void jumpToMsgSignal(int index);
    void newFileSignal(QString filename);
    void openFileSignal(QStringList filenames);
    void saveAsFileSignal(QString filename);
    void clearFileSignal();
    void quitDltViewerSignal();
    void markerSignal();
    void connectEcuSignal(int index);
    void disconnectEcuSignal(int index);
    void connectAllEcuSignal();
    void disconnectAllEcuSignal();
    void reopenFileSignal();

protected:
private:
    QObject *server;

};


#endif // QDLT_CONTROL_H

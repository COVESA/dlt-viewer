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
 * \file dummycontrolplugin.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef DUMMYCONTROLPLUGIN_H
#define DUMMYCONTROLPLUGIN_H

#include <QObject>
#include "plugininterface.h"
#include "form.h"

#define DUMMY_CONTROL_PLUGIN_VERSION "1.0.0"

class DummyControlPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface, QDltPluginControlInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
    Q_INTERFACES(QDltPluginControlInterface)
#ifdef QT5
    Q_PLUGIN_METADATA(IID "org.genivi.DLT.DummyControlPlugin")
#endif

public:
    DummyControlPlugin();
    ~DummyControlPlugin();

    /* QDLTPluginInterface interface */
    QString name();
    QString pluginVersion();
    QString pluginInterfaceVersion();
    QString description();
    QString error();
    bool loadConfig(QString filename);
    bool saveConfig(QString filename);
    QStringList infoConfig();

    /* QDltPluginViewerInterface */
    QWidget* initViewer();
    void initFileStart(QDltFile *file);
    void initFileFinish();
    void initMsg(int index, QDltMsg &msg);
    void initMsgDecoded(int index, QDltMsg &msg);
    void updateFileStart();
    void updateMsg(int index, QDltMsg &msg);
    void updateMsgDecoded(int index, QDltMsg &msg);
    void updateFileFinish();
    void selectedIdxMsg(int index, QDltMsg &msg);
    void selectedIdxMsgDecoded(int index, QDltMsg &msg);

    /* QDltPluginControlInterface */
    bool initControl(QDltControl *control);
    bool initConnections(QStringList list);
    bool controlMsg(int index, QDltMsg &msg);
    bool stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname);
    bool autoscrollStateChanged(bool enabled);
    void initMessageDecoder(QDltMessageDecoder* pMessageDecoder);
    void initMainTableView(QTableView* pTableView);
    void configurationChanged();

    /* internal variables */
    DummyControl::Form *form;
    int counterMessages;
    int counterNonVerboseMessages;
    int counterVerboseMessages;

    void show(bool value);

    void updateCounters(int start,int end);

    QDltControl *dltControl;

private:
    QDltFile *dltFile;
    QString errorText;
    QStringList *ecuList;
};

#endif // DUMMYCONTROLPLUGIN_H

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
 * \file dummyviewerplugin.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef DLTSTATISTICPLUGIN_H
#define DLTSTATISTICPLUGIN_H

#include <QObject>
#include <QMap>

#include "plugininterface.h"
#include "form.h"

#define DLT_STATISTIC_PLUGIN_VERSION "1.0.0"

class DltStatisticPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface, QDltPluginControlInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
    Q_INTERFACES(QDltPluginControlInterface)
#ifdef QT5
    Q_PLUGIN_METADATA(IID "org.genivi.DLT.DltStatisticPlugin")
#endif

public:
    DltStatisticPlugin();
    ~DltStatisticPlugin();

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
    bool stateChanged(int index, QDltConnection::QDltConnectionState connectionState);
    bool autoscrollStateChanged(bool enabled);

    // helper functions
    void show(bool value);
    void updateCounters(int index, QDltMsg &msg);
    void clear();
    void updateData();
    void updateWidget();

    // statistic functions
    void resetStatistics();
    void updateStatistics(int index, QDltMsg &msg);
    void printStatistics();

public slots:

    void cursorTime1Changed(time_t time);
    void cursorTime2Changed(time_t time);

private:

    // add message to timeline
    void addToTimeline(int index,QDltMsg &msg);

    // helper function
    QString stringToHtml(QString str);

    // internal variables
    Form *form;
    QDltFile *dltFile;
    QString errorText;
    QDltControl *dltControl;

    // timeline and bandwidth parameters
    time_t minimumTime,maximumTime;
    QMap<time_t,int> timeline;
    QMap<time_t,int> bandwidth;

    // Statistics
    int counterMessages;
    int counterVerboseLogs;
    int counterVerboseTraces;
    int countersVerboseLogs[QDltMsg::DltLogVerbose+1];
    int countersVerboseTraces[QDltMsg::DltNetworkTraceMost+1];
    int counterNonVerboseControl;
    int countersNonVerboseControl[QDltMsg::DltControlTime+1];

};

#endif // DLTSTATISTICPLUGIN_H

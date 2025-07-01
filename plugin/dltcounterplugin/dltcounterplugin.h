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
 * \file dltcounterplugin.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef DLTCOUNTERPLUGIN_H
#define DLTCOUNTERPLUGIN_H

#include <QObject>
#include "plugininterface.h"
#include "form.h"

#define DLT_COUNTER_PLUGIN_VERSION "1.0.0"

class DltCounterPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface, QDltPluginControlInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
    Q_INTERFACES(QDltPluginControlInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.genivi.DLT.DltCounterPlugin")
#endif

public:
    DltCounterPlugin();
    ~DltCounterPlugin();


    QMap<QString, QSet<int>> consolidatedMap;
    void dataConsolidatedMap();
    QList<QString> nameList;
    QList<unsigned char> countList;

    //For clearing the list and map before initMsg, so that old data will not
    //carried forward for next log file.
    void clearAll() {
        nameList.clear();
        countList.clear();
        consolidatedMap.clear();
    }

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
    void initMsg(int, QDltMsg &msg);
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
    DltCounter::Form *form;
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

#endif // DLTCOUNTERPLUGIN_H

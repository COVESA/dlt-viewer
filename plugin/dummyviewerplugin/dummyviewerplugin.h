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
 * \file dummyviewerplugin.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef DUMMYVIEWERPLUGIN_H
#define DUMMYVIEWERPLUGIN_H

#include <QObject>
#include "plugininterface.h"
#include "form.h"

#define DUMMY_VIEWER_PLUGIN_VERSION "1.0.0"

class DummyViewerPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
#ifdef QT5
    Q_PLUGIN_METADATA(IID "org.genivi.DLT.DummyViewerPlugin")
#endif

public:
    DummyViewerPlugin();
    ~DummyViewerPlugin();

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


    /* internal variables */
    DummyViewer::Form *form;
    int counterMessages;
    int counterNonVerboseMessages;
    int counterVerboseMessages;

    void show(bool value);
    void updateCounters(int index, QDltMsg &msg);

private:
    QDltFile *dltFile;
    QString errorText;
};

#endif // DUMMYVIEWERPLUGIN_H

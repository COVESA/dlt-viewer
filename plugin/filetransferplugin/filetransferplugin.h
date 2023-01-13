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
 * \file filetransferplugin.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef DLTVIEWERPLUGIN_H
#define DLTVIEWERPLUGIN_H

#include <QObject>
#include "plugininterface.h"
#include "form.h"
#include "globals.h"
#include "configuration.h"

#define FILETRANSFER_PLUGIN_VERSION "1.4.1"

class FiletransferPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface, QDltPluginCommandInterface, QDltPluginControlInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
    Q_INTERFACES(QDltPluginCommandInterface)
    Q_INTERFACES(QDltPluginControlInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.genivi.DLT.FileTransferPlugin")
#endif

public:
    FiletransferPlugin();
    ~FiletransferPlugin();

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

    void updateFiletransfer(int index, QDltMsg &msg);
    void show(bool value);

    /* QDltPluginCommandInterface */
    bool command(QString command, QList<QString> params);
    bool exportAll(QDir extract_dir);

    /* QDltPluginControlInterface */
    bool initControl(QDltControl *control);
    bool initConnections(QStringList list);
    bool controlMsg(int index, QDltMsg &msg);
    bool stateChanged(int index, QDltConnection::QDltConnectionState connectionState, QString hostname);
    bool autoscrollStateChanged(bool enabled);
    void initMessageDecoder(QDltMessageDecoder* pMessageDecoder);
    void initMainTableView(QTableView* pTableView);
    void configurationChanged();


private:
    QString plugin_name_displayed = QString("Filetransfer Plugin");
    FileTransferPlugin::Form *form;
    QDltFile *dltFile;
    QDltControl *dltControl;
    QString errorText;
    bool plugin_is_active = false;
    bool ft_autosave = false;

    void doFLST(QDltMsg *msg); // file transfer start
    void doFLDA(int index, QDltMsg *msg); // file transfer update
    void doFLIF(QDltMsg *msg);
    void doFLER(QDltMsg *msg); // file transfer error handling

    Configuration config;


};

#endif // DLTVIEWERPLUGIN_H

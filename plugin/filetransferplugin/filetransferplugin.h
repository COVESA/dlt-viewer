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
 * \file filetransferplugin.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef DLTVIEWERPLUGIN_H
#define DLTVIEWERPLUGIN_H

#include <QObject>
#include "plugininterface.h"
#include "form.h"
#include "globals.h"
#include "configuration.h"

#define FILETRANSFER_PLUGIN_VERSION "1.0.0"

class FiletransferPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface, QDltPluginCommandInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
    Q_INTERFACES(QDltPluginCommandInterface)

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

private:
    Form *form;
    QDltFile *dltFile;
    QString errorText;

    void doFLST(QDltMsg *msg);
    void doFLDA(int index, QDltMsg *msg);
    void doFLFI(QDltMsg *msg);
    void doFLIF(QDltMsg *msg);
    void doFLER(QDltMsg *msg);

    Configuration config;
};

#endif // DLTVIEWERPLUGIN_H

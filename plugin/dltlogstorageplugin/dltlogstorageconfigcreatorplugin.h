/**
 * @licence app begin@
 * Copyright (C) 2015 Advanced Driver Information Technology
 *
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch and DENSO.
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \author Christoph Lipka <clipka@jp.adit-jv.com> ADIT 2015
 *
 * \file dltlogstorageconfigcreator.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef DLTLOGSTORAGECONFIGCREATORPLUGIN_H
#define DLTLOGSTORAGECONFIGCREATORPLUGIN_H

#include <QObject>
#include "plugininterface.h"
#include "logstorageconfigcreatorform.h"

#define DLT_LOGSTORAGE_CONFIG_CREATOR_PLUGIN_VERSION "0.0.1"

class DltLogstorageConfigCreatorPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.genivi.DLT.DltLogstorageConfigCreatorPlugin")
#endif

public:
    DltLogstorageConfigCreatorPlugin();
    ~DltLogstorageConfigCreatorPlugin();

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
    LogstorageConfigCreatorForm *ui;
};

#endif // DLTLOGSTORAGECONFIGCREATORPLUGIN_H

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
 * \file dummycontrolplugin.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef DUMMYCONTROLPLUGIN_H
#define DUMMYCONTROLPLUGIN_H

#include <QObject>
#include "plugininterface.h"
#include "dummywaitthread.h"

#define DUMMY_COMMAND_PLUGIN_VERSION "1.0.0"

class DummyCommandPlugin : public QObject, QDLTPluginInterface, QDltPluginCommandInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginCommandInterface)

public:
    DummyCommandPlugin();
    ~DummyCommandPlugin();

    /* QDLTPluginInterface interface */
    QString name();
    QString pluginVersion();
    QString pluginInterfaceVersion();
    QString description();
    QString error();
    bool loadConfig(QString filename);
    bool saveConfig(QString filename);
    QStringList infoConfig();

    /* QDltPluginCommandInterface */
    bool command(QString command, QList<QString> params);
    void cancel();
    QString commandReturnValue();
    int commandProgress();
    QList<QString> commandList();

    /* The wait thread */
    DummyWaitThread *thread;
};

#endif // DUMMYCONTROLPLUGIN_H

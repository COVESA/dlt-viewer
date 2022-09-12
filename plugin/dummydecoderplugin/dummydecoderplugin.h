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
 * \file dummydecoderplugin.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef DUMMYDECODERPLUGIN_H
#define DUMMYDECODERPLUGIN_H

#include <QObject>
#include "plugininterface.h"

#define DUMMY_DECODER_PLUGIN_VERSION "1.0.0"

class DummyDecoderPlugin : public QObject, QDLTPluginInterface, QDLTPluginDecoderInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDLTPluginDecoderInterface)
#ifdef QT5
    Q_PLUGIN_METADATA(IID "org.genivi.DLT.DummyDecoderPlugin")
#endif

public:
    DummyDecoderPlugin();
    ~DummyDecoderPlugin();

    /* QDLTPluginInterface interface */
    QString name();
    QString pluginVersion();
    QString pluginInterfaceVersion();
    QString description();
    QString error();
    bool loadConfig(QString filename);
    bool saveConfig(QString filename);
    QStringList infoConfig();

    /* QDltPluginDecoderInterface */
    bool isMsg(QDltMsg &msg, int triggeredByUser);
    bool decodeMsg(QDltMsg &msg, int triggeredByUser);

private:
    QString errorText;
};

#endif // DUMMYDECODERPLUGIN_H

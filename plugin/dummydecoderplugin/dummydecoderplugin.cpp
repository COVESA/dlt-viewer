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
 * \file dummydecoderplugin.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QtGui>

#include "dummydecoderplugin.h"

DummyDecoderPlugin::DummyDecoderPlugin()
{
}

DummyDecoderPlugin::~DummyDecoderPlugin()
{

}

QString DummyDecoderPlugin::name()
{
    return QString("Dummy Decoder Plugin");
}

QString DummyDecoderPlugin::pluginVersion(){
    return DUMMY_DECODER_PLUGIN_VERSION;
}

QString DummyDecoderPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DummyDecoderPlugin::description()
{
    return QString();
}

QString DummyDecoderPlugin::error()
{
    return QString();
}

bool DummyDecoderPlugin::loadConfig(QString /* filename */ )
{
    return true;
}

bool DummyDecoderPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList DummyDecoderPlugin::infoConfig()
{
    return QStringList();
}

bool DummyDecoderPlugin::isMsg(QDltMsg & msg, int triggeredByUser)
{
    Q_UNUSED(msg);
    Q_UNUSED(triggeredByUser);

    return false;
}

bool DummyDecoderPlugin::decodeMsg(QDltMsg &msg, int triggeredByUser)
{
    Q_UNUSED(msg);
    Q_UNUSED(triggeredByUser);

    return false;
}

#ifndef QT5
Q_EXPORT_PLUGIN2(dummydecoderplugin, DummyDecoderPlugin);
#endif


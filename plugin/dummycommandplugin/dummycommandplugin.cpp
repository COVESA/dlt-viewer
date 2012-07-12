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
 * \file dummycontrolplugin.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtGui>

#include "dummycommandplugin.h"
#include <QtConcurrentRun>
#include <QThread>

DummyCommandPlugin::DummyCommandPlugin()
{
    thread = new DummyWaitThread();
}

DummyCommandPlugin::~DummyCommandPlugin()
{

}

QString DummyCommandPlugin::name()
{
    return QString("Dummy Command Plugin");
}

QString DummyCommandPlugin::pluginVersion(){
    return DUMMY_COMMAND_PLUGIN_VERSION;
}

QString DummyCommandPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DummyCommandPlugin::description()
{
    return QString("Description of Dummy Command Plugin");
}

QString DummyCommandPlugin::error()
{
    return QString("Error from Dummy Command Plugin");
}

bool DummyCommandPlugin::loadConfig(QString /* filename */)
{
    return true;
}

bool DummyCommandPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList DummyCommandPlugin::infoConfig()
{
    return QStringList("Config info from Dummy Command Plugin");
}

bool DummyCommandPlugin::command(QString command, QList<QString> params)
{
    thread->reset();
    thread->start();

    QFile dst("DummyCommandPlugin.txt");
    QDataStream sout;
    if(command == "append")
    {
        if(!dst.open(QIODevice::WriteOnly|QIODevice::Append))
        {
            return false;
        }
    }
    else if(command == "overwrite")
    {
        if(!dst.open(QIODevice::WriteOnly|QIODevice::Truncate))
        {
            return false;
        }
    }

    sout.setDevice(&dst);

    for(int i=0;i < params.size();i++)
    {
        QString line(params[i] + QString("\n"));
        sout << line;
    }
    dst.close();
    return true;
}

void DummyCommandPlugin::cancel()
{
    thread->exit();
}

QString DummyCommandPlugin::commandReturnValue()
{
    return QString("Dummy Command Plugin return value");
}

int DummyCommandPlugin::commandProgress()
{
    return thread->getProgress();
}

QList<QString> DummyCommandPlugin::commandList()
{
    QList<QString> ret;
    ret.append(QString("append"));
    ret.append(QString("overwrite"));
    return ret;
}

Q_EXPORT_PLUGIN2(dummycommandplugin, DummyCommandPlugin);

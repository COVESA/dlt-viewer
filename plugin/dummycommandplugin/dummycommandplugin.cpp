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
 * \file dummycontrolplugin.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QtGui>
#include <QApplication>

#include "dummycommandplugin.h"

DummyCommandPlugin::DummyCommandPlugin()
{
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
    for(int i=0;i<100;i++)
    {
        qDebug() << "Doing nothing: " << i << "%";
        QApplication::processEvents();
    }
    dst.close();
    return true;
}

#ifndef QT5
Q_EXPORT_PLUGIN2(dummycommandplugin, DummyCommandPlugin)
#endif


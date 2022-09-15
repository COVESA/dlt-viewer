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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> BMW 2011,2012
 *         Gernot Wirschal  <Gernot.Wirschal@bmw.de> BMW 2018
 *
 * \file speedplugin.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

/*
 *  Last build with QT5.8, QWT 6.1.3 for Windows
*/

#include <QtGui>
#include "speedplugin.h"

SpeedPlugin::SpeedPlugin()
{
    //qDebug() << "SpeedPlugin::SpeedPlugin" << __LINE__ << __FILE__;
    form = NULL;
    dltFile = NULL;
    msgIndex = 0;
}

SpeedPlugin::~SpeedPlugin()
{
}

QString SpeedPlugin::name()
{
    return QString("Speed Plugin");
}

/*
The plugin has to return a version number in the format X.Y.Z.
• X should count up in case of really heavy changes (API changes or purpose changes)
• Y should count up when the module is reworked internally, functions are added etc
• Z should count up whenever a bug is fixed
Recommendation: #define <plugin name>_PLUGIN_VERSION "X.Y.Z" in your plugin header
file.*/
QString SpeedPlugin::pluginVersion()
{
    //qDebug() << "SpeedPlugin::pluginVersion" << __LINE__ << __FILE__;
    return SPEEDPLUGINVERSION;
}

/* The plugin has to return a version number of the used plugin interface. The plugin interface provides
the PLUGIN_INTERFACE_VERSION definition for this purpose.*/
QString SpeedPlugin::pluginInterfaceVersion()
{
    return PLUGIN_INTERFACE_VERSION;
}

QString SpeedPlugin::description()
{
    return QString("Shows up something");
}


/* The plugin can provide an error message for the last failed function call. In some cases the DLT Viewer
can display this message to the user. You can also just return an empty QString if you don’t care about
this.*/
QString SpeedPlugin::error()
{
    return QString();
}

/* The plugin can use configuration stored in files. This can be a single file or a directory containing several
files. This function is called whenever a new file gets loaded.*/
bool SpeedPlugin::loadConfig(QString filename)
{
    Q_UNUSED(filename);
    //qDebug() << "SpeedPlugin::loadConfig" << __LINE__ << __FILE__;
    return true;
}



/* Currently not used by DLT Viewer.*/
bool SpeedPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList SpeedPlugin::infoConfig()
{
    return QStringList();
}


/* Called when loading the plugin. The created widget must be returned.*/
QWidget* SpeedPlugin::initViewer()
{
    //qDebug() << "SpeedPlugin::initViewer" << __LINE__ << __FILE__;
    form = new Form();
    return form;
}


/*
An undecoded log message was selected in the message table. The Viewer plugin can now show additional
information about this message.
*/
void SpeedPlugin::selectedIdxMsg(int index, QDltMsg &msg)
{
  Q_UNUSED(index);
  //qDebug() << "SpeedPlugin::selectedIdxMsg" << __LINE__ << __FILE__;
  QDltArgument argument;
  if( (msg.getApid().compare("SPEE") == 0) && (msg.getCtid().compare("SIG") == 0))
      {
          if(msg.getArgument(1,argument))
          {
           form->setSpeedLCD(argument,msg.getTimestamp());
          }
      }
}


/*
A decoded log message was selected in the message table. The Viewer plugin can now show additional
information about this message.
*/
void SpeedPlugin::selectedIdxMsgDecoded(int , QDltMsg &/*msg*/)
{

}


/*
This function is called every time a new log file is opened by the Viewer, or a new log file is created, and
before any messages are processed with initMsg() and initMsgDecoded().
*/
void SpeedPlugin::initFileStart(QDltFile *file)
{
    //qDebug() << "SpeedPlugin::initFileStart with " << file->getFileName() << __LINE__ << __FILE__;
    dltFile = file;
}

/*
This function is called every time a new undecoded message is being processed when loading or creating
a new log file.
*/
void SpeedPlugin::initMsg(int index, QDltMsg &msg)
{
 Q_UNUSED(msg);
 Q_UNUSED(index);
 //qDebug() << "SpeedPlugin::initMsg" << __LINE__ << __FILE__;
}


/* This function is called every time a new decoded message is being processed when loading or creating a
new log file.*/
void SpeedPlugin::initMsgDecoded(int , QDltMsg &)
{
//empty. Implemented because derived plugin interface functions are virtual.
}


/*
This function is called after a log file was opened by the Viewer, or a new log file was created, and after
all messages were processed with initMsg() and initMsgDecoded().
*/
void SpeedPlugin::initFileFinish()
{
  //qDebug() << "SpeedPlugin::initFileFinish" << __LINE__ << __FILE__;
    if(NULL == dltFile)
    {
        qDebug() << "no dltFile in" << __LINE__ << __FILE__;
        return;
    }

QByteArray buffer;
QDltMsg msg;
QDltArgument argument;
for(;msgIndex<dltFile->size();msgIndex++)
{

   buffer =  dltFile->getMsg(msgIndex);

   if(buffer.isEmpty())
       break;

   msg.setMsg(buffer);

    if( (msg.getApid().compare("SPEE") == 0) && (msg.getCtid().compare("SIG") == 0))
    {
        if(msg.getArgument(1,argument))
        {
          form->setSpeedLCD(argument,msg.getTimestamp());
          form->setSpeedGraph(argument,msg.getTimestamp());
        }

    }
 }
}


/* This function is called every time a new undecoded message was received by the Viewer. */
void SpeedPlugin::updateMsg(int index, QDltMsg &msg)
{
    Q_UNUSED(index);
    //qDebug() << "SpeedPlugin::updateMsg" << __LINE__ << __FILE__<< index;
    
    QDltArgument argument;

    if( (msg.getApid().compare("SPEE") == 0) && (msg.getCtid().compare("SIG") == 0))
        {
            if(true == msg.getArgument(1,argument))
            {
                //qDebug() << msg.getArgument(1,argument) << argument.toString();
                form->setSpeedLCD(argument,msg.getTimestamp());
                form->setSpeedGraph(argument,msg.getTimestamp());
                /*
                // just some example for seinding a message injection
                QString value = argument.toString();
                if (value.toInt() == 20)
                {
                    QString data = QString("%1").arg(index);
                    qDebug() << "Send injection with" << data << "at" << value;
                    dltControl->sendInjection(0,"SPEE","SIG",4098,data.toLatin1());
                }
                */
           }
        }
}


/* This function is called every time a new message was received by the Viewer and before the message is
processed with updateMsg() and updateMsgDecoded().*/
void SpeedPlugin::updateFileStart()
{
 //qDebug() << "updateFileStart" <<__LINE__ << __FILE__;
}

/* This function is called every time a new decoded message was received by the Viewer.*/
void SpeedPlugin::updateMsgDecoded(int , QDltMsg &)
{
//empty. Implemented because derived plugin interface functions are virtual.
}


/* This function is called every time a new message was received by the Viewer and after the message was
processed with updateMsg() and updateMsgDecoded().*/
void SpeedPlugin::updateFileFinish()
{
  //qDebug() << "SpeedPlugin::updateFileFinish" <<__LINE__ << __FILE__;
}

/* QDltPluginControlInterface */
bool SpeedPlugin::initControl(QDltControl *control)
{
   dltControl = control;
return true;
}

bool SpeedPlugin::initConnections(QStringList list)
{
return true;
}

bool SpeedPlugin::controlMsg(int index, QDltMsg &msg)
{
Q_UNUSED(index);
Q_UNUSED(msg);
return true;
}

bool SpeedPlugin::stateChanged(int index, QDltConnection::QDltConnectionState connectionState, QString hostname)
{
Q_UNUSED(index);
Q_UNUSED(connectionState);
return true;
}

bool SpeedPlugin::autoscrollStateChanged(bool enabled)
{
Q_UNUSED(enabled);
return true;
}

void SpeedPlugin::initMessageDecoder(QDltMessageDecoder* pMessageDecoder)
{
Q_UNUSED(pMessageDecoder);
}

void SpeedPlugin::initMainTableView(QTableView* pTableView)
{
 Q_UNUSED(pTableView);
}

void SpeedPlugin::configurationChanged()
{

}

#ifndef QT5
Q_EXPORT_PLUGIN2(speedplugin, SpeedPlugin);
#endif

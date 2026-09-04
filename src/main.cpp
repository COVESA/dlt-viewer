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
 * \file main.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QModelIndex>
#include <QApplication>
#include <QStyleFactory>
#include <QThread>
#include <QFile>
#include <QDateTime>
#include <QDebug>

#include <qdltoptmanager.h>

#include "mainwindow.h"

/* Custom message handler: writes qDebug to a log file for debugging */
static QFile s_logFile;
void debugMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (s_logFile.isOpen()) {
        QString txt = QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + " ";
        switch (type) {
            case QtDebugMsg:    txt += "DBG "; break;
            case QtInfoMsg:     txt += "INF "; break;
            case QtWarningMsg:  txt += "WRN "; break;
            case QtCriticalMsg: txt += "CRT "; break;
            case QtFatalMsg:    txt += "FTL "; break;
        }
        txt += msg + "\n";
        s_logFile.write(txt.toUtf8());
        s_logFile.flush();
    }
}

int main(int argc, char *argv[])
{

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    // check if silent mode or help is requested
    // if yes, activate offscreen mode to be able to run also without display
    for(int i=0;i<argc;i++)
    {
        if(strcmp(argv[i],"-s")==0 || strcmp(argv[i],"--silent")==0 || strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0)
        {
            qputenv("QT_QPA_PLATFORM","offscreen");
            break;
        }
    }

    QApplication a(argc, argv);
    // Keep the UI responsive under heavy background processing.
    QThread::currentThread()->setPriority(QThread::HighestPriority);
    QDltOptManager::getInstance()->parse(a.arguments());

    /* Install debug log file handler (only with --debug-log flag) */
    if (a.arguments().contains("--debug-log")) {
        QString logPath = QCoreApplication::applicationDirPath() + "/debug_tcp_server.log";
        s_logFile.setFileName(logPath);
        if (s_logFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qInstallMessageHandler(debugMessageHandler);
            qDebug() << "Debug logging started ->" << logPath;
        }
    }

    MainWindow w;
    w.show();

    return a.exec();
}

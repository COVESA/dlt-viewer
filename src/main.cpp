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
 * \file main.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QModelIndex>
#include <QApplication>
#include <QStyleFactory>

#include <qdlt.h>

#include "mainwindow.h"

int main(int argc, char *argv[])
{

#ifdef Q_OS_WIN
    QSettings themeSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",QSettings::NativeFormat);
    if(themeSettings.value("AppsUseLightTheme")==0){
        qApp->setStyle(QStyleFactory::create("Fusion"));
        QPalette darkMode;
        QColor foregroundColor = QColor(49,50,53);
        QColor backgroundColor = QColor(31,31,31);
        QColor disabledColor = QColor(127,127,127);
        QColor brightColor = QColor(253,253,255);
        QColor brighterColor = QColor(Qt::white);
        QColor darkColor = QColor(Qt::black);
        QColor highlightColor = QColor(51,144,255);
        darkMode.setColor(QPalette::AlternateBase, foregroundColor);
        darkMode.setColor(QPalette::Base, backgroundColor);
        darkMode.setColor(QPalette::BrightText, brighterColor);
        darkMode.setColor(QPalette::Disabled, QPalette::BrightText, disabledColor);
        darkMode.setColor(QPalette::Button, foregroundColor);
        darkMode.setColor(QPalette::ButtonText, brightColor);
        darkMode.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
        darkMode.setColor(QPalette::Highlight, highlightColor);
        darkMode.setColor(QPalette::HighlightedText, darkColor);
        darkMode.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);
        darkMode.setColor(QPalette::Link, highlightColor);
        darkMode.setColor(QPalette::Text, brightColor);
        darkMode.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
        darkMode.setColor(QPalette::ToolTipBase, foregroundColor);
        darkMode.setColor(QPalette::ToolTipText, brighterColor);
        darkMode.setColor(QPalette::Disabled, QPalette::ToolTipText, disabledColor);
        darkMode.setColor(QPalette::PlaceholderText, brightColor);
        darkMode.setColor(QPalette::Disabled, QPalette::PlaceholderText, disabledColor);
        darkMode.setColor(QPalette::Window, foregroundColor);
        darkMode.setColor(QPalette::WindowText, brightColor);
        darkMode.setColor(QPalette::Disabled, QPalette::WindowText, disabledColor);
        darkMode.setColor(QPalette::Light, disabledColor);
        darkMode.setColor(QPalette::Midlight, disabledColor);
        darkMode.setColor(QPalette::Dark, foregroundColor);
        darkMode.setColor(QPalette::Mid, backgroundColor);
        darkMode.setColor(QPalette::Shadow, darkColor);
        qApp->setPalette(darkMode);
    }
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication a(argc, argv);

    QStringList arguments = a.arguments();
    QDltOptManager *opt = QDltOptManager::getInstance();
    opt->parse(&arguments);

    MainWindow w;
    /* check variable commandline_finished
       instead of stopping during constructor run
       after running a commandline mode call
       which often leads to crash
    */
    if(opt->getInstance()->isConvert() == true )
    {
        return 0;
    }

    w.show();

    return a.exec();
}

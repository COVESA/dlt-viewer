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
#include "mainwindow.h"
#include "optmanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QStringList arguments = a.arguments();
    OptManager *opt = OptManager::getInstance();
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

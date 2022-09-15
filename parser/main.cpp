/**
 * @licence app begin@
 * Copyright (C) 2014  BMW AG
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
 * \author
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \file main.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QApplication>

#include <iostream>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    bool noGui = false;

    QApplication a(argc, argv);

    /* parse arguments */
    QStringList args = a.arguments();
    for(int num=1;num<args.size();num++)
    {
        if(args[num]=="--no-gui")
        {
            noGui=true;
        }
        else if(args[num]=="--help")
        {
            std::cout << commandLineOptions << std::endl;
            return 0;
        }
    }

    MainWindow w(noGui);
    w.noGui = noGui;

    /* parse further arguments */
    for(int num=1;num<args.size();num++)
    {
        if(args[num]=="--no-gui")
        {
        }
        else if(args[num]=="--parse-file")
        {
            w.argParseFile = args[++num];
        }
        else if(args[num]=="--parse-dir")
        {
            w.argParseDir = args[++num];
        }
        else if(args[num]=="--parse-cfg")
        {
            w.argParseCfg = args[++num];
        }
        else if(args[num]=="--converte-file")
        {
            w.argConverteFile = args[++num];
        }
        else if(args[num]=="--converte-dir")
        {
            w.argConverteDir = args[++num];
        }
        else if(args[num]=="--read-fibex")
        {
            w.argReadFibex = args[++num];
        }
        else if(args[num]=="--write-fibex")
        {
            w.argWriteFibex = args[++num];
        }
        else if(args[num]=="--write-csv")
        {
            w.argWriteCsv = args[++num];
        }
        else if(args[num]=="--write-id")
        {
            w.argWriteId = args[++num];
        }
        else if(args[num]=="--write-id-app")
        {
            w.argWriteIdApp = args[++num];
        }
        else if(args[num]=="--update-id")
        {
            w.argUpdateIdStart = args[++num];
            w.argUpdateIdEnd = args[++num];
        }
        else if(args[num]=="--update-id-app")
        {
            w.argUpdateIdAppStart = args[++num];
            w.argUpdateIdAppEnd = args[++num];
        }
        else if(args[num]=="--check-double")
        {
            w.checkDouble = true;
        }
        else if(args[num]=="--check-double-app")
        {
            w.checkDoubleApp = true;
        }
        else
        {
            std::cout <<  "Unknown option: " << (const char*)(args[num].toLatin1()) << std::endl;
            return -1;
        }
    }

    if(!w.noGui)
    {
        /* show main window */
        w.show();
    }

    /* execute arguments if they exist */
    int errorCode = w.argsParser();

    if(w.noGui)
    {
        return errorCode;
    }
    else
    {
        /* run main loop of main window */
        return a.exec();
    }
}

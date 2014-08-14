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
 * \file optmanager.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include "optmanager.h"
#include <QDebug>
// Global static pointer used to ensure a single instance of the class.
OptManager* OptManager::instance;

OptManager::OptManager()
{
    project = false;
    log = false;
    convert = false;
    filter = false;
    plugin = false;
    silent_mode = false;
}

OptManager* OptManager::getInstance()
{
    if (!instance)
        instance = new OptManager;

    return instance;
}

OptManager::OptManager(OptManager const&)
{

}

void OptManager::printUsage()
{
#if (WIN32)
    qDebug()<<"Usage: dlt_viewer.exe [OPTIONS]";
#else
    qDebug()<<"Usage: dlt_viewer [OPTIONS]";
#endif

    qDebug()<<"Options:";
    qDebug()<<" -h \t\tPrint usage";
    qDebug()<<" -p projectfile \tLoading project file on startup (must end with .dlp)";
    qDebug()<<" -l logfile \tLoading logfile on startup (must end with .dlt)";
    qDebug()<<" -f filterfile \tLoading filterfile on startup (must end with .dlf)";
    qDebug()<<" -c logfile textfile \tConvert logfile file to textfile (logfile must end with .dlt)";
    qDebug()<<" -e \"plugin|command|param1|..|param<n>\" \tExecute a plugin command with <n> parameters.";
    qDebug()<<" -s or --silent \tEnable silent mode without warning message boxes.";

}

void OptManager::parse(QStringList *opt){
    QString str;

    if(opt->size()==2)
    {
        if(opt->at(1).endsWith(".dlp") || opt->at(1).endsWith(".DLP")){
            projectFile = QString("%1").arg(opt->at(1));
            project = true;
            return;
        }
        if(opt->at(1).endsWith(".dlt") || opt->at(1).endsWith(".DLT")){
            logFile = QString("%1").arg(opt->at(1));
            log = true;
            return;
        }
    }

    // 0==Binary 1==First Argument
    for (int i = 0; i < opt->size(); ++i){
        str = opt->at(i);

          if(str.compare("-h") == 0 || str.compare("--help") == 0){
            printUsage();
            exit(0);
          }
        if(str.compare("-s") == 0 || str.compare("--silent") == 0){
            silent_mode = true;
          }

        if(str.compare("-p")==0) {
            QString p1 = opt->value(i+1);

            if(p1!=0 && (p1.endsWith(".dlp") || p1.endsWith(".DLP"))){
                projectFile = QString("%1").arg(opt->at(i+1));
                project = true;
              }else{
                qDebug()<<"Error occured during processing of command line option \"-p\"";
                printUsage();
                exit(-1);
              }
          }
        if(str.compare("-l")==0) {
            QString l1 = opt->value(i+1);

            if(l1!=0 && (l1.endsWith(".dlt")||l1.endsWith(".DLT"))){
                logFile = QString("%1").arg(l1);
                log = true;
              }else{
                qDebug()<<"Error occured during processing of command line option \"-l\"";
                printUsage();
                exit(-1);
              }
          }
        if(str.compare("-f")==0) {
            QString f1 = opt->value(i+1);

            if(f1!=0 && (f1.endsWith(".dlf")||f1.endsWith(".DLF"))){
                filterFile = QString("%1").arg(f1);
                filter = true;
              }else{
                qDebug()<<"Error occured during processing of command line option \"-f\"";
                printUsage();
                exit(-1);
              }
          }
        if(str.compare("-c")==0) {

            QString c1 = opt->value(i+1);
            QString c2 = opt->value(i+2);

            if(c1!=0 && (c1.endsWith(".dlt")||c1.endsWith(".DLT")) && c2!=0){
                convertSourceFile = QString("%1").arg(c1);
                convertDestFile = QString("%1").arg(c2);
                convert = true;
              }else{
                qDebug()<<"Error occured during processing of command line option \"-c\"";
                printUsage();
                exit(-1);
              }
          }
        if(str.compare("-e")==0) {
            QString c = opt->value(i+1);
            QStringList args = c.split("|");;
            if(c != 0 && args.size() > 1) {
                pluginName = args.at(0);
                commandName = args.at(1);
                args.removeAt(0);
                args.removeAt(0);
                commandParams = args;
                plugin = true;

              }else{
                qDebug()<<"Error occured during processing of command line option \"-e\"";
                printUsage();
                exit(-1);
              }
          }
      }

}

bool OptManager::isProjectFile(){ return project;}
bool OptManager::isLogFile(){return log;}
bool OptManager::isFilterFile(){return filter;}
bool OptManager::isConvert(){return convert;}
bool OptManager::isPlugin(){return plugin;}
bool OptManager::issilentMode(){return silent_mode;}

QString OptManager::getProjectFile(){return projectFile;}
QString OptManager::getLogFile(){return logFile;}
QString OptManager::getFilterFile(){return filterFile;}
QString OptManager::getConvertSourceFile(){return convertSourceFile;}
QString OptManager::getConvertDestFile(){return convertDestFile;}
QString OptManager::getPluginName(){return pluginName;}
QString OptManager::getCommandName(){return commandName;}
QStringList OptManager::getCommandParams(){return commandParams;}

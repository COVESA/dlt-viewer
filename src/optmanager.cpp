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
}

OptManager* OptManager::getInstance()
{
    if (!instance)
        instance = new OptManager;

    return instance;
}

OptManager::OptManager(OptManager const&){

}

void OptManager::OptManager::printUsage(){
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

    // Please copy changes to mainwindow.cpp - on_actionCommand_Line_triggered()
}

void OptManager::parse(QStringList *opt){
    QString str;

    if(opt->size()==2)
    {
//        qDebug() << QString(" [%1]").arg(opt->at(1));
        if(opt->at(1).endsWith(".dlp")){
            projectFile = QString("%1").arg(opt->at(1));
            project = true;
        }
        if(opt->at(1).endsWith(".dlt")){
            logFile = QString("%1").arg(opt->at(1));
            log = true;
        }
    }
    else
    {
        // 0==Binary 1==First Argument
        for (int i = 0; i < opt->size(); ++i){
            str = opt->at(i);

    //        qDebug() << QString(" [%1]").arg(str);

            if(str.compare("-h") == 0 || str.compare("--help") == 0){
                printUsage();
                exit(0);
            }

            if(str.compare("-p")==0) {
                QString p1 = opt->value(i+1);

                if(p1!=0 && p1.endsWith(".dlp")){
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

                if(l1!=0 && l1.endsWith(".dlt")){
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

                if(f1!=0 && f1.endsWith(".dlf")){
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

                if(c1!=0 && c1.endsWith(".dlt") && c2!=0){
                    convertSourceFile = QString("%1").arg(c1);
                    convertDestFile = QString("%1").arg(c2);
                    convert = true;
                }else{
                    qDebug()<<"Error occured during processing of command line option \"-c\"";
                    printUsage();
                    exit(-1);
                }
            }
        }
     }
}

bool OptManager::isProjectFile(){ return project;}
bool OptManager::isLogFile(){return log;}
bool OptManager::isFilterFile(){return filter;}
bool OptManager::isConvert(){return convert;}
bool OptManager::isPlugin(){return plugin;}

QString OptManager::getProjectFile(){return projectFile;}
QString OptManager::getLogFile(){return logFile;}
QString OptManager::getFilterFile(){return filterFile;}
QString OptManager::getConvertSourceFile(){return convertSourceFile;}
QString OptManager::getConvertDestFile(){return convertDestFile;}

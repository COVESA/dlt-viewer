#include "optmanager.h"
#include <QDebug>
// Global static pointer used to ensure a single instance of the class.
OptManager* OptManager::instance;

OptManager::OptManager()
{
//    qDebug()<<"created optmanager";
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
    qDebug()<<"Nothing to print";
}

void OptManager::parse(QStringList *opt){
    QString str;
    for (int i = 0; i < opt->size(); ++i){
        str = opt->at(i);

//        qDebug() << QString(" [%1]").arg(str);

        if(str.compare("-h") == 0){
            printUsage();
            exit(0);
        }

        if(str.compare("-p")==0) {
            projectFile = QString("%1").arg(opt->at(i+1));
            project = true;
        }
        if(str.compare("-l")==0) {
            logFile = QString("%1").arg(opt->at(i+1));
            log = true;
        }
        if(str.compare("-f")==0) {
            filterFile = QString("%1").arg(opt->at(i+1));
            filter = true;
        }
        if(str.compare("-c")==0) {
            convertSourceFile = QString("%1").arg(opt->at(i+1));
            convertDestFile = QString("%1").arg(opt->at(i+2));
            convert = true;
        }
        if(str.compare("-e")==0) {

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

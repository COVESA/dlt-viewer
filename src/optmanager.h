#ifndef OPTMANAGER_H
#define OPTMANAGER_H

#include <QStringList>

class OptManager
{
public:
    static OptManager* getInstance();
    void printUsage();
    void parse(QStringList *opt);

    bool isProjectFile();
    bool isLogFile();
    bool isFilterFile();
    bool isConvert();
    bool isPlugin();

    QString getProjectFile();
    QString getLogFile();
    QString getFilterFile();
    QString getConvertSourceFile();
    QString getConvertDestFile();

private:
    OptManager();
    OptManager(OptManager const&);
    static OptManager *instance;

    bool project;
    bool log;
    bool filter;
    bool convert;
    bool plugin;
    QString projectFile;
    QString logFile;
    QString filterFile;
    QString convertSourceFile;
    QString convertDestFile;
};

#endif // OPTMANAGER_H

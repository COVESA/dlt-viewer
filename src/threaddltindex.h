#ifndef THREADDLTINDEX_H
#define THREADDLTINDEX_H

#include <QtCore>
#include "qdlt.h"
#include "project.h"
#include "plugininterface.h"

class ThreadDltIndex : public QThread
{
    Q_OBJECT

public:
    ThreadDltIndex(QObject *parent = 0);

    void setFilename(QString &_filename);
    QList<unsigned long> getIndexAll();

protected:
    void run();

private:
     QFile infile;
     QString filename;
     QList<unsigned long> indexAll;
signals:
    void updateProgressText(QString str);
    
public slots:
    
};

#endif // THREADDLTINDEX_H

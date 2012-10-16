#ifndef THREADFILTER_H
#define THREADFILTER_H

#include <QtCore>
#include "qdlt.h"
#include "project.h"
#include "plugininterface.h"
class ThreadFilter : public QThread
{
    Q_OBJECT
public:
    ThreadFilter(QObject *parent = 0);
    
    void setQDltFile(QDltFile *_qDltFile);
    void setActiveDecoderPlugins(QList<PluginItem*> *_activeDecoderPlugins);
    void setStartIndex(int i);
    void setStopIndex(int i);

protected:
    void run();

private:
    QDltFile *qDltFile;
    PluginItem *item;
    QList<PluginItem*> *activeDecoderPlugins;

    int startIndex;
    int stopIndex;

    bool stopExecution;

signals:
    void updateProgressText(QString str);
    void percentageComplete(int num);

public slots:
    void stopProcessMsg();
    
};

#endif // THREADFILTER_H

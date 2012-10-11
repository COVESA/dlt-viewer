#include "threadplugin.h"
#include <QDebug>

ThreadPlugin::ThreadPlugin(QObject* parent) : QThread(parent), stopExecution(false)
{
    //qDebug() << "Thread created";
}

void ThreadPlugin::stopProcessMsg(){
    stopExecution = true;
}

void ThreadPlugin::run(){
    qDebug() << "Starting Thread: " << currentThreadId();
    qDebug() << "startIdx: " << startIndex << " stopIdx: " << stopIndex;

    if(!qDltFile || !activeViewerPlugins || !activeDecoderPlugins || startIndex < 0 || stopIndex < 0 || stopIndex > qDltFile->size() )
    {
        qDebug() << "Error: finished thread for plugins - entry gate not passed";
        return;
    }

    qDltFile->clearFilterIndex();

    QDltMsg msg;
    QByteArray data;
    for(int num=startIndex;num<stopIndex;num++) {

        //        qDebug()<< QString("%1: %2").arg(threadIndex).arg(num);

        if(stopExecution)
            break;

        data = qDltFile->getMsg(num);
        if(data.isEmpty()){
            qDebug()<<"Error: getMsg in thread for plugins failed for num: " << num;
            //break;
        }
        msg.setMsg(data);
        //msg.setMsg(qDltFile->getMsg(num));

        for(int i = 0; i < activeViewerPlugins->size(); i++){
            item = (PluginItem*)activeViewerPlugins->at(i);
            item->pluginviewerinterface->initMsg(num,msg);
        }

        for(int i = 0; i < activeDecoderPlugins->size(); i++)
        {
            item = (PluginItem*)activeDecoderPlugins->at(i);

            if(item->plugindecoderinterface->isMsg(msg,0))
            {

                item->plugindecoderinterface->decodeMsg(msg,0);
                break;
            }
        }

        if(qDltFile->checkFilter(msg)) {
            qDltFile->addFilterIndex(num);
        }

        for(int i = 0; i < activeViewerPlugins->size(); i++){
            item = (PluginItem*)activeViewerPlugins->at(i);
            item->pluginviewerinterface->initMsgDecoded(num,msg);
        }

        if((num%((qDltFile->size()/200)+1))==0)
        {
            emit percentageComplete(num);
            emit updateProgressText(QString("Applying Plugins for Message %1/%2").arg(num).arg(qDltFile->size()));
        }
    }

    qDebug() << "Finished Thread";
}

void ThreadPlugin::setQDltFile(QDltFile *_qDltFile){
    this->qDltFile=_qDltFile;
}
void ThreadPlugin::setActiveViewerPlugins( QList<PluginItem*> *_activeViewerPlugins){
    activeViewerPlugins=_activeViewerPlugins;
}
void ThreadPlugin::setActiveDecoderPlugins(QList<PluginItem*> *_activeDecoderPlugins){
    activeDecoderPlugins=_activeDecoderPlugins;
}
void ThreadPlugin::setStartIndex(int i){
    startIndex = i;
}
void ThreadPlugin::setStopIndex(int i) {
    stopIndex = i;
}

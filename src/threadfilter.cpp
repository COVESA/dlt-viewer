#include "threadfilter.h"

ThreadFilter::ThreadFilter(QObject *parent) :
    QThread(parent), stopExecution(false)
{
}

void ThreadFilter::stopProcessMsg(){
    stopExecution = true;
}

void ThreadFilter::run(){
    qDebug() << "Starting Thread: " << currentThreadId();

    if(!qDltFile || !activeDecoderPlugins || startIndex < 0 || stopIndex < 0 || stopIndex > qDltFile->size() )
    {
        qDebug() << "Error: finished thread for plugins - entry gate not passed";
        return;
    }

    QDltMsg msg;
    QByteArray data;

    for(int num=startIndex;num<stopIndex;num++) {

        data = qDltFile->getMsg(num);
        if(data.isEmpty()){
            //qDebug()<<"Error: getMsg in thread for plugins failed for num: " << num;
            //break;
        }
        msg.setMsg(data);
        //msg.setMsg(qDltFile->getMsg(num));

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

        if((num%((stopIndex/200)+1))==0)
        {
            emit percentageComplete(num);
            emit updateProgressText(QString("Applying filters for message %1/%2").arg(num).arg(stopIndex));
        }

        if(stopExecution)
            break;
    }

    qDebug() << "Finished Thread";
}

void ThreadFilter::setQDltFile(QDltFile *_qDltFile){
    this->qDltFile=_qDltFile;
}

void ThreadFilter::setActiveDecoderPlugins(QList<PluginItem*> *_activeDecoderPlugins){
    activeDecoderPlugins=_activeDecoderPlugins;
}

void ThreadFilter::setStartIndex(int i){
    startIndex = i;
}

void ThreadFilter::setStopIndex(int i) {
    stopIndex = i;
}

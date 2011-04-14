#include <QtGui>

#include "speedplugin.h"

SpeedPlugin::SpeedPlugin()
{
    dltFile = 0;
}

SpeedPlugin::~SpeedPlugin()
{

}

QString SpeedPlugin::name()
{
    return QString("Dummy Viewer Plugin");
}

QString SpeedPlugin::description()
{
    return QString();
}

QString SpeedPlugin::error()
{
    return QString();
}

bool SpeedPlugin::loadConfig(QString /* filename */)
{
    return true;
}

bool SpeedPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList SpeedPlugin::infoConfig()
{
    return QStringList();
}

QWidget* SpeedPlugin::initViewer()
{
    form = new Form();
    return form;
}

bool SpeedPlugin::initFile(QDltFile *file)
{
    dltFile = file;

    form->setMessages(dltFile->size());

    counterMessages = dltFile->size();

    counterNonVerboseMessages = 0;
    counterVerboseMessages = 0;

    updateCounters(0,counterMessages-1);

    return true;
}

void SpeedPlugin::updateFile()
{
    if(!dltFile)
        return;

    updateCounters(counterMessages-1,dltFile->size()-1);

    counterMessages = dltFile->size();
}

void SpeedPlugin::selectedIdxMsg(int index)
{
    if(!dltFile)
        return;

    form->setSelectedMessage(index);
}

void SpeedPlugin::updateCounters(int start,int end)
{
    QByteArray data;
    QDltMsg msg;

    if(!dltFile)
        return;


    for(int num=start;num<=end;num++)
    {
        if(dltFile->getMsg(num,msg)==true)
        {
            if(msg.getMode() == QDltMsg::DltModeVerbose)
            {
                counterVerboseMessages++;
            }
            if(msg.getMode() == QDltMsg::DltModeNonVerbose)
            {
                counterNonVerboseMessages++;
            }
        }
    }


   form->setMessages(dltFile->size());
   form->setVerboseMessages(counterVerboseMessages);
   form->setNonVerboseMessages(counterNonVerboseMessages);

}

Q_EXPORT_PLUGIN2(speedplugin, SpeedPlugin);

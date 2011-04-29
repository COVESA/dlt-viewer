#include <QtGui>

#include "speedplugin.h"

SpeedPlugin::SpeedPlugin()
{
    dltFile = 0;
    msgIndex = 0;
}

SpeedPlugin::~SpeedPlugin()
{

}

QString SpeedPlugin::name()
{
    return QString("Speed Plugin");
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

    return true;
}

void SpeedPlugin::updateFile()
{
    QByteArray buffer;
    QDltMsg msg;
    QDltArgument argument;
    if(!dltFile)
        return;




   for(;msgIndex<dltFile->size();msgIndex++)
    {

       buffer =  dltFile->getMsg(msgIndex);

       if(buffer.isEmpty())
           break;

       msg.setMsg(buffer);

        if( (msg.getApid().compare("SPEE") == 0) && (msg.getCtid().compare("SIG") == 0))
        {
            if(msg.getArgument(1,argument)) {
                            form->setSpeedLCD(argument,msg.getTimestamp());
            }

        }
    }
}

void SpeedPlugin::selectedIdxMsg(int index)
{
    if(!dltFile)
        return;

}

Q_EXPORT_PLUGIN2(speedplugin, SpeedPlugin);

#include <QtGui>

#include "dummydecoderplugin.h"

DummyDecoderPlugin::DummyDecoderPlugin()
{
}

DummyDecoderPlugin::~DummyDecoderPlugin()
{

}

QString DummyDecoderPlugin::name()
{
    return QString("Dummy Decoder Plugin");
}

QString DummyDecoderPlugin::description()
{
    return QString();
}

QString DummyDecoderPlugin::error()
{
    return QString();
}

bool DummyDecoderPlugin::loadConfig(QString /* filename */ )
{
    return true;
}

bool DummyDecoderPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList DummyDecoderPlugin::infoConfig()
{
    return QStringList();
}

bool DummyDecoderPlugin::isMsg(QDltMsg & /* msg */)
{
    return false;
}

bool DummyDecoderPlugin::decodeMsg(QDltMsg & /* msg */)
{
    return false;
}

Q_EXPORT_PLUGIN2(dummydecoderplugin, DummyDecoderPlugin);

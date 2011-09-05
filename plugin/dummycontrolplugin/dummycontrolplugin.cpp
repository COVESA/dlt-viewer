#include <QtGui>

#include "dummycontrolplugin.h"

DummyControlPlugin::DummyControlPlugin()
{
    dltFile = 0;
    dltControl = 0;
}

DummyControlPlugin::~DummyControlPlugin()
{

}

QString DummyControlPlugin::name()
{
    return QString("Dummy Control Plugin");
}

QString DummyControlPlugin::pluginVersion(){
    return DUMMY_CONTROL_PLUGIN_VERSION;
}

QString DummyControlPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DummyControlPlugin::description()
{
    return QString();
}

QString DummyControlPlugin::error()
{
    return QString();
}

bool DummyControlPlugin::loadConfig(QString /* filename */)
{
    return true;
}

bool DummyControlPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList DummyControlPlugin::infoConfig()
{
    return QStringList();
}

QWidget* DummyControlPlugin::initViewer()
{
    form = new Form(this);
    return form;
}

bool DummyControlPlugin::initFile(QDltFile *file)
{
    dltFile = file;

    return true;
}

void DummyControlPlugin::updateFile()
{
    if(!dltFile)
        return;

}

void DummyControlPlugin::selectedIdxMsg(int index)
{
    if(!dltFile)
        return;

}

bool DummyControlPlugin::initControl(QDltControl *control)
{
    dltControl = control;

    return true;
}

bool DummyControlPlugin::initConnections(QStringList list)
{
    form->setConnections(list);

    return false;
}

bool DummyControlPlugin::controlMsg(int index, QDltMsg &msg)
{
    return false;
}

void DummyControlPlugin::updateCounters(int start,int end)
{

}

Q_EXPORT_PLUGIN2(dummycontrolplugin, DummyControlPlugin);

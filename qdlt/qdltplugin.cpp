#include "qdltplugin.h"

#include <QDir>
#include <QCoreApplication>
#include <QTableView>

#include <QPluginLoader>

QDltPlugin::QDltPlugin()
{
    plugininterface = 0;
    pluginviewerinterface = 0;
    plugindecoderinterface = 0;
    plugincontrolinterface = 0;
    plugincommandinterface = 0;

    mode = ModeDisable;
}

int QDltPlugin::getMode()
{
    //return QDltSettingsManager::getInstance()->value("plugin/pluginmodefor"+getName(),QVariant(QDltPlugin::ModeDisable)).toInt();
    return mode;
}

void QDltPlugin::setMode(QDltPlugin::Mode _mode)
{
    //return QDltSettingsManager::getInstance()->value("plugin/pluginmodefor"+getName(),QVariant(QDltPlugin::ModeDisable)).toInt();
    mode = _mode;
}

void QDltPlugin::setFilename(QString _filename)
{
    filename = _filename;
    if(plugininterface)
        plugininterface->loadConfig(_filename);
    setMode(ModeEnable);

}

QString QDltPlugin::getFilename()
{
    return filename;
}

void QDltPlugin::loadPlugin(QObject *plugin)
{
    plugininterface = qobject_cast<QDLTPluginInterface *>(plugin);
    pluginviewerinterface = qobject_cast<QDltPluginViewerInterface *>(plugin);
    if(pluginviewerinterface)
    {
        /*widget = pluginviewerinterface->initViewer();

        if(widget)
        {
            dockWidget = new QDockWidget(getName(),this);
            dockWidget = new MyPluginDockWidget(item,this);
            dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
            dockWidget->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
            dockWidget->setWidget(widget);
            dockWidget->setObjectName(getName());

            addDockWidget(Qt::LeftDockWidgetArea, dockWidget);

            if(getMode() != PluginItem::ModeShow)
            {
                dockWidget->hide();
            }
        }*/
    }
    plugindecoderinterface = qobject_cast<QDLTPluginDecoderInterface *>(plugin);
    plugincontrolinterface = qobject_cast<QDltPluginControlInterface *>(plugin);
    plugincommandinterface = qobject_cast<QDltPluginCommandInterface *>(plugin);
    //item->update();

}

bool QDltPlugin::isDecoder()
{
    return (plugindecoderinterface?true:false);
}

bool QDltPlugin::isViewer()
{
    return (pluginviewerinterface?true:false);
}

bool QDltPlugin::isControl()
{
    return (plugincontrolinterface?true:false);
}

bool QDltPlugin::isCommand()
{
    return (plugincommandinterface?true:false);
}

// generic plugin interfaces
QString QDltPlugin::name()
{
    if(plugininterface)
        return plugininterface->name();
    else
        return QString();
}

QString QDltPlugin::pluginVersion()
{
    if(plugininterface)
        return plugininterface->pluginVersion();
    else
        return QString();
}

QString QDltPlugin::pluginInterfaceVersion()
{
    if(plugininterface)
        return plugininterface->pluginInterfaceVersion();
    else
        return QString();
}

QString QDltPlugin::error()
{
    if(plugininterface)
        return plugininterface->error();
    else
        return QString();
}

bool QDltPlugin::loadConfig(QString filename)
{
    if(plugininterface)
        return plugininterface->loadConfig(filename);
    else
        return false;
}

QStringList QDltPlugin::infoConfig()
{
    if(plugininterface)
        return plugininterface->infoConfig();
    else
        return QStringList();
}

// viewer plugin interfaces
QWidget* QDltPlugin::initViewer()
{
if(pluginviewerinterface)
    return pluginviewerinterface->initViewer();
else
    return 0;
}
void QDltPlugin::initFileStart(QDltFile *file)
{
if(pluginviewerinterface)
    pluginviewerinterface->initFileStart(file);
}
void QDltPlugin::initMsg(int index, QDltMsg &msg)
{
if(pluginviewerinterface)
    pluginviewerinterface->initMsg(index,msg);
}
void QDltPlugin::initMsgDecoded(int index, QDltMsg &msg)
{
if(pluginviewerinterface)
    pluginviewerinterface->initMsgDecoded(index,msg);
}
void QDltPlugin::initFileFinish()
{
if(pluginviewerinterface)
    pluginviewerinterface->initFileFinish();
}
void QDltPlugin::updateFileStart()
{
if(pluginviewerinterface)
    pluginviewerinterface->updateFileStart();
}
void QDltPlugin::updateMsg(int index, QDltMsg &msg)
{
if(pluginviewerinterface)
    pluginviewerinterface->updateMsg(index,msg);
}
void QDltPlugin::updateMsgDecoded(int index, QDltMsg &msg)
{
if(pluginviewerinterface)
    pluginviewerinterface->updateMsgDecoded(index,msg);
}
void QDltPlugin::updateFileFinish()
{
if(pluginviewerinterface)
    pluginviewerinterface->updateFileFinish();
}
void QDltPlugin::selectedIdxMsg(int index, QDltMsg &msg)
{
if(pluginviewerinterface)
    pluginviewerinterface->selectedIdxMsg(index,msg);
}
void QDltPlugin::selectedIdxMsgDecoded(int index, QDltMsg &msg)
{
if(pluginviewerinterface)
    pluginviewerinterface->selectedIdxMsgDecoded(index,msg);
}

// control plugin interface
bool QDltPlugin::initControl(QDltControl *control)
{
    if(plugincontrolinterface)
        return plugincontrolinterface->initControl(control);
    else
        return false;
}

bool QDltPlugin::initConnections(QStringList list)
{
    if(plugincontrolinterface)
        return plugincontrolinterface->initConnections(list);
    else
        return false;
}

bool QDltPlugin::controlMsg(int index, QDltMsg &msg)
{
    if(plugincontrolinterface)
        return plugincontrolinterface->controlMsg(index,msg);
    else
        return false;
}

bool QDltPlugin::stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname)
{
    if(plugincontrolinterface)
        return plugincontrolinterface->stateChanged(index,connectionState,hostname);
    else
        return false;
}

bool QDltPlugin::autoscrollStateChanged(bool enabled)
{
    if(plugincontrolinterface)
        return plugincontrolinterface->autoscrollStateChanged(enabled);
    else
        return false;
}

void QDltPlugin::initMessageDecoder(QDltMessageDecoder* messageDecoder)
{
if(plugincontrolinterface)
    plugincontrolinterface->initMessageDecoder(messageDecoder);
}

void QDltPlugin::initMainTableView(QTableView* pTableView)
{
if(plugincontrolinterface)
    plugincontrolinterface->initMainTableView(pTableView);
}

void QDltPlugin::configurationChanged()
{
if(plugincontrolinterface)
    plugincontrolinterface->configurationChanged();
}

// decoder plugin interfaces
bool QDltPlugin::decodeMsg(QDltMsg &msg, int triggeredByUser)
{
    if(mode != ModeDisable && plugindecoderinterface && plugindecoderinterface->isMsg(msg,triggeredByUser))
    {
        return plugindecoderinterface->decodeMsg(msg,triggeredByUser);
    }
    return false;
}

// command plugin interfaces
bool QDltPlugin::command(QString cmd,QList<QString> params)
{
    if(plugincommandinterface)
        return plugincommandinterface->command(cmd,params);
    else
        return false;
}

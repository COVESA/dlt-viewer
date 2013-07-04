#include "qdltplugin.h"

#include <QDir>
#include <QCoreApplication>
#include <QPluginLoader>
#include <QMessageBox>
#include <QString>

QDltPluginManager::QDltPluginManager()
{
}

int QDltPluginManager::size()
{
    return plugins.size();
}

int QDltPluginManager::sizeEnabled()
{
    int count = 0;

    for(int num=0;num<plugins.size();num++)
    {
        QDltPlugin *plugin = plugins[num];
        if(plugin->getMode()>=QDltPlugin::ModeEnable)
            count++;
    }
    return count;
}

void QDltPluginManager::loadPlugins(const QString &settingsPluginPath)
{
    QDir pluginsDir;

    /* The viewer looks in the relativ to the executable in the ./plugins directory */
    pluginsDir.setPath(QCoreApplication::applicationDirPath());
    if(pluginsDir.cd("plugins"))
    {
        loadPluginsPath(pluginsDir);
    }

    /* load plugins form settings path if set */
    if(!settingsPluginPath.isEmpty())
    {
        pluginsDir.setPath(settingsPluginPath);
        if(pluginsDir.exists() && pluginsDir.isReadable())
        {
            loadPluginsPath(pluginsDir);
        }
    }

    /* Load plugins from system directory in linux */
    pluginsDir.setPath("/usr/share/dlt-viewer/plugins");
    if(pluginsDir.exists() && pluginsDir.isReadable())
    {
        loadPluginsPath(pluginsDir);
    }
}

void QDltPluginManager::loadPluginsPath(QDir &dir)
{
    /* set filter for plugin files */
    QStringList filters;
    filters << "*.dll" << "*.so";
    dir.setNameFilters(filters);

    /* iterate through all plugins */
    foreach (QString fileName, dir.entryList(QDir::Files))
    {
        QPluginLoader pluginLoader(dir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin)
        {
            QDLTPluginInterface *plugininterface = qobject_cast<QDLTPluginInterface *>(plugin);
            if (plugininterface)
            {
                if(QString::compare( plugininterface->pluginInterfaceVersion(),PLUGIN_INTERFACE_VERSION, Qt::CaseSensitive) == 0){

                    QDltPlugin* item = new QDltPlugin();
                    item->loadPlugin(plugin);
                    plugins.append(item);

                    //project.plugin->addTopLevelItem(item);

                } else {

                    QMessageBox::warning(0, QString("DLT Viewer"),QString("Error: Plugin could not be loaded!\nMismatch with plugin interface version of DLT Viewer.\n\nPlugin name: %1\nPlugin version: %2\nPlugin interface version: %3\nPlugin path: %4\n\nDLT Viewer - Plugin interface version: %5").arg(plugininterface->name()).arg(plugininterface->pluginVersion()).arg(plugininterface->pluginInterfaceVersion()).arg(dir.absolutePath()).arg(PLUGIN_INTERFACE_VERSION));
                }
            }
        }
        else {
            QMessageBox::warning(0, QString("DLT Viewer"),QString("The plugin %1 cannot be loaded.\n\nError: %2").arg(dir.absoluteFilePath(fileName)).arg(pluginLoader.errorString()));
        }
    }
}

void QDltPluginManager::loadConfig(QString pluginName,QString filename)
{
    for(int num=0;num<plugins.size();num++)
    {
        QDltPlugin *plugin = plugins[num];
        if(plugin->getName()==pluginName)
            plugin->setFilename(filename);
    }
}


void QDltPluginManager::decodeMsg(QDltMsg &msg, int triggeredByUser)
{
    for(int num=0;num<plugins.size();num++)
    {
        QDltPlugin *plugin = plugins[num];

        if(plugin->decodeMsg(msg,triggeredByUser))
            break;
    }
}

QDltPlugin* QDltPluginManager::findPlugin(QString &name)
{
    for(int num=0;num<plugins.size();num++)
    {
        QDltPlugin *plugin = plugins[num];

        if(plugin->getName()==name)
            return plugin;
    }
    return 0;
}

QList<QDltPlugin*> QDltPluginManager::getDecoderPlugins()
{
    QList<QDltPlugin*> list;

    for(int num=0;num<plugins.size();num++)
    {
        QDltPlugin *plugin = plugins[num];

        if(plugin->isDecoder() && plugin->getMode()>=QDltPlugin::ModeEnable)
            list.append(plugin);
    }
    return list;
}

QList<QDltPlugin*> QDltPluginManager::getViewerPlugins()
{
    QList<QDltPlugin*> list;

    for(int num=0;num<plugins.size();num++)
    {
        QDltPlugin *plugin = plugins[num];

        if(plugin->isViewer() && plugin->getMode()>=QDltPlugin::ModeEnable)
            list.append(plugin);
    }
    return list;
}

bool QDltPluginManager::stateChanged(int index, QDltConnection::QDltConnectionState connectionState)
{
    for(int num=0;num<plugins.size();num++)
    {
        QDltPlugin *plugin = plugins[num];
        if(plugin->isControl() )
            plugin->stateChanged(index,connectionState);
    }
    return true;
}

bool QDltPluginManager::initControl(QDltControl *control)
{
    for(int num=0;num<plugins.size();num++)
    {
        QDltPlugin *plugin = plugins[num];
        if(plugin->isControl() )
            plugin->initControl(control);
    }
    return true;
}

bool QDltPluginManager::initConnections(QStringList list)
{
    for(int num=0;num<plugins.size();num++)
    {
        QDltPlugin *plugin = plugins[num];
        if(plugin->isControl() )
            plugin->initConnections(list);
    }
    return true;
}

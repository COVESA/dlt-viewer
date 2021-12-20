#include "qdltplugin.h"

#include <QDir>
#include <QCoreApplication>
#include <QPluginLoader>
//#include <QMessageBox>
#include <QTextStream>
#include <QString>

#ifndef PLUGIN_INSTALLATION_PATH
#define PLUGIN_INSTALLATION_PATH ""
#endif

QDltPluginManager::QDltPluginManager()
{
}

int QDltPluginManager::size() const
{
    return plugins.size();
}

int QDltPluginManager::sizeEnabled() const
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
QStringList QDltPluginManager::loadPlugins(const QString &settingsPluginPath)
{
    QDir pluginsDir1;
    QDir pluginsDir2;
    QDir pluginsDir3;
    QStringList errorStrings;

    QString defaultPluginPath = PLUGIN_INSTALLATION_PATH;

    /* The viewer always looks in the relative to the executable in the ./plugins directory */
    pluginsDir1.setPath(QCoreApplication::applicationDirPath());
    if(pluginsDir1.cd("plugins"))
    {
        errorStrings << loadPluginsPath(pluginsDir1);
    }

    /* Check system plugins path */
    if(!defaultPluginPath.isEmpty())
    {
        pluginsDir2.setPath(defaultPluginPath);
        if(pluginsDir2.exists() && pluginsDir2.canonicalPath() != pluginsDir1.canonicalPath())
        {
            errorStrings << loadPluginsPath(pluginsDir2);
        }
    }

    /* load plugins form settings path if set */
    if(!settingsPluginPath.isEmpty())
    {
        pluginsDir3.setPath(settingsPluginPath);
        if(pluginsDir3.exists() && pluginsDir3.isReadable()
            && pluginsDir3.canonicalPath() != pluginsDir1.canonicalPath()
            && pluginsDir3.canonicalPath() != pluginsDir2.canonicalPath())
        {
            errorStrings << loadPluginsPath(pluginsDir3);
        }
    }

    return errorStrings;
}

QStringList QDltPluginManager::loadPluginsPath(QDir &dir)
{
    /* set filter for plugin files */
    QStringList filters, errorStrings;
    QString errorString;
    filters << "*.dll" << "*.so" << "*.dylib";
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
                    item->initMessageDecoder(this);
                    plugins.append(item);

                    //project.plugin->addTopLevelItem(item);

                } else {

                    // This is an abomination - it's unreadable 425+ chars long
                    // QMessageBox::warning(0, QString("DLT Viewer"),QString("Error: Plugin could not be loaded!\nMismatch with plugin interface version of DLT Viewer.\n\nPlugin name: %1\nPlugin version: %2\nPlugin interface version: %3\nPlugin path: %4\n\nDLT Viewer - Plugin interface version: %5").arg(plugininterface->name()).arg(plugininterface->pluginVersion()).arg(plugininterface->pluginInterfaceVersion()).arg(dir.absolutePath()).arg(PLUGIN_INTERFACE_VERSION));
                    QString s;
                    QTextStream errStr(&s);
                    errStr << "-------------"
                           << "Error: Plugin could not be loaded!\n"
                           << "Mismatch with plugin interface version of DLT Viewer.\n\n"
                           << "Plugin name: " << plugininterface->name() << "\n"
                           << "Plugin version: " << plugininterface->pluginVersion() << "\n"
                           << "Plugin interface version: " << plugininterface->pluginInterfaceVersion() << "\n"
                           << "Plugin path: " << dir.absolutePath() << "\n\n"
                           << "DLT Viewer - Plugin interface version: " << PLUGIN_INTERFACE_VERSION  << "\n";
                    errorStrings.append(s);
                }
            }
        }
        else {
            //QMessageBox::warning(0, QString("DLT Viewer"),QString("The plugin %1 cannot be loaded.\n\nError: %2").arg(dir.absoluteFilePath(fileName)).arg(pluginLoader.errorString()));
            QString s;
            QTextStream  errStr(&s);
            errStr << "-------------"
                    << "The plugin " << dir.absoluteFilePath(fileName) << "cannot be loaded.\n\n"
                    << "Error: " << pluginLoader.errorString() << "\n";
            errorStrings.append(s);

        }
    }
    return errorStrings;
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

bool QDltPluginManager::stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname)
{
    for(int num=0;num<plugins.size();num++)
    {
        QDltPlugin *plugin = plugins[num];
        if(plugin->isControl() )
            plugin->stateChanged(index,connectionState,hostname);
    }
    return true;
}

bool  QDltPluginManager::autoscrollStateChanged(bool enabled)
{
    for(int num=0;num<plugins.size();num++)
    {
        QDltPlugin *plugin = plugins[num];
        if(plugin->isControl() )
            plugin->autoscrollStateChanged(enabled);
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

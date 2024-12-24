#include "qdltplugin.h"
#include "qdltpluginmanager.h"

#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QPluginLoader>
#include <QMutex>
#include <QTextStream>
#include <QString>

#ifndef PLUGIN_INSTALLATION_PATH
#define PLUGIN_INSTALLATION_PATH ""
#endif

int QDltPluginManager::size() const
{
    return plugins.size();
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
    QStringList errorStrings;

    dir.setNameFilters(QStringList{} << "*.dll" << "*.so" << "*.dylib");
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
                    pluginListMutex.lock();
                    plugins.append(item);
                    pluginListMutex.unlock();

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

void QDltPluginManager::loadConfig(QString pluginName, QString filename) {
    forEachPlugin([&](auto* plugin) {
        if (plugin->name() == pluginName)
            plugin->setFilename(filename);
    });
}

void QDltPluginManager::decodeMsg(QDltMsg &msg, int triggeredByUser)
{
    QMutexLocker mutexLocker(&pluginListMutex);
    for(auto* plugin : plugins)
    {
        if(plugin->decodeMsg(msg,triggeredByUser))
            break;
    }
}

QDltPlugin* QDltPluginManager::findPlugin(const QString& name) const {
    QDltPlugin* result = nullptr;
    QMutexLocker mutexLocker(&pluginListMutex);
    for (auto* plugin : plugins) {
        if (plugin->name() == name) {
            result = plugin;
            break;
        }
    }

    return result;
}

void QDltPluginManager::initPluginPriority(const QStringList& desiredPrio)
{
    QStringList finalPrio;

    if(plugins.size() > 1) {
        int prio = 0;
        for (int i = 0; i < desiredPrio.count(); ++i) {
            QString pluginName(desiredPrio[i]);
            if (setPluginPriority(pluginName, prio)) {
                ++prio;
            }
        }
    }
}

bool QDltPluginManager::decreasePluginPriority(const QString &name)
{
    bool result = false;

    if(plugins.size() > 1)
    {
        QMutexLocker mutexLocker(&pluginListMutex);
        for(int num=0; num < plugins.size()-1; ++num)
        {
            if(plugins[num]->name() == name)
            {
                qDebug() << "decrease prio of" << name << "from" << num << "to" << num+1;
                plugins.move(num, num+1);
                result = true;
                break;
            }
        }
    }

    return result;
}

bool QDltPluginManager::raisePluginPriority(const QString &name)
{
    bool result = false;

    if(plugins.size() > 1)
    {
        QMutexLocker mutexLocker(&pluginListMutex);
        for(int num=1; num < plugins.size(); ++num)
        {
            if( plugins[num]->name() == name)
            {
                qDebug() << "raise prio of" << name << "from" << num << "to" << num-1;
                plugins.move(num, num-1);
                result = true;
                break;
            }
        }
    }

    return result;
}

bool QDltPluginManager::setPluginPriority(const QString& name, int prio)
{
    bool result = false;

    if(plugins.size() > 1) {
        //if prio is too large, put to the end of the list
        if(prio >= plugins.size()) {
            prio = plugins.size() - 1;
        }

        QMutexLocker mutexLocker(&pluginListMutex);
        for (int num = 0; num < plugins.size(); ++num) {
            if (plugins[num]->name() == name) {
                if (prio != num) {
                    qDebug() << "Changing priority of plugin" << name << "from" << num << "to" << prio;
                    plugins.move(num, prio);
                }
                result = true;
                break;
            }
        }
    }

    return result;
}

QStringList QDltPluginManager::getPluginPriorities() const
{
    QStringList finalPrio;
    QMutexLocker mutexLocker(&pluginListMutex);
    for (auto* plugin: plugins) {
        finalPrio << plugin->name();
    }

    return finalPrio;
}

QList<QDltPlugin*> QDltPluginManager::getDecoderPlugins()
{
    QList<QDltPlugin*> list;
    forEachPlugin([&](auto* plugin) {
        if (plugin->isDecoder() && plugin->getMode() >= QDltPlugin::ModeEnable)
            list.append(plugin);
    });

    return list;
}

QList<QDltPlugin*> QDltPluginManager::getViewerPlugins()
{
    QList<QDltPlugin*> list;
    forEachPlugin([&](auto* plugin) {
        if (plugin->isViewer() && plugin->getMode() >= QDltPlugin::ModeEnable)
            list.append(plugin);
    });

    return list;
}

bool QDltPluginManager::stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname)
{
    forEachPlugin([&](auto* plugin) {
        if (plugin->isControl())
            plugin->stateChanged(index, connectionState, hostname);
    });

    return true;
}

bool  QDltPluginManager::autoscrollStateChanged(bool enabled)
{
    forEachPlugin([&](auto* plugin){
        if(plugin->isControl() )
            plugin->autoscrollStateChanged(enabled);
    });

    return true;
}

bool QDltPluginManager::initControl(QDltControl *control)
{
    forEachPlugin([&](auto* plugin){
        if(plugin->isControl() )
            plugin->initControl(control);
    });

    return true;
}

bool QDltPluginManager::initConnections(QStringList list)
{
    forEachPlugin([&](auto* plugin){
        if(plugin->isControl() )
            plugin->initConnections(list);
    });

    return true;
}


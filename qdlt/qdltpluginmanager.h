#ifndef QDLTPLUGINMANAGER_H
#define QDLTPLUGINMANAGER_H

#include "plugininterface.h"

#include <QDir>

#include "export_rules.h"

//! Manage all DLT Plugins
/*!
  This class loads all DLT Viewer Plugins and provides access to them.
*/

class QDltPlugin;

class QDLT_EXPORT QDltPluginManager
{
public:

    //! Constructor
    QDltPluginManager();

    //! The number of plugins
    /*!
      \return the number of loaded plugins.
    */
    int size();

    //! The number of plugins and also enabled
    /*!
      \return the number of loaded plugins and enabled.
    */
    int sizeEnabled();

    //! Loads all plugins from three directories.current working sub directory /plugin
    /*!
      The three directories:
      - ./plugin
      - settingsPluginPath
      - /usr/share/dlt-viewer/plugins
      \param settingsPluginPath Full path name.
    */
    void loadPlugins(const QString &settingsPluginPath);

    //! Loads the configuration of the plugin with the pluginName
    /*!
      \param pluginName The name of the plugin to load the configuration.
      \param filename The file to be loaded.
    */
    void loadConfig(QString pluginName,QString filename);

    //! Decode message by decoding through all loaded an activated decoder plugins
    /*!
      \param msg The message to be decoded.
    */
    void decodeMsg(QDltMsg &msg,int triggeredByUser);

    //! Get the list of pointers to all loaded plugins
    QList<QDltPlugin*> getPlugins() { return plugins; }

    //! Get the list of pointers to all enabled decoder plugins
    QList<QDltPlugin*> getDecoderPlugins();

    //! Get the list of pointers to all enabled viewer plugins
    QList<QDltPlugin*> getViewerPlugins();

    //! Find a plugin with the specific name
    /*!
      \param name The name of the plugin to be searched for.
      \return pinter to plugin or zero if no plugin with the name is found
    */
    QDltPlugin* findPlugin(QString &name);

    //control plugin interface
    bool stateChanged(int index, QDltConnection::QDltConnectionState connectionState, QString hostname);
    bool autoscrollStateChanged(bool enabled);
    bool initControl(QDltControl *control);
    bool initConnections(QStringList list);

private:

    //! The list of pointers to all loaded plugins
    QList<QDltPlugin*> plugins;

    //! Loads all plugins from a special directory
    void loadPluginsPath(QDir &dir);

};

#endif // QDLTPLUGINMANAGER_H

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
class QMutex;

class QDLT_EXPORT QDltPluginManager : public QDltMessageDecoder
{
public:

    //! Constructor
    QDltPluginManager();
    ~QDltPluginManager();

    //! The number of plugins
    /*!
      \return the number of loaded plugins.
    */
    int size() const;

    //! The number of plugins that are enabled
    /*!
      \return the number of loaded plugins and enabled.
    */
    int sizeEnabled() const;

    //! Loads all plugins from three directories.current working sub directory /plugin
    /*!
      The three directories:
      - ./plugin
      - settingsPluginPath
      - /usr/share/dlt-viewer/plugins
      \param settingsPluginPath Full path name.
    */
    QStringList loadPlugins(const QString &settingsPluginPath);

    //! Loads the configuration of the plugin with the pluginName
    /*!
      \param pluginName The name of the plugin to load the configuration.
      \param filename The file to be loaded.
    */
    void loadConfig(QString pluginName,QString filename);

    //! Implementation of QDltMessageDecoder's pure virtual method.
    //! Decode message by decoding through all loaded an activated decoder plugins.
    /*!
      \param msg The message to be decoded.
      \param triggeredByUser Whether decode operation was triggered by the user or not
    */
    void decodeMsg(QDltMsg &msg,int triggeredByUser) override;

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

    //control plugin execution order
    void initPluginPriority(const QStringList &desiredPrio);
    bool decreasePluginPriority(const QString &name);
    bool raisePluginPriority(const QString &name);
    bool setPluginPriority(const QString name, unsigned int prio);
    QStringList getPluginPriorities() const;

private:
    mutable QMutex* pMutex_pluginList;

    //! The list of pointers to all loaded plugins
    QList<QDltPlugin*> plugins;

    //! Loads all plugins from a special directory
    QStringList loadPluginsPath(QDir &dir);

};

#endif // QDLTPLUGINMANAGER_H

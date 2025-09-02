#ifndef QDLTPLUGINMANAGER_H
#define QDLTPLUGINMANAGER_H

#include "qdltconnection.h"
#include "qdltcontrol.h"
#include "qdltmessagedecoder.h"

#include "export_rules.h"

#include <QDir>

//! Manage all DLT Plugins
/*!
  This class loads all DLT Viewer Plugins and provides access to them.
*/

class QDltPlugin;
class QMutex;

class QDLT_EXPORT QDltPluginManager : public QDltMessageDecoder
{
public:
    //! The number of plugins
    /*!
      \return the number of loaded plugins.
    */
    int size() const;

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
    QList<QDltPlugin*> getPlugins() const { return plugins; }

    //! Get the list of pointers to all enabled decoder plugins
    QList<QDltPlugin*> getDecoderPlugins() const;

    //! Get the list of pointers to all enabled viewer plugins
    QList<QDltPlugin*> getViewerPlugins() const;

    //! Find a plugin with the specific name
    /*!
      \param name The name of the plugin to be searched for.
      \return pointer to plugin or nullptr if no plugin with the name is found
    */
    QDltPlugin* findPlugin(const QString &name) const;

    //control plugin interface
    bool stateChanged(int index, QDltConnection::QDltConnectionState connectionState, QString hostname);
    bool autoscrollStateChanged(bool enabled);
    bool initControl(QDltControl *control);
    bool initConnections(QStringList list);

    //control plugin execution order
    void initPluginPriority(const QStringList &desiredPrio);
    bool decreasePluginPriority(const QString &name);
    bool raisePluginPriority(const QString &name);
    bool setPluginPriority(const QString& name, int prio);
    QStringList getPluginPriorities() const;

private:
    mutable QMutex pluginListMutex;

    //! The list of pointers to all loaded plugins
    QList<QDltPlugin*> plugins;

    //! Loads all plugins from a special directory
    QStringList loadPluginsPath(QDir &dir);

};

#endif // QDLTPLUGINMANAGER_H

#ifndef QDLTPLUGINMANAGER_H
#define QDLTPLUGINMANAGER_H

#include "qdltconnection.h"
#include "qdltcontrol.h"
#include "qdltmessagedecoder.h"

#include "export_rules.h"

#include <QDir>
#include <atomic>
#include <cstdint>

//! Manage all DLT Plugins
/*!
  This class loads all DLT Viewer Plugins and provides access to them.
*/

class QDltPlugin;
class QMutex;
class QPluginLoader;

class QDLT_EXPORT QDltPluginManager : public QDltMessageDecoder
{
public:
    enum class PluginStage
    {
        Ingest,
        Decode,
        Enrich
    };

    QDltPluginManager() = default;
    ~QDltPluginManager();

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

    //! Decode message through enabled decoder plugins and report if handled.
    bool decodeMsgHandled(QDltMsg &msg, int triggeredByUser);

    //! Return the generation identifying the current decoder pipeline state.
    std::uint64_t decodePipelineGeneration() const noexcept;
    //! Mark decoder order, mode, or configuration as changed.
    void invalidateDecodePipeline() noexcept;

    //! Try to decode without blocking when plugin list is currently busy.
    /*! Returns false when decode was skipped to avoid lock contention. */
    bool decodeMsgTry(QDltMsg &msg, int triggeredByUser);
    //! Decode a message using a caller-supplied plugin snapshot, serialized against all other decode callers.
    //! Callers (search workers, CFI, live worker) can snapshot the decoder list without a per-message list lock,
    //! but must funnel the actual decodeMsg() invocation through here since plugin implementations are stateful
    //! and are not safe to call concurrently from multiple threads.
    void decodeMsgUsingPlugins(const QList<QDltPlugin*> &pluginsSnapshot, QDltMsg &msg, int triggeredByUser) const;

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

    //! Serializes the actual plugin->decodeMsg() invocation across every entry point
    //! (decodeMsg(), decodeMsgTry(), decodeMsgUsingPlugins()), since plugin instances
    //! are shared and stateful and must never be called concurrently.
    mutable QMutex decodeMutex;
    std::atomic<std::uint64_t> m_decodePipelineGeneration{1};

    //! The list of pointers to all loaded plugins
    QList<QDltPlugin*> plugins;

    //! Keep plugin loaders alive so plugins unload before QApplication shuts down.
    QList<QPluginLoader*> pluginLoaders;

    //! Loads all plugins from a special directory
    QStringList loadPluginsPath(QDir &dir);

};

#endif // QDLTPLUGINMANAGER_H

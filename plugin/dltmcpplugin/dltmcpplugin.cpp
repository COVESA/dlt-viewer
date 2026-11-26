// Integrates MCP with DLT Viewer: plugin lifecycle, configuration, and host callbacks.
// Starts and stops the network server on a separate thread.

#include "dltmcpplugin.h"
#include "dltmcpserver.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMetaObject>
#include <QRandomGenerator>
#include <QThread>

namespace {
//! Maximum trace size (in total message count) for which MCP is allowed to
//! mutate the active DLT Viewer filter list. Above this limit, add/remove/
//! enable-filter tools refuse the request because rebuilding the filtered
//! index on huge traces can freeze the UI for seconds.
constexpr int kDefaultFilterMutationMessageLimit = 100000;
} // namespace

DltMcpPlugin::DltMcpPlugin()
    : dltControl(nullptr), messageDecoder(nullptr), mainTableView(nullptr), shuttingDown(false),
      mcpServer(nullptr), serverThread(nullptr), dltFile(nullptr), viewerWidget(nullptr),
      viewerText(nullptr), viewerRefreshPending(false), serverPort(3719),
      allowRemoteConnections(false), filterMutationMessageLimit(kDefaultFilterMutationMessageLimit),
      controlInitialized(false), configLoaded(false) {
    connect(this, &DltMcpPlugin::viewerStateChanged, this, &DltMcpPlugin::requestViewerRefresh,
            Qt::QueuedConnection);
    connect(this, &DltMcpPlugin::responseReady, this, &DltMcpPlugin::onResponseReadyTracked);
    qDebug() << "DLT MCP Plugin: Constructor";
}

// Converts a JSON-RPC id value into a stable, type-qualified key string.
// The key is used to correlate outgoing responses back to the originating
// tool name in pendingToolNamesByRequestId, even when ids have different
// JSON types (e.g. string "1" vs number 1).
QString DltMcpPlugin::requestIdKey(const QJsonValue& id) const {
    if (id.isUndefined())
        return QStringLiteral("<undefined>");
    if (id.isNull())
        return QStringLiteral("<null>");
    if (id.isString())
        return QStringLiteral("s:") + id.toString();
    if (id.isDouble())
        return QStringLiteral("n:") + QString::number(id.toDouble(), 'g', 17);
    if (id.isBool())
        return QStringLiteral("b:") + QString::number(id.toBool() ? 1 : 0);
    if (id.isArray())
        return QStringLiteral("a:") +
               QString::fromUtf8(QJsonDocument(id.toArray()).toJson(QJsonDocument::Compact));
    if (id.isObject())
        return QStringLiteral("o:") +
               QString::fromUtf8(QJsonDocument(id.toObject()).toJson(QJsonDocument::Compact));
    return QStringLiteral("<unknown>");
}

void DltMcpPlugin::setLastToolCallStatus(const QString& toolName, const QString& status) {
    lastToolCallName = toolName;
    lastToolCallStatus = status;
    lastToolCallTime = QDateTime::currentDateTime();
    emit viewerStateChanged();
}

// Tracks tool completion state for the viewer panel.
// This slot is connected to responseReady and updates the "last tool call"
// fields (name/status/time) when a response is about to be sent to a client.
// The viewer is then refreshed via viewerStateChanged.
void DltMcpPlugin::onResponseReadyTracked(const QString& clientId, const QJsonObject& response) {
    Q_UNUSED(clientId);

    const QString key = requestIdKey(response.value("id"));
    const QString toolName = pendingToolNamesByRequestId.take(key);

    // Only the most recently started tool call drives the viewer display.
    // Responses from older still-in-flight requests (or non-tool responses
    // such as initialize) are delivered to the client as usual but ignored
    // here so they don't clobber the current call's name/status/time.
    if (key != latestToolCallRequestKey || toolName.isEmpty())
        return;

    setLastToolCallStatus(toolName, response.contains("error") ? QStringLiteral("error")
                                                               : QStringLiteral("ok"));
}

// Cleanly shut down the background server thread before the plugin is unloaded.
// All teardown lives in stopServer(): the thread's event loop is asked to
// quit, the destructor blocks until it has exited (with a safety timeout),
// and the server/thread objects are released via deleteLater() because they
// live on a different thread than this destructor.
DltMcpPlugin::~DltMcpPlugin() {
    qDebug() << "DLT MCP Plugin: Destructor";

    stopServer();
}

/* QDLTPluginInterface implementation */
QString DltMcpPlugin::name() { return QString("DLT MCP Plugin"); }

QString DltMcpPlugin::pluginVersion() { return QString(DLT_MCP_PLUGIN_VERSION); }

QString DltMcpPlugin::pluginInterfaceVersion() { return QString(PLUGIN_INTERFACE_VERSION); }

QString DltMcpPlugin::description() {
    return QString("Model Context Protocol (MCP) plugin for DLT Viewer - enables AI assistants to "
                   "analyze DLT logs");
}

QString DltMcpPlugin::error() { return errorText; }

bool DltMcpPlugin::loadConfig(QString filename) {
    const quint16 oldServerPort = serverPort;
    const bool oldAllowRemoteConnections = allowRemoteConnections;
    filterMutationMessageLimit = kDefaultFilterMutationMessageLimit;
    customDictionaryEntries.clear();

    if (filename.trimmed().isEmpty()) {
        serverPort = 3719;
        allowRemoteConnections = false;
        filterMutationMessageLimit = kDefaultFilterMutationMessageLimit;
        errorText.clear();
        configLoaded = true;

        if (mcpServer &&
            (oldServerPort != serverPort || oldAllowRemoteConnections != allowRemoteConnections)) {
            stopServer();
        }
        tryStartServer();
        emit viewerStateChanged();
        return true;
    }

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "DLT MCP Plugin: loadConfig: cannot open" << filename;
        errorText = QString("Can not load configuration File: %1").arg(filename);
        return false;
    }

    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (!xml.isStartElement())
            continue;
        if (xml.name() == QLatin1String("port")) {
            bool ok = false;
            const quint16 p = static_cast<quint16>(xml.readElementText().trimmed().toUInt(&ok));
            if (ok && p > 0)
                serverPort = p;
        } else if (xml.name() == QLatin1String("allowRemoteConnections")) {
            allowRemoteConnections = (xml.readElementText().trimmed() == "true");
        } else if (xml.name() == QLatin1String("filterMutationMessageLimit")) {
            bool ok = false;
            const int limit = xml.readElementText().trimmed().toInt(&ok);
            if (ok)
                filterMutationMessageLimit = limit;
        } else if (xml.name() == QLatin1String("dictionary")) {
            while (!(xml.isEndElement() && xml.name() == QLatin1String("dictionary")) &&
                   !xml.atEnd() && !xml.hasError()) {
                xml.readNext();
                if (!xml.isStartElement())
                    continue;

                if (xml.name() != QLatin1String("entry")) {
                    xml.skipCurrentElement();
                    continue;
                }

                DictionaryEntry entry;
                entry.key = xml.attributes().value("key").toString().trimmed().toLower();

                const QString aliasesAttr = xml.attributes().value("aliases").toString();
                if (!aliasesAttr.trimmed().isEmpty()) {
                    const QStringList rawAliases = aliasesAttr.split(",", Qt::SkipEmptyParts);
                    for (const QString& alias : rawAliases)
                        entry.aliases << alias.trimmed().toLower();
                }

                entry.text = xml.readElementText().trimmed();

                if (!entry.key.isEmpty() && !entry.text.isEmpty())
                    customDictionaryEntries.append(entry);
            }
        }
    }

    if (xml.hasError())
        qDebug() << "DLT MCP Plugin: loadConfig XML error:" << xml.errorString();

    errorText.clear();
    configLoaded = true;
    qDebug() << "DLT MCP Plugin: loadConfig: port=" << serverPort
             << "allowRemoteConnections=" << allowRemoteConnections
             << "filterMutationMessageLimit=" << filterMutationMessageLimit
             << "customDictionaryEntries=" << customDictionaryEntries.size();

    if (mcpServer &&
        (oldServerPort != serverPort || oldAllowRemoteConnections != allowRemoteConnections)) {
        stopServer();
    }
    tryStartServer();
    emit viewerStateChanged();

    return true;
}

bool DltMcpPlugin::saveConfig(QString filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "DLT MCP Plugin: saveConfig: cannot open" << filename;
        return false;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("DltMcpPlugin");
    xml.writeTextElement("port", QString::number(serverPort));
    xml.writeTextElement("allowRemoteConnections", allowRemoteConnections ? "true" : "false");
    xml.writeTextElement("filterMutationMessageLimit", QString::number(filterMutationMessageLimit));
    if (!customDictionaryEntries.isEmpty()) {
        xml.writeStartElement("dictionary");
        for (const DictionaryEntry& entry : customDictionaryEntries) {
            xml.writeStartElement("entry");
            xml.writeAttribute("key", entry.key);
            if (!entry.aliases.isEmpty())
                xml.writeAttribute("aliases", entry.aliases.join(", "));
            xml.writeCharacters(entry.text);
            xml.writeEndElement();
        }
        xml.writeEndElement();
    }
    xml.writeEndElement();
    xml.writeEndDocument();

    qDebug() << "DLT MCP Plugin: saveConfig: port=" << serverPort
             << "allowRemoteConnections=" << allowRemoteConnections
             << "filterMutationMessageLimit=" << filterMutationMessageLimit
             << "customDictionaryEntries=" << customDictionaryEntries.size();
    return true;
}

QStringList DltMcpPlugin::infoConfig() {
    QStringList info;
    info << "DLT MCP Plugin - provides MCP server for AI integration";
    info << QString("Version: %1").arg(pluginVersion());
    info << QString("Status: %1").arg(mcpServer ? "running" : "stopped");
    info << QString("Host: %1")
                .arg(allowRemoteConnections ? "0.0.0.0 (all interfaces)"
                                            : "127.0.0.1 (loopback only)");
    info << QString("Port: %1").arg(serverPort);
    info << QString("Filter mutation message limit: %1")
                .arg(filterMutationMessageLimit < 0 ? QStringLiteral("no limit (may freeze UI)")
                                                    : QString::number(filterMutationMessageLimit));
    info << QString("Dictionary entries: %1 built-in + %2 custom")
                .arg(builtInDictionaryEntries().size())
                .arg(customDictionaryEntries.size());
    return info;
}

/* QDltPluginControlInterface implementation */

bool DltMcpPlugin::initControl(QDltControl* control) {
    qDebug() << "DLT MCP Plugin: initControl";
    dltControl = control;
    controlInitialized = true;
    tryStartServer();
    emit viewerStateChanged();

    return true;
}

QString DltMcpPlugin::authTokenPath() const {
    return QDir::homePath() + QStringLiteral("/.dlt/mcp/token.txt");
}

void DltMcpPlugin::ensureAuthToken() {
    if (!authToken.isEmpty())
        return;

    const QString path = authTokenPath();
    QDir dir(QDir::homePath() + QStringLiteral("/.dlt/mcp"));
    if (!dir.exists())
        dir.mkpath(dir.absolutePath());

    QFile file(path);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        const QByteArray existing = file.readAll().trimmed();
        file.close();
        if (!existing.isEmpty()) {
            authToken = existing;
            qDebug() << "DLT MCP Plugin: loaded auth token from" << path;
            return;
        }
    }

    // 32 random bytes -> 64 hex chars.
    QByteArray raw(32, Qt::Uninitialized);
    QRandomGenerator::system()->fillRange(reinterpret_cast<quint32*>(raw.data()), 8);
    authToken = raw.toHex();

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(authToken);
        file.close();
        file.setPermissions(QFile::ReadOwner | QFile::WriteOwner);
        qDebug() << "DLT MCP Plugin: generated new auth token at" << path;
    } else {
        qWarning() << "DLT MCP Plugin: could not persist auth token to" << path
                   << "- using in-memory token for this session";
    }
}

void DltMcpPlugin::startServer() {
    if (mcpServer || serverThread)
        return;

    shuttingDown = false;

    serverThread = new QThread(this);
    mcpServer = new DltMcpServer();

    const QString host = allowRemoteConnections ? "0.0.0.0" : "127.0.0.1";
    ensureAuthToken();
    mcpServer->setConfig(host, serverPort, authToken);
    mcpServer->moveToThread(serverThread);

    connect(serverThread, &QThread::started, mcpServer, &DltMcpServer::start);
    connect(serverThread, &QThread::finished, mcpServer, &QObject::deleteLater);
    connect(mcpServer, &DltMcpServer::error, this, &DltMcpPlugin::onServerError);
    connect(mcpServer, &DltMcpServer::messageReceived, this, &DltMcpPlugin::onServerMessage);
    connect(mcpServer, &DltMcpServer::clientSessionChanged, this,
            &DltMcpPlugin::viewerStateChanged);
    connect(this, &DltMcpPlugin::responseReady, mcpServer, &DltMcpServer::sendResponseToClient);

    serverThread->start();
    emit viewerStateChanged();
}

void DltMcpPlugin::stopServer() {
    if (!mcpServer && !serverThread)
        return;

    shuttingDown = true;
    findMessageByTimestampJob = FindMessageByTimestampJob();
    searchLogsAroundIndexJob = SearchLogsAroundIndexJob();
    searchLogsJob = SearchLogsJob();
    getErrorMessagesJob = GetErrorMessagesJob();

    if (mcpServer && serverThread) {
        disconnect(this, &DltMcpPlugin::responseReady, mcpServer,
                   &DltMcpServer::sendResponseToClient);
        disconnect(mcpServer, &DltMcpServer::clientSessionChanged, this,
                   &DltMcpPlugin::viewerStateChanged);
        disconnect(mcpServer, &DltMcpServer::messageReceived, this, &DltMcpPlugin::onServerMessage);
        disconnect(mcpServer, &DltMcpServer::error, this, &DltMcpPlugin::onServerError);

        if (serverThread->isRunning()) {
            QMetaObject::invokeMethod(mcpServer, "stop", Qt::QueuedConnection);
            serverThread->quit();

            if (!serverThread->wait(5000)) {
                qDebug()
                    << "DLT MCP Plugin: Server thread did not stop within timeout, terminating";
                serverThread->terminate();
                serverThread->wait(1000);
            }
        }
    } else if (mcpServer) {
        delete mcpServer;
    }

    if (serverThread)
        serverThread->deleteLater();

    mcpServer = nullptr;
    serverThread = nullptr;
    emit viewerStateChanged();
}

void DltMcpPlugin::tryStartServer() {
    // Host initialization can arrive in either order. Wait for both callbacks so
    // the listener uses the loaded settings and tool calls have a control interface.
    if (!controlInitialized || !configLoaded || mcpServer || serverThread)
        return;

    startServer();
}

bool DltMcpPlugin::initConnections(QStringList list) {
    configuredConnections = list;

    QMap<int, QDltConnection::QDltConnectionState> updatedStates;
    QMap<int, QString> updatedHostnames;

    for (int i = 0; i < configuredConnections.size(); ++i) {
        updatedStates.insert(i, connectionStates.value(i, QDltConnection::QDltConnectionOffline));
        updatedHostnames.insert(i, connectionHostnames.value(i));
    }

    connectionStates = updatedStates;
    connectionHostnames = updatedHostnames;
    emit viewerStateChanged();
    return true;
}

// Called for DLT control messages (service messages like get/set log level).
// Regular live log messages are not handled here.
bool DltMcpPlugin::controlMsg(int index, QDltMsg& msg) {
    Q_UNUSED(index);
    Q_UNUSED(msg);
    return false;
}

// Called whenever an ECU connection transitions between states:
//   QDltConnectionOffline / QDltConnectionConnecting /
//   QDltConnectionOnline  / QDltConnectionError
// Used to keep connection state visible to MCP tools and the plugin panel.
bool DltMcpPlugin::stateChanged(int index, QDltConnection::QDltConnectionState connectionState,
                                QString hostname) {
    if (index >= 0) {
        connectionStates.insert(index, connectionState);
        connectionHostnames.insert(index, hostname);
        emit viewerStateChanged();
    }
    return true;
}

bool DltMcpPlugin::autoscrollStateChanged(bool enabled) {
    Q_UNUSED(enabled);
    return true;
}

// Stores the shared message decoder so we can decode raw QDltMsg payloads
// through all active decoder plugins (e.g. non-verbose decoder) before
// returning them to the AI. This gives the AI human-readable text instead
// of raw binary payloads.
void DltMcpPlugin::initMessageDecoder(QDltMessageDecoder* messageDecoder) {
    this->messageDecoder = messageDecoder;
}

// Stores a pointer to the main message table so the navigate_to_message
// MCP tool can scroll to and select the requested row in the DLT Viewer UI.
void DltMcpPlugin::initMainTableView(QTableView* pTableView) { this->mainTableView = pTableView; }

// Called when the user changes filters or sort order in DLT Viewer.
// The dltFile pointer stays valid; QDltFile rebuilds its filter index
// internally, so the next tool call automatically sees the updated view.
void DltMcpPlugin::configurationChanged() {
    cancelAllActiveJobs(
        "%1 was cancelled because the DLT Viewer filter or sort configuration changed");
    emit viewerStateChanged();
}

// Called by DLT Viewer whenever a file is opened or reloaded.
// We store the pointer so all MCP tools can call dltFile->getMsg() and
// dltFile->getMsgFilter() to read the currently active data scope.
void DltMcpPlugin::initFileStart(QDltFile* file) {
    cancelAllActiveJobs("%1 was cancelled because the active DLT file changed");
    dltFile = file;
    emit viewerStateChanged();
}

void DltMcpPlugin::initFileFinish() {}
void DltMcpPlugin::initMsg(int, QDltMsg&) {}
void DltMcpPlugin::initMsgDecoded(int, QDltMsg&) {}
void DltMcpPlugin::updateFileStart() {}
void DltMcpPlugin::updateMsg(int, QDltMsg&) {}
void DltMcpPlugin::updateMsgDecoded(int, QDltMsg&) {}
void DltMcpPlugin::updateFileFinish() {}
void DltMcpPlugin::selectedIdxMsg(int, QDltMsg&) {}
void DltMcpPlugin::selectedIdxMsgDecoded(int, QDltMsg&) {}

/* Private slots */

// Slot connected to DltMcpServer::error signal (cross-thread, queued connection).
// Stores the error text so DLT Viewer can display it via error().
void DltMcpPlugin::onServerError(const QString& error) {
    qDebug() << "DLT MCP Plugin: Server error:" << error;
    errorText = error;
    emit viewerStateChanged();
}

#ifndef DLTMCPSERVER_H
#define DLTMCPSERVER_H

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

//! TCP-based HTTP/SSE server implementing the MCP transport layer.
//!
//! DltMcpServer listens on a local TCP port (default 3719) and accepts
//! connections from MCP-compatible clients. Requests are received over HTTP,
//! while responses are streamed back over Server-Sent Events (SSE). Payloads
//! are JSON-RPC 2.0 objects.
//!
//! This class is designed to run entirely inside a dedicated QThread
//! (created by DltMcpPlugin::startServer) so that network I/O
//! never stalls the DLT Viewer GUI thread. Communication back to the
//! plugin happens exclusively through Qt signals (queued connections).
//!
//! Lifecycle:
//!   start()  – called automatically when the owning thread starts
//!   stop()   – called from the destructor or when the thread is quit
class DltMcpServer : public QObject {
    Q_OBJECT

public:
    DltMcpServer(QObject* parent = nullptr);
    ~DltMcpServer();

    //! Configures the host and port before the server is started.
    //! Must be called before start() — i.e. before the server thread is launched.
    //! \param host       Host address to bind to (e.g. "127.0.0.1" or "0.0.0.0").
    //! \param port       TCP port to listen on (e.g. 3719).
    //! \param authToken  Bearer token required on every non-OPTIONS request.
    //!                   Must be non-empty; an empty token causes all requests
    //!                   to be rejected with 401 (fail-closed).
    void setConfig(const QString& host, quint16 port, const QByteArray& authToken);

    //! Starts the TCP server and begins accepting client connections.
    //! Called automatically via the QThread::started signal when the
    //! background thread is launched by DltMcpPlugin::startServer().
    //! Emits error() if the port cannot be bound.
    void start();

public slots:
    //! Stops the TCP server and gracefully disconnects all clients.
    //! Closes the listening socket and calls deleteLater() on each
    //! client socket to ensure safe cross-thread cleanup.
    Q_INVOKABLE void stop();

    //! Sends a JSON-RPC response to a specific connected MCP client identified by its session id.
    //! Called via a queued signal from the plugin thread to ensure the socket
    //! write happens on the server thread where the socket lives.
    //! \param clientId  The session id of the target MCP client.
    //! \param response  The JSON-RPC 2.0 response object to serialise and send.
    void sendResponseToClient(const QString& clientId, const QJsonObject& response);

    //! Returns a snapshot of currently connected MCP sessions.
    //! Each entry may include sessionId, peer address/port, user agent,
    //! and optional client identity fields provided via initialize.clientInfo.
    Q_INVOKABLE QJsonArray getConnectedClients() const;

signals:
    //! Emitted when the server encounters a fatal error (e.g. port already
    //! in use). Connected to DltMcpPlugin::onServerError via a queued
    //! cross-thread connection.
    //! \param message  Human-readable error description.
    void error(const QString& message);

    //! Emitted for every valid JSON-RPC 2.0 request received from a client.
    //! Connected to DltMcpPlugin::onServerMessage via a queued cross-thread
    //! connection so the plugin can process the request on the GUI thread.
    //! \param clientId  Identifies the originating MCP client session. The plugin
    //!                  must pass this value back to sendResponseToClient() so the
    //!                  reply reaches the correct socket.
    //! \param message   The parsed JSON-RPC 2.0 request object.
    void messageReceived(const QString& clientId, const QJsonObject& message);

    //! Emitted when an MCP client connects or disconnects its SSE session.
    //! Connected to DltMcpPlugin::viewerStateChanged to update the viewer panel.
    void clientSessionChanged();

private slots:
    //! Slot connected to QTcpServer::newConnection.
    //! Drains all pending connections, registers each new client socket,
    //! and wires up its readyRead/disconnected signals.
    void onNewConnection();

    //! Slot connected to QTcpSocket::readyRead for every connected client.
    //! Reads all available bytes, parses them as HTTP, and dispatches
    //! individual requests to handleRequest().
    void onClientReadyRead();

    //! Slot connected to QTcpSocket::disconnected for every connected client.
    //! Removes the socket from the client list and schedules it for deletion.
    void onClientDisconnected();

private:
    struct ClientState {
        QByteArray buffer;
        bool headersParsed = false;
        bool closing = false;
        QByteArray method;
        QByteArray path;
        QHash<QByteArray, QByteArray> headers;
        qint64 contentLength = 0;
        QByteArray body;
        QString sessionId;
        bool isEventStream = false;
        QString userAgent;
        QString clientName;
        QString clientVersion;
    };

    //! Dispatches a single parsed JSON-RPC 2.0 object.
    //! MCP protocol messages (initialize, tools/list, notifications) are
    //! handled directly here. tools/call requests are forwarded to the
    //! plugin via the messageReceived signal.
    //! \param request  Parsed JSON-RPC 2.0 object.
    //! \param clientId The originating MCP client session id.
    void handleRequest(const QJsonObject& request, const QString& clientId);

    //! Handles the MCP "initialize" handshake request.
    //! Responds with server name, version, and declared capabilities.
    QJsonObject handleInitialize(const QJsonObject& request) const;

    //! Handles the "tools/list" request.
    //! Returns the JSON schema for every tool the plugin exposes so that
    //! MCP clients know what they can call and what parameters each tool
    //! expects.
    QJsonObject handleToolsList(const QJsonObject& request) const;

    //! Builds and returns the array of tool descriptors (name, description,
    //! inputSchema) for all tools exposed by this MCP server.
    QJsonArray buildToolsArray() const;

    void sendHttpResponse(QTcpSocket* socket, int statusCode, const QByteArray& statusText,
                          const QByteArray& contentType, const QByteArray& body = QByteArray(),
                          const QList<QByteArray>& extraHeaders = QList<QByteArray>());
    void sendSseEvent(QTcpSocket* socket, const QByteArray& eventName, const QByteArray& payload);
    void sendSseEvent(QTcpSocket* socket, const QByteArray& eventName, const QJsonObject& payload);
    void sendSseEvent(const QString& clientId, const QByteArray& eventName,
                      const QJsonObject& payload);
    void sendSseComment(QTcpSocket* socket, const QByteArray& comment);
    bool tryProcessBufferedRequest(QTcpSocket* client);
    void handleHttpRequest(QTcpSocket* socket, const ClientState& state);

    //! Native MCP clients send no Origin; browsers always do. In loopback mode
    //! any non-empty Origin is rejected. In remote mode any Origin is accepted.
    bool isOriginAllowed(const QByteArray& origin) const;

    //! Constant-time bearer token check against the configured authToken.
    bool isAuthorized(const QByteArray& authHeader) const;

    //! In loopback mode only 127.0.0.1 / localhost / ::1 (any port) are
    //! accepted. In remote mode any Host is accepted.
    bool isHostAllowed(const QByteArray& hostHeader) const;

    //! True when the server is bound to 0.0.0.0 (remote-reachable mode).
    bool isRemoteMode() const { return host == QLatin1String("0.0.0.0"); }

    QString createSessionId();
    QString extractSessionId(const QByteArray& path) const;
    void removeClient(QTcpSocket* client);
    void sendKeepAlive();

    //! The Qt TCP server that listens for incoming MCP client connections.
    QTcpServer* tcpServer;

    //! List of all currently connected client sockets.
    //! Supports multiple simultaneous AI client connections.
    QList<QTcpSocket*> clients;

    QHash<QTcpSocket*, ClientState> clientStates;
    QHash<QString, QTcpSocket*> sessionSockets;
    QTimer* keepAliveTimer;

    //! TCP port the server listens on. Default: 3719.
    //! Configured via setConfig() (called by DltMcpPlugin from the plugin's
    //! persisted XML config: <serverPort>).
    quint16 port;

    //! Host address the server binds to. Default: "127.0.0.1" (loopback only).
    //! Keeping this on loopback ensures the MCP interface is not accidentally
    //! exposed on a network interface. When the plugin's XML config sets
    //! <allowRemoteConnections>true</allowRemoteConnections>, DltMcpPlugin
    //! calls setConfig() with "0.0.0.0" instead.
    QString host;

    //! Bearer token expected in the Authorization header. Supplied by
    //! DltMcpPlugin from ~/.dlt/mcp/token.txt. If empty, isAuthorized() rejects
    //! every request (fail-closed).
    QByteArray authToken;
};

#endif // DLTMCPSERVER_H

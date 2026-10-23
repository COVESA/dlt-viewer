// Handles HTTP/SSE connections, MCP sessions, and tool discovery on the server thread.
// Forwards tool calls to DltMcpPlugin and routes its replies to the originating session.

#include "dltmcpserver.h"
#include <QDebug>
#include <QJsonArray>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>
#include <algorithm>

namespace {
constexpr qint64 kMaxHeaderBytes = 16 * 1024;
constexpr qint64 kMaxBodyBytes = 1024 * 1024;
constexpr int kMaxClients = 64;
constexpr int kRequestTimeoutMs = 30000;
}

// Initialise the server with safe defaults. The actual TCP socket is not
// created here – that happens in start(), which is called from the
// background thread once it has started its event loop.
DltMcpServer::DltMcpServer(QObject* parent)
    : QObject(parent), tcpServer(nullptr), keepAliveTimer(nullptr),
      port(3719) // Default MCP port – must not conflict with DLT daemon (port 3490)
      ,
      host("127.0.0.1") // Loopback only; keeps the MCP interface off the network
{
    qDebug() << "DltMcpServer: Constructor";
}

// Delegates all cleanup to stop() so there is a single teardown path
// regardless of whether the destructor or an explicit stop() call is used.
DltMcpServer::~DltMcpServer() {
    qDebug() << "DltMcpServer: Destructor";
    stop();
}

// Allows the plugin to override the default host and port before the server
// thread is started. Must be called before start() — calling it after the
// server is already listening has no effect on the current session.
void DltMcpServer::setConfig(const QString& h, quint16 p, const QByteArray& token) {
    host = h;
    port = p;
    authToken = token;
    if (authToken.isEmpty())
        qWarning() << "DltMcpServer: empty auth token — all requests will be rejected with 401";
    qDebug() << "DltMcpServer: Config set to" << host << ":" << port
             << "auth=" << (authToken.isEmpty() ? "reject-all" : "required");
}

// Creates the QTcpServer and starts listening on host:port.
// If listen() fails (e.g. port already in use by another DLT Viewer instance),
// the error signal is emitted and tcpServer is cleaned up so the object
// remains in a consistent state. On success, newConnection is wired to
// onNewConnection so incoming clients are handled asynchronously.
void DltMcpServer::start() {
    qDebug() << "DltMcpServer: Starting server on" << host << ":" << port;

    tcpServer = new QTcpServer(this);

    if (!tcpServer->listen(QHostAddress(host), port)) {
        emit error(QString("Failed to start server: %1").arg(tcpServer->errorString()));
        delete tcpServer;
        tcpServer = nullptr;
        return;
    }

    qDebug() << "DltMcpServer: Server started successfully";

    keepAliveTimer = new QTimer(this);
    keepAliveTimer->setInterval(15000);
    connect(keepAliveTimer, &QTimer::timeout, this, &DltMcpServer::sendKeepAlive);
    keepAliveTimer->start();

    connect(tcpServer, &QTcpServer::newConnection, this, &DltMcpServer::onNewConnection);
}

// Shuts down the listening socket and disconnects every active client.
// disconnectFromHost() initiates a graceful TCP FIN handshake; deleteLater()
// defers the actual memory release until the current event-loop iteration
// finishes, which is necessary because stop() may be called from a signal
// handler that still holds a reference to the socket.
void DltMcpServer::stop() {
    qDebug() << "DltMcpServer: Stopping server";

    if (keepAliveTimer) {
        keepAliveTimer->stop();
        keepAliveTimer->deleteLater();
        keepAliveTimer = nullptr;
    }

    if (tcpServer) {
        tcpServer->close();       // Stop accepting new connections
        tcpServer->deleteLater(); // Safe deferred delete (we're on the server thread)
        tcpServer = nullptr;
    }

    const QList<QTcpSocket*> currentClients = clients;
    for (QTcpSocket* client : currentClients) {
        client->disconnectFromHost(); // Graceful TCP shutdown
        client->deleteLater();
    }
    clients.clear();
    clientStates.clear();
    sessionSockets.clear();
}

// Qt may coalesce multiple simultaneous connections into a single
// newConnection signal, so we drain the queue with a while loop.
// For each new client we register it, wire up its signals, and initialise
// an empty per-client request buffer entry. No protocol response is sent
// here; the client must send an HTTP request first (typically opening the
// SSE stream endpoint).
void DltMcpServer::onNewConnection() {
    while (tcpServer->hasPendingConnections()) {
        QTcpSocket* client = tcpServer->nextPendingConnection();
        if (clients.size() >= kMaxClients) {
            client->abort();
            client->deleteLater();
            continue;
        }
        client->setReadBufferSize(kMaxHeaderBytes);
        clients.append(client);
        clientStates.insert(client, ClientState{});

        connect(client, &QTcpSocket::readyRead, this, &DltMcpServer::onClientReadyRead);
        connect(client, &QTcpSocket::disconnected, this, &DltMcpServer::onClientDisconnected);
        QTimer::singleShot(kRequestTimeoutMs, client, [this, client]() {
            const auto state = clientStates.constFind(client);
            if (state != clientStates.constEnd() &&
                (state->closing || !state->isEventStream))
                client->abort();
        });
    }
}

// Triggered whenever a client socket has data available to read.
void DltMcpServer::onClientReadyRead() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client)
        return;

    while (client->state() == QAbstractSocket::ConnectedState &&
           client->bytesAvailable() > 0 && clientStates.contains(client)) {
        ClientState& state = clientStates[client];
        if (state.closing)
            return;
        if (state.isEventStream) {
            client->abort();
            return;
        }
        const qint64 limit = state.headersParsed ? state.contentLength : kMaxHeaderBytes;
        const qint64 remaining = limit - state.buffer.size();
        if (remaining <= 0) {
            client->abort();
            return;
        }
        const QByteArray incoming = client->read(remaining);
        if (incoming.isEmpty())
            return;
        state.buffer += incoming;
        tryProcessBufferedRequest(client);
    }
}

// Triggered when a client closes the TCP connection (either gracefully or
// due to a network error). Removes the socket and its buffered state from all
// tracking structures, then schedules the socket for deletion.
void DltMcpServer::onClientDisconnected() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client)
        return;

    removeClient(client);
    client->deleteLater();
}

// Central dispatcher for all incoming JSON-RPC 2.0 objects.
//
// MCP protocol messages handled directly by the server:
//   initialize          – MCP handshake; respond with capabilities
//   notifications/*     – MCP notifications (no "id"); silently acknowledged
//   tools/list          – return the array of available tool descriptors
//
// Delegated to the plugin via messageReceived signal:
//   tools/call          – the plugin executes the tool and emits responseReady
//
// Everything else is answered with JSON-RPC error -32601 (Method not found).
void DltMcpServer::handleRequest(const QJsonObject& request, const QString& clientId) {
    if (!request.contains("method"))
        return; // not a request or notification — ignore

    const QString method = request["method"].toString();
    const bool isNotification = !request.contains("id");

    if (method == "initialize") {
        const QJsonObject params = request.value("params").toObject();
        const QJsonObject clientInfo = params.value("clientInfo").toObject();
        const QString clientName = clientInfo.value("name").toString().trimmed();
        const QString clientVersion = clientInfo.value("version").toString().trimmed();
        QTcpSocket* socket = sessionSockets.value(clientId, nullptr);
        if (socket && clientStates.contains(socket)) {
            ClientState& clientState = clientStates[socket];
            if (!clientName.isEmpty())
                clientState.clientName = clientName;
            if (!clientVersion.isEmpty())
                clientState.clientVersion = clientVersion;
            emit clientSessionChanged();
        }

        sendSseEvent(clientId, "message", handleInitialize(request));
    } else if (method.startsWith("notifications/")) {
        // MCP notifications require no response.
    } else if (method == "tools/list") {
        sendSseEvent(clientId, "message", handleToolsList(request));
    } else if (method == "tools/call") {
        emit messageReceived(clientId, request);
    } else if (!isNotification) {
        // Unknown method with an id — return a proper JSON-RPC error.
        QJsonObject response;
        response["jsonrpc"] = "2.0";
        response["id"] = request["id"];
        response["error"] = QJsonObject{
            {{"code", -32601}, {"message", QString("Method not found: %1").arg(method)}}};
        sendSseEvent(clientId, "message", response);
    }
}

// Responds to the MCP "initialize" request with the server's identity and
// capability declaration. The "tools" capability signals that this server
// supports tools/list and tools/call. Also includes session expiration guidance.
QJsonObject DltMcpServer::handleInitialize(const QJsonObject& request) const {
    QJsonObject result;
    result["protocolVersion"] = "2024-11-05";

    QJsonObject serverInfo;
    serverInfo["name"] = "DLT Viewer MCP Server";
    serverInfo["version"] = "1.0.0";
    result["serverInfo"] = serverInfo;

    // Declare that we support the "tools" capability so the MCP client
    // knows it can call tools/list and tools/call.
    QJsonObject capabilities;
    capabilities["tools"] = QJsonObject{};
    result["capabilities"] = capabilities;

    // Inform the AI about session expiration and reconnection requirements
    QJsonObject sessionInfo;
    sessionInfo["expires"] = false; // Sessions don't auto-expire, but can be lost
    sessionInfo["volatile"] = true; // Sessions are lost if DLT Viewer closes or disconnects
    sessionInfo["reconnectRequired"] = true; // Client must reconnect if session is lost
    sessionInfo["invalidatedWhen"] = "SSE connection closes or server restarts";
    sessionInfo["reconnectHint"] =
        "If you receive HTTP 410 'Gone' responses or lose connection, "
        "create a new SSE connection to / or /mcp to obtain a fresh session. "
        "The session ID provided in the SSE endpoint is required for all "
        "subsequent tool calls.";
    result["sessionInfo"] = sessionInfo;

    QJsonObject response;
    response["jsonrpc"] = "2.0";
    response["id"] = request["id"];
    response["result"] = result;
    return response;
}

// Responds to "tools/list" with the full schema of every tool this server
// exposes. MCP clients use these schemas to know which tools exist, what
// parameters they accept, and what they do.
QJsonObject DltMcpServer::handleToolsList(const QJsonObject& request) const {
    QJsonObject result;
    result["tools"] = buildToolsArray();

    QJsonObject response;
    response["jsonrpc"] = "2.0";
    response["id"] = request["id"];
    response["result"] = result;
    return response;
}

// Builds the array of tool descriptors.
// Each entry follows the MCP tool schema:
//   name        – identifier used in tools/call
//   description – shown to the AI so it knows when to use the tool
//   inputSchema – JSON Schema describing the accepted parameters
QJsonArray DltMcpServer::buildToolsArray() const {
    QJsonArray tools;

    // --- get_context ---
    {
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = QJsonObject{};

        QJsonObject tool;
        tool["name"] = "get_context";
        tool["description"] =
            "Loaded trace, filter state, sampled message overview (types, ECUs, APIDs, CTIDs). "
            "Explains use_filter scope, time/timestamp mapping for all tools. Call first.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- get_dictionary ---
    {
        QJsonObject props;
        props["term"] = QJsonObject{
            {"type", "string"},
            {"description",
             "Optional DLT term to explain, for example \"trace\", \"APID\", "
             "\"CTID\", or \"filtered view\". If omitted, returns the full dictionary."}};

        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = props;

        QJsonObject tool;
        tool["name"] = "get_dictionary";
        tool["description"] =
            "DLT terminology dictionary. Optional term parameter for focused lookup.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- get_file_info ---
    {
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = QJsonObject{}; // no parameters

        QJsonObject tool;
        tool["name"] = "get_file_info";
        tool["description"] =
            "Total and filtered message counts. Call first to understand dataset size.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- get_messages_range ---
    {
        QJsonObject props;
        props["start"] =
            QJsonObject{{"type", "integer"},
                        {"description", "First message index (0-based, inclusive). "
                                        "Refers to the filtered view when use_filter=true, "
                                        "or the full file when use_filter=false."}};
        props["end"] = QJsonObject{
            {"type", "integer"},
            {"description", "Last message index (exclusive). Maximum 500 messages per call."}};
        props["use_filter"] = QJsonObject{
            {"type", "boolean"},
            {"description", "If true, indices follow the currently active DLT Viewer "
                            "scope: the filtered view when a filter is active, otherwise the "
                            "full log. If false, indices refer to the full unfiltered file "
                            "even when a filter is active."}};
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = props;
        schema["required"] = QJsonArray{"start", "end"};

        QJsonObject tool;
        tool["name"] = "get_messages_range";
        tool["description"] = "Slice of messages (max 500). use_filter=true: respect active "
                              "filters; false: full file. "
                              "Results: time/wall_clock_time (wall-clock), timestamp (ECU "
                              "seconds), timestamp_raw (0.1 ms).";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- search_logs ---
    {
        QJsonObject props;
        props["pattern"] = QJsonObject{
            {"type", "string"},
            {"description",
             "Optional case-insensitive regular expression to match against decoded log message "
             "payloads. Leave empty when searching only by header fields."}};
        props["start_index"] = QJsonObject{
            {"type", "integer"},
            {"description",
             "Optional 0-based index in the selected scope to start scanning from. To resume a "
             "partial search, pass its next_index here unchanged."}};
        props["max_scan"] = QJsonObject{
            {"type", "integer"},
            {"description", "Optional maximum number of message indices to examine in this call. Defaults to "
                            "the full filtered scope, or 10000 for unfiltered searches to avoid "
                            "MCP timeouts on large traces. "
                            "Tuning guide: scanning speed is typically 50000-200000 "
                            "messages/second depending on payload decoding cost. "
                            "For a 60-second client timeout, a safe upper bound is ~500000; for a "
                            "30-second timeout use ~200000. "
                            "When in doubt, keep the default (10000) and use "
                            "start_index/next_index to resume if needed."}};
        props["max_results"] = QJsonObject{
            {"type", "integer"},
            {"description",
             "Maximum number of matching messages to return. Default: 100. Maximum: 200. "
             "If expected matches exceed this cap, additional matches are not returned in this "
             "call. "
             "Narrow filters/window or continue with start_index/next_index for broader "
             "coverage."}};
        props["ecuid"] = QJsonObject{
            {"type", "string"},
            {"description",
             "Optional ECU ID filter. Matches the DLT header ECU ID exactly, case-insensitive."}};
        props["apid"] =
            QJsonObject{{"type", "string"},
                        {"description", "Optional APID/App ID filter. Matches the DLT header "
                                        "application ID exactly, case-insensitive."}};
        props["ctid"] =
            QJsonObject{{"type", "string"},
                        {"description", "Optional CTID/Context ID filter. Matches the DLT header "
                                        "context ID exactly, case-insensitive."}};
        props["type"] = QJsonObject{
            {"type", "string"},
            {"description",
             "Optional DLT message type filter, for example Log, AppTrace, NwTrace, or Control."}};
        props["subtype"] = QJsonObject{
            {"type", "string"},
            {"description", "Optional DLT subtype filter. For Log messages this is the log "
                            "subtype/level such as Fatal, Error, Warn, Info, Debug, or Verbose."}};
        props["mode"] = QJsonObject{
            {"type", "string"},
            {"description", "Optional DLT mode filter, for example Verbose or Non-Verbose."}};
        props["session_id"] =
            QJsonObject{{"type", "string"},
                        {"description", "Optional DLT SessionId filter. Match using the numeric "
                                        "SessionId value as a string."}};
        props["use_filter"] = QJsonObject{
            {"type", "boolean"},
            {"description", "If true, search follows the currently active DLT Viewer scope: "
                            "the filtered view when a filter is active, otherwise the full "
                            "log. If false, search the entire unfiltered file even when a "
                            "filter is active."}};
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = props;

        QJsonObject tool;
        tool["name"] = "search_logs";
        tool["description"] =
            "Search by payload regex and/or header fields "
            "(ecuid/apid/ctid/type/subtype/mode/session_id). "
            "max_results default 100, max 200. use_filter: filtered view (true) or full file "
            "(false). "
            "Indexes are relative to the selected scope. If completed=false, resume by passing "
            "next_index unchanged as start_index; it is the first index not yet examined. "
            "completed=true only when the end of the selected scope is reached. Reaching max_scan "
            "or max_results stops this call; if completed=false, continue from next_index. "
            "Use subtype for severity (e.g. Warn). "
            "IMPORTANT: tune max_scan to find the sweet spot with max_results (default 100). "
            "Too small -> matches past the scan window require another call; when max_results is "
            "reached, remaining indexes are unexamined and can be searched by resuming at "
            "next_index. "
            "Strategy: scan widely with tight filters (regex + ecuid/apid/ctid) so true match "
            "count stays <= max_results; "
            "if still capped, narrow filters or paginate via start_index/next_index. "
            "Also keep max_scan small enough to finish within the client timeout (~60 s; ~50k-200k "
            "msgs/s decode rate). "
            "Concurrency: latest request wins (a new search_logs or get_error_messages cancels a "
            "running search_logs).";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- find_message_by_timestamp ---
    {
        QJsonObject props;
        props["timestamp"] =
            QJsonObject{{"type", "string"},
                        {"description",
                         "Wall-clock timestamp to anchor in the trace. Supported formats: "
                         "yyyy/MM/dd HH:mm:ss[.zzz] or HH:mm:ss[.zzz]. Time-only input is matched "
                         "against the trace time-of-day, which is the common use case."}};
        props["use_filter"] =
            QJsonObject{{"type", "boolean"},
                        {"description",
                         "If true, search within the currently active DLT Viewer scope: the "
                         "filtered view when a filter is active, otherwise the full log. If false, "
                         "search the full unfiltered file even when a filter is active."}};
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = props;
        schema["required"] = QJsonArray{"timestamp"};

        QJsonObject tool;
        tool["name"] = "find_message_by_timestamp";
        tool["description"] = "Find message nearest to wall-clock timestamp. Input: wall-clock "
                              "time (yyyy/MM/dd HH:mm:ss or HH:mm:ss). "
                              "Output includes global_index, time, timestamp (ECU seconds), "
                              "timestamp_raw (0.1 ms). Use global_index before "
                              "search_logs_around_index or navigate_to_message.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- search_logs_around_index ---
    {
        QJsonObject props;
        props["index"] =
            QJsonObject{{"type", "integer"},
                        {"description",
                         "Anchor global message index to search around. Use global_index from "
                         "find_message_by_timestamp or search results."}};
        props["pattern"] = QJsonObject{
            {"type", "string"},
            {"description", "Optional case-insensitive regular expression to match against decoded "
                            "log message payloads. Leave empty when searching only by header "
                            "fields around the anchor index."}};
        props["ecuid"] = QJsonObject{
            {"type", "string"},
            {"description",
             "Optional ECU ID filter. Matches the DLT header ECU ID exactly, case-insensitive."}};
        props["apid"] =
            QJsonObject{{"type", "string"},
                        {"description", "Optional APID/App ID filter. Matches the DLT header "
                                        "application ID exactly, case-insensitive."}};
        props["ctid"] =
            QJsonObject{{"type", "string"},
                        {"description", "Optional CTID/Context ID filter. Matches the DLT header "
                                        "context ID exactly, case-insensitive."}};
        props["type"] = QJsonObject{
            {"type", "string"},
            {"description",
             "Optional DLT message type filter, for example Log, AppTrace, NwTrace, or Control."}};
        props["subtype"] = QJsonObject{
            {"type", "string"},
            {"description", "Optional DLT subtype filter. For Log messages this is the log "
                            "subtype/level such as Fatal, Error, Warn, Info, Debug, or Verbose."}};
        props["mode"] = QJsonObject{
            {"type", "string"},
            {"description", "Optional DLT mode filter, for example Verbose or Non-Verbose."}};
        props["session_id"] =
            QJsonObject{{"type", "string"},
                        {"description", "Optional DLT SessionId filter. Match using the numeric "
                                        "SessionId value as a string."}};
        props["window_before_ms"] = QJsonObject{
            {"type", "integer"},
            {"description", "Milliseconds before the anchor message's monotonic timestamp to "
                            "include in the search window. Default: 5000."}};
        props["window_after_ms"] = QJsonObject{
            {"type", "integer"},
            {"description", "Milliseconds after the anchor message's monotonic timestamp to "
                            "include in the search window. Default: 5000."}};
        props["use_filter"] =
            QJsonObject{{"type", "boolean"},
                        {"description",
                         "If true, search within the currently active DLT Viewer scope: the "
                         "filtered view when a filter is active, otherwise the full log. If false, "
                         "search the full unfiltered file even when a filter is active."}};
        props["max_results"] = QJsonObject{
            {"type", "integer"},
            {"description",
             "Maximum number of matching messages to return. Default: 100. Maximum: 200. "
             "If your selected window and filters would produce more matches than this cap, "
             "additional matches are not returned in this call. "
             "Narrow window_before_ms/window_after_ms and/or tighten filters so expected hits stay "
             "below max_results."}};
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = props;
        schema["required"] = QJsonArray{"index"};

        QJsonObject tool;
        tool["name"] = "search_logs_around_index";
        tool["description"] =
            "Search by regex/headers around anchor index within time window (default ±5000 ms). "
            "Max 200 results. Use find_message_by_timestamp first to convert wall-clock time to "
            "a global index. "
            "Result-count tradeoff: keep max_results low enough for AI context quality (default "
            "100), but ensure your "
            "window and filters are narrow enough that true matches stay below max_results; "
            "otherwise later matches are dropped. "
            "If capped, reduce window_before_ms/window_after_ms and/or add tighter header filters "
            "before retrying.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- get_error_messages ---
    {
        QJsonObject props;
        props["use_filter"] = QJsonObject{
            {"type", "boolean"},
            {"description", "If true, scanning follows the currently active DLT Viewer "
                            "scope: the filtered view when a filter is active, otherwise the "
                            "full log. If false, scan the entire unfiltered file even when a "
                            "filter is active."}};
        props["max_results"] =
            QJsonObject{{"type", "integer"},
                        {"description", "Maximum number of matching error/warning messages to "
                                        "return. Default: 100. Maximum: 200."}};
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = props;

        QJsonObject tool;
        tool["name"] = "get_error_messages";
        tool["description"] =
            "Fatal/Error/Warning level messages. max_results default 100, maximum 200. "
            "use_filter: filtered (true) or full file (false). "
            "Result-count tradeoff: keep max_results as low as practical to avoid overloading AI "
            "context. "
            "If you need exhaustive error coverage on large traces, use search_logs with subtype "
            "(Fatal/Error/Warn) and paginate "
            "with start_index/max_scan/next_index instead of relying on one capped summary call. "
            "Concurrency behavior: latest request wins. A new get_error_messages cancels any "
            "running get_error_messages and also cancels a running search_logs.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- list_filters ---
    {
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = QJsonObject{};

        QJsonObject tool;
        tool["name"] = "list_filters";
        tool["description"] = "List configured filters with index, type "
                              "(positive/negative/marker), criteria, enabled state.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- add_filter ---
    {
        QJsonObject props;
        props["type"] =
            QJsonObject{{"type", "string"},
                        {"description",
                         "Filter type: \"positive\" (show only matching messages), "
                         "\"negative\" (hide matching messages), "
                         "\"marker\" (highlight matching messages without affecting visibility). "
                         "Defaults to \"positive\"."}};
        props["name"] = QJsonObject{
            {"type", "string"}, {"description", "Optional human-readable name for the filter."}};
        props["ecuid"] = QJsonObject{
            {"type", "string"},
            {"description", "ECU ID to match (e.g. \"ECU1\"). Leave empty to match any ECU."}};
        props["apid"] = QJsonObject{
            {"type", "string"},
            {"description", "Application ID to match (e.g. \"MYAP\"). Leave empty to match any."}};
        props["ctid"] = QJsonObject{
            {"type", "string"},
            {"description", "Context ID to match (e.g. \"MYCT\"). Leave empty to match any."}};
        props["header"] = QJsonObject{
            {"type", "string"},
            {"description", "Text to match in the message header. Leave empty to skip."}};
        props["payload"] = QJsonObject{
            {"type", "string"},
            {"description", "Text to match in the message payload. Leave empty to skip."}};
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = props;

        QJsonObject tool;
        tool["name"] = "add_filter";
        tool["description"] = "Add filter (positive/negative/marker) with "
                              "ecuid/apid/ctid/header/payload criteria. Rebuilds view. "
                              "On large traces, may reject to avoid freeze. Use use_filter=false "
                              "for temporary unfiltered analysis.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- remove_filter ---
    {
        QJsonObject props;
        props["index"] = QJsonObject{
            {"type", "integer"},
            {"description", "Index of the filter to remove, as returned by list_filters."}};
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = props;
        schema["required"] = QJsonArray{"index"};

        QJsonObject tool;
        tool["name"] = "remove_filter";
        tool["description"] = "Remove filter by index (from list_filters). Rebuilds view. On large "
                              "traces, may reject to avoid freeze.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- set_filter_enabled ---
    {
        QJsonObject props;
        props["index"] =
            QJsonObject{{"type", "integer"},
                        {"description",
                         "Index of the filter to enable or disable, as returned by list_filters."}};
        props["enabled"] = QJsonObject{
            {"type", "boolean"},
            {"description",
             "true to activate the filter, false to deactivate it without removing it."}};
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = props;
        schema["required"] = QJsonArray{"index", "enabled"};

        QJsonObject tool;
        tool["name"] = "set_filter_enabled";
        tool["description"] =
            "Enable/disable filter by index (from list_filters) without removing. Rebuilds view. "
            "Prefer use_filter=false for temporary bypass on read/search tools.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- list_connections ---
    {
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = QJsonObject{};

        QJsonObject tool;
        tool["name"] = "list_connections";
        tool["description"] =
            "List ECU connections with indices. Use indices with connect_ecu/disconnect_ecu.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- get_connection_states ---
    {
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = QJsonObject{};

        QJsonObject tool;
        tool["name"] = "get_connection_states";
        tool["description"] =
            "Current state of each ECU connection (offline/connecting/online/error).";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- connect_ecu ---
    {
        QJsonObject props;
        props["index"] = QJsonObject{{"type", "integer"},
                                     {"description", "Index of the configured ECU connection to "
                                                     "connect, as returned by list_connections."}};
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = props;
        schema["required"] = QJsonArray{"index"};

        QJsonObject tool;
        tool["name"] = "connect_ecu";
        tool["description"] =
            "Connect ECU by index (from list_connections). Start receiving messages.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- disconnect_ecu ---
    {
        QJsonObject props;
        props["index"] =
            QJsonObject{{"type", "integer"},
                        {"description", "Index of the configured ECU connection to disconnect, as "
                                        "returned by list_connections."}};
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = props;
        schema["required"] = QJsonArray{"index"};

        QJsonObject tool;
        tool["name"] = "disconnect_ecu";
        tool["description"] =
            "Disconnect ECU by index (from list_connections). Stop receiving messages.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- connect_all_ecus ---
    {
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = QJsonObject{};

        QJsonObject tool;
        tool["name"] = "connect_all_ecus";
        tool["description"] = "Connect all configured ECUs. Start logging from all targets.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- disconnect_all_ecus ---
    {
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = QJsonObject{};

        QJsonObject tool;
        tool["name"] = "disconnect_all_ecus";
        tool["description"] = "Disconnect all configured ECUs. Stop logging from all targets.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- navigate_to_message ---
    {
        QJsonObject props;
        props["index"] =
            QJsonObject{{"type", "integer"},
                        {"description", "Global message index to navigate to in DLT Viewer"}};
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = props;
        schema["required"] = QJsonArray{"index"};

        QJsonObject tool;
        tool["name"] = "navigate_to_message";
        tool["description"] = "Jump DLT Viewer table to global message index. Use global_index "
                              "from search_logs, get_error_messages, or "
                              "find_message_by_timestamp.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // --- clear_logs ---
    {
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = QJsonObject{};

        QJsonObject tool;
        tool["name"] = "clear_logs";
        tool["description"] = "Clear all messages from the current DLT Viewer trace view.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    return tools;
}

// Runs on the server thread via the plugin's queued responseReady signal,
// sending a completed JSON-RPC response back to the originating client.
// Looks up the active SSE socket by session id. If the client has already
// disconnected the call is silently ignored.
void DltMcpServer::sendResponseToClient(const QString& clientId, const QJsonObject& response) {
    if (!sessionSockets.contains(clientId)) {
        qDebug() << "DltMcpServer: sendResponseToClient – client already gone, dropping response";
        return;
    }
    sendSseEvent(clientId, "message", response);
}

void DltMcpServer::sendHttpResponse(QTcpSocket* socket, int statusCode,
                                    const QByteArray& statusText, const QByteArray& contentType,
                                    const QByteArray& body, const QList<QByteArray>& extraHeaders) {
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    QByteArray header = "HTTP/1.1 " + QByteArray::number(statusCode) + " " + statusText + "\r\n";
    header += "Content-Type: " + contentType + "\r\n";
    for (const QByteArray& extraHeader : extraHeaders) {
        header += extraHeader + "\r\n";
    }
    if (!body.isEmpty()) {
        header += "Content-Length: " + QByteArray::number(body.size()) + "\r\n";
    }
    header += "\r\n";

    socket->write(header);
    if (!body.isEmpty()) {
        socket->write(body);
    }
    // Note: do not call flush() here; let Qt's event loop manage socket draining asynchronously.
    // Calling flush() can block the current server thread on slow networks.
}

void DltMcpServer::sendSseEvent(QTcpSocket* socket, const QByteArray& eventName,
                                const QByteArray& payload) {
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    QByteArray event;
    event += "event: " + eventName + "\n";
    event += "data: " + payload + "\n\n";
    socket->write(event);
    // Do not flush; let Qt's event loop handle asynchronous transmission
}

void DltMcpServer::sendSseEvent(QTcpSocket* socket, const QByteArray& eventName,
                                const QJsonObject& payload) {
    sendSseEvent(socket, eventName, QJsonDocument(payload).toJson(QJsonDocument::Compact));
}

void DltMcpServer::sendSseEvent(const QString& clientId, const QByteArray& eventName,
                                const QJsonObject& payload) {
    QTcpSocket* socket = sessionSockets.value(clientId, nullptr);
    sendSseEvent(socket, eventName, payload);
}

void DltMcpServer::sendSseComment(QTcpSocket* socket, const QByteArray& comment) {
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    socket->write(":" + comment + "\n\n");
    // Do not flush; let Qt's event loop handle asynchronous transmission
}

// Parses one HTTP request per socket, retaining partial headers/bodies between reads.
// POST requires Content-Length; chunked transfer and pipelining are unsupported.
// Size limits and Host/Origin/token checks are enforced before waiting for the body.
bool DltMcpServer::tryProcessBufferedRequest(QTcpSocket* client) {
    if (!clientStates.contains(client)) {
        return false;
    }

    ClientState& state = clientStates[client];
    if (state.closing || state.isEventStream)
        return false;

    const auto reject = [&](int code, const QByteArray& status, const QByteArray& message) {
        state.closing = true;
        state.buffer.clear();
        QList<QByteArray> headers{"Connection: close"};
        if (code == 401)
            headers.append("WWW-Authenticate: Bearer realm=\"dlt-viewer-mcp\"");
        sendHttpResponse(client, code, status, "text/plain", message, headers);
        client->disconnectFromHost();
        return false;
    };

    if (!state.headersParsed) {
        const int headerEnd = state.buffer.indexOf("\r\n\r\n");
        if (headerEnd < 0) {
            if (state.buffer.size() >= kMaxHeaderBytes)
                return reject(431, "Request Header Fields Too Large", "Request headers too large");
            return false;
        }
        if (headerEnd + 4 > kMaxHeaderBytes)
            return reject(431, "Request Header Fields Too Large", "Request headers too large");

        const QByteArray headerBlock = state.buffer.left(headerEnd);
        state.buffer = state.buffer.mid(headerEnd + 4);

        const QList<QByteArray> lines = headerBlock.split('\n');
        if (lines.isEmpty()) {
            return reject(400, "Bad Request", "Invalid HTTP request");
        }

        const QList<QByteArray> requestLineParts = lines.first().trimmed().split(' ');
        if (requestLineParts.size() < 2) {
            return reject(400, "Bad Request", "Invalid HTTP request line");
        }

        state.method = requestLineParts.at(0).trimmed().toUpper();
        state.path = requestLineParts.at(1).trimmed();
        state.headers.clear();
        state.contentLength = 0;
        state.body.clear();

        for (int i = 1; i < lines.size(); ++i) {
            const QByteArray line = lines.at(i).trimmed();
            const int colonPos = line.indexOf(':');
            if (colonPos <= 0) {
                return reject(400, "Bad Request", "Invalid HTTP header");
            }
            const QByteArray key = line.left(colonPos).trimmed().toLower();
            const QByteArray value = line.mid(colonPos + 1).trimmed();
            if (state.headers.contains(key) &&
                (key == "content-length" || key == "host" || key == "origin" ||
                 key == "authorization"))
                return reject(400, "Bad Request", "Duplicate request header");
            state.headers.insert(key, value);
            if (key == "content-length") {
                bool ok = false;
                const qint64 length = value.toLongLong(&ok);
                if (!ok || value.isEmpty() ||
                    !std::all_of(value.cbegin(), value.cend(),
                                 [](char digit) { return digit >= '0' && digit <= '9'; }))
                    return reject(400, "Bad Request", "Invalid Content-Length");
                if (length > kMaxBodyBytes)
                    return reject(413, "Payload Too Large", "Request body too large");
                state.contentLength = length;
            }
            if (key == "transfer-encoding")
                return reject(501, "Not Implemented", "Use Content-Length instead of Transfer-Encoding");
        }
        if (state.method == "POST" && !state.headers.contains("content-length"))
            return reject(411, "Length Required", "POST requires Content-Length");
        if (state.method != "POST" && state.contentLength != 0)
            return reject(400, "Bad Request", "Request body is only supported for POST");
        if (!isHostAllowed(state.headers.value("host")))
            return reject(403, "Forbidden", "Host header not allowed");
        if (!isOriginAllowed(state.headers.value("origin")))
            return reject(403, "Forbidden", "Origin not allowed");
        if (state.method != "OPTIONS" && !isAuthorized(state.headers.value("authorization")))
            return reject(401, "Unauthorized", "Missing or invalid bearer token");
        state.headersParsed = true;
    }

    if (state.buffer.size() > state.contentLength)
        return reject(400, "Bad Request", "Unexpected data after request");
    if (state.method == "POST" && state.buffer.size() < state.contentLength) {
        return false;
    }

    if (state.method == "POST" && state.contentLength > 0) {
        state.body = state.buffer.left(state.contentLength);
        state.buffer = state.buffer.mid(state.contentLength);
    }

    state.closing = true;
    handleHttpRequest(client, state);

    if (!clientStates.contains(client)) {
        return false;
    }

    clientStates[client].buffer.clear();
    clientStates[client].body.clear();

    return false;
}

bool DltMcpServer::isOriginAllowed(const QByteArray& origin) const {
    if (origin.isEmpty())
        return true;
    // Remote mode accepts any Origin; add config-driven allowlist later if needed.
    return isRemoteMode();
}

bool DltMcpServer::isAuthorized(const QByteArray& authHeader) const {
    if (authToken.isEmpty())
        return false;

    const QByteArray value = authHeader.trimmed();
    if (value.size() < 7 || value.left(7).toLower() != "bearer ")
        return false;
    const QByteArray presented = value.mid(7).trimmed();
    if (presented.size() != authToken.size())
        return false;

    unsigned char diff = 0;
    for (int i = 0; i < presented.size(); ++i)
        diff |= static_cast<unsigned char>(presented[i]) ^
                static_cast<unsigned char>(authToken[i]);
    return diff == 0;
}

bool DltMcpServer::isHostAllowed(const QByteArray& hostHeader) const {
    // Remote mode accepts any Host; add config-driven allowlist later if needed.
    if (isRemoteMode())
        return true;
    if (hostHeader.isEmpty())
        return false;

    QByteArray h = hostHeader.trimmed();
    if (h.startsWith('[')) {
        const int closing = h.indexOf(']');
        if (closing < 0)
            return false;
        return h.mid(1, closing - 1).trimmed() == "::1";
    }
    const int colon = h.lastIndexOf(':');
    if (colon >= 0)
        h = h.left(colon);
    h = h.toLower();
    return h == "127.0.0.1" || h == "localhost";
}

// GET creates the SSE session and advertises its POST endpoint. POST receives only
// an HTTP acceptance response; JSON-RPC replies use the existing SSE session.
// Session IDs are routing keys, not credentials: bearer authentication is separate.
void DltMcpServer::handleHttpRequest(QTcpSocket* socket, const ClientState& state) {
    const QByteArray originHeader = state.headers.value("origin");
    const QByteArray hostHeader = state.headers.value("host");

    if (!isHostAllowed(hostHeader)) {
        qWarning() << "DltMcpServer: rejecting request with disallowed Host:" << hostHeader;
        sendHttpResponse(socket, 403, "Forbidden", "text/plain", "Host header not allowed");
        socket->disconnectFromHost();
        return;
    }

    if (!isOriginAllowed(originHeader)) {
        qWarning() << "DltMcpServer: rejecting request with disallowed Origin:" << originHeader;
        sendHttpResponse(socket, 403, "Forbidden", "text/plain", "Origin not allowed");
        socket->disconnectFromHost();
        return;
    }

    if (state.method == "OPTIONS") {
        QList<QByteArray> preflightHeaders{
            "Allow: GET, POST, OPTIONS",
            "Access-Control-Allow-Methods: GET, POST, OPTIONS",
            "Access-Control-Allow-Headers: Content-Type, Authorization",
            "Vary: Origin"};
        if (!originHeader.isEmpty())
            preflightHeaders.append("Access-Control-Allow-Origin: " + originHeader);
        sendHttpResponse(socket, 204, "No Content", "text/plain", QByteArray(), preflightHeaders);
        socket->disconnectFromHost();
        return;
    }

    if (!isAuthorized(state.headers.value("authorization"))) {
        qWarning() << "DltMcpServer: rejecting unauthorized request from"
                   << socket->peerAddress().toString();
        sendHttpResponse(
            socket, 401, "Unauthorized", "text/plain", "Missing or invalid bearer token",
            {"WWW-Authenticate: Bearer realm=\"dlt-viewer-mcp\"", "Connection: close"});
        socket->disconnectFromHost();
        return;
    }

    if (state.method == "GET" && (state.path == "/" || state.path.startsWith("/?") ||
                                  state.path == "/mcp" || state.path.startsWith("/mcp?"))) {
        const QString sessionId = createSessionId();
        ClientState& clientState = clientStates[socket];
        clientState.sessionId = sessionId;
        clientState.isEventStream = true;
        clientState.closing = false;
        clientState.userAgent = QString::fromUtf8(state.headers.value("user-agent")).trimmed();

        sessionSockets[sessionId] = socket;
        emit clientSessionChanged();
        sendHttpResponse(
            socket, 200, "OK", "text/event-stream", QByteArray(),
            {"Cache-Control: no-cache", "Connection: keep-alive", "X-Accel-Buffering: no"});

        const bool rootEndpoint = (state.path == "/" || state.path.startsWith("/?"));
        QUrl endpointUrl;
        endpointUrl.setScheme(QStringLiteral("http"));
        endpointUrl.setHost(socket->localAddress().toString());
        endpointUrl.setPort(socket->localPort());
        endpointUrl.setPath(rootEndpoint ? QStringLiteral("/message")
                         : QStringLiteral("/mcp/message"));
        QUrlQuery endpointQuery;
        endpointQuery.addQueryItem(QStringLiteral("sessionId"), sessionId);
        endpointUrl.setQuery(endpointQuery);
        sendSseEvent(socket, "endpoint", endpointUrl.toString(QUrl::FullyEncoded).toUtf8());
        return;
    }

    if (state.method == "POST" &&
        (state.path == "/" || state.path.startsWith("/?") || state.path.startsWith("/message") ||
         state.path == "/mcp" || state.path.startsWith("/mcp/message"))) {
        const QString sessionId = extractSessionId(state.path);
        if (sessionId.isEmpty()) {
            sendHttpResponse(
                socket, 400, "Bad Request", "application/json",
                QByteArray("{\"error\":{\"code\":\"missing_session_id\",\"message\":\"The request "
                           "did not include a sessionId query parameter.\",\"retryable\":false}}"),
                {"Cache-Control: no-store", "Connection: close"});
            socket->disconnectFromHost();
            return;
        }
        if (!sessionSockets.contains(sessionId)) {
            sendHttpResponse(
                socket, 410, "Gone", "application/json",
                QByteArray("{\"error\":{\"code\":\"session_expired\",\"message\":\"The MCP session "
                           "is no longer valid. Create a new session by reconnecting to the SSE "
                           "endpoint.\",\"hint\":\"Open a new GET connection to / or /mcp to "
                           "receive a fresh sessionId.\",\"retryable\":true}}"),
                {"Cache-Control: no-store", "Connection: close"});
            socket->disconnectFromHost();
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(state.body, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            sendHttpResponse(socket, 400, "Bad Request", "text/plain", "Invalid JSON body");
            socket->disconnectFromHost();
            return;
        }

        sendHttpResponse(socket, 202, "Accepted", "text/plain", QByteArray(),
                         {"Connection: close"});
        socket->disconnectFromHost();
        handleRequest(doc.object(), sessionId);
        return;
    }

    sendHttpResponse(socket, 404, "Not Found", "text/plain", "Not Found");
    socket->disconnectFromHost();
}

QString DltMcpServer::createSessionId() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString DltMcpServer::extractSessionId(const QByteArray& path) const {
    const QUrl url = QUrl::fromEncoded(QByteArray("http://localhost") + path);
    const QUrlQuery query(url);
    return query.queryItemValue("sessionId");
}

QJsonArray DltMcpServer::getConnectedClients() const {
    QJsonArray snapshot;

    QStringList sessionIds = sessionSockets.keys();
    sessionIds.sort();

    for (int i = 0; i < sessionIds.size(); ++i) {
        const QString sessionId = sessionIds.at(i);
        QTcpSocket* socket = sessionSockets.value(sessionId, nullptr);
        if (!socket || !clientStates.contains(socket))
            continue;

        const ClientState state = clientStates.value(socket);

        QJsonObject client;
        client["sessionId"] = sessionId;
        client["peerAddress"] = socket->peerAddress().toString();
        client["peerPort"] = static_cast<int>(socket->peerPort());
        if (!state.userAgent.isEmpty())
            client["userAgent"] = state.userAgent;
        if (!state.clientName.isEmpty())
            client["clientName"] = state.clientName;
        if (!state.clientVersion.isEmpty())
            client["clientVersion"] = state.clientVersion;

        snapshot.append(client);
    }

    return snapshot;
}

void DltMcpServer::removeClient(QTcpSocket* client) {
    clients.removeAll(client);
    if (!clientStates.contains(client)) {
        return;
    }

    const QString sessionId = clientStates.value(client).sessionId;
    const bool wasSession = !sessionId.isEmpty();
    if (wasSession) {
        sessionSockets.remove(sessionId);
    }
    clientStates.remove(client);
    if (wasSession) {
        emit clientSessionChanged();
    }
}

void DltMcpServer::sendKeepAlive() {
    const QList<QTcpSocket*> currentClients = clients;
    for (QTcpSocket* client : currentClients) {
        if (!clientStates.contains(client)) {
            continue;
        }
        if (!clientStates.value(client).isEventStream) {
            continue;
        }
        sendSseComment(client, "keepalive");
    }
}

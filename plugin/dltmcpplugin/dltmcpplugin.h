#ifndef DLTMCPPLUGIN_H
#define DLTMCPPLUGIN_H

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QRegularExpression>
#include <QTableView>
#include <QThread>
#include <QTimer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "dltmcpplugin_tools_common.h"
#include "plugininterface.h"
#include "qdltcontrol.h"
#include "qdltfile.h"
#include "qdltmessagedecoder.h"
#include "qdltmsg.h"

#define DLT_MCP_PLUGIN_VERSION "1.0.0"

//! Forward declaration of the MCP server class.
//! The server runs in its own thread and handles all network communication
//! with MCP clients using HTTP/SSE transport carrying JSON-RPC 2.0 messages.
class DltMcpServer;
class QPlainTextEdit;
class QWidget;

//! Main MCP plugin class for DLT Viewer.
//!
//! This plugin implements the Model Context Protocol (MCP) server inside DLT Viewer,
//! allowing any MCP-compatible AI assistant to query and navigate DLT log data
//! in real time via an HTTP/SSE endpoint using JSON-RPC 2.0 tool calls.
//!
//! It implements:
//!   - QDLTPluginInterface        : mandatory base interface for all DLT Viewer plugins
//!   - QDltPluginViewerInterface  : provides the QDltFile* handle via initFileStart(),
//!                                  giving access to the active DLT view data
//!   - QDltPluginControlInterface : gives access to QDltControl (send injections,
//!                                  open/close files, connect/disconnect ECUs) plus
//!                                  control-plugin callbacks (connection state,
//!                                  autoscroll state, and control/service messages)
class DltMcpPlugin : public QObject,
                     public QDLTPluginInterface,
                     public QDltPluginViewerInterface,
                     public QDltPluginControlInterface {
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
    Q_INTERFACES(QDltPluginControlInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.genivi.DLT.DltMcpPlugin")
#endif

public:
    DltMcpPlugin();
    ~DltMcpPlugin();

    /* QDLTPluginInterface interface */

    //! Returns the display name of the plugin shown in the DLT Viewer plugin list.
    QString name();

    //! Returns the plugin's own version string (DLT_MCP_PLUGIN_VERSION).
    QString pluginVersion();

    //! Returns the version of the DLT plugin interface this plugin was built against.
    QString pluginInterfaceVersion();

    //! Returns a human-readable description of what this plugin does.
    QString description();

    //! Returns the last error message, or an empty string if there is no error.
    //! DLT Viewer calls this after any method returns false.
    QString error();

    //! Loads plugin settings from the given file path.
    //! Used for host/port, remote binding policy, filter mutation limit,
    //! and optional custom dictionary entries.
    bool loadConfig(QString filename);

    //! Saves current plugin settings to the given file path.
    //! Persists host/port, remote binding policy, filter mutation limit,
    //! and custom dictionary entries.
    bool saveConfig(QString filename);

    //! Returns a list of strings describing the current plugin configuration.
    //! Shown in the DLT Viewer plugin info dialog.
    QStringList infoConfig();

    /* QDltPluginViewerInterface */

    //! Creates the widget shown in DLT Viewer when selecting Plugin -> Show.
    QWidget* initViewer();

    //! Called by DLT Viewer when a file is opened or reloaded.
    //! Stores the QDltFile pointer so all MCP tools can read the
    //! currently active filtered view of the file.
    //! \param file  Handle to the currently open DLT file.
    void initFileStart(QDltFile* file);

    //! Called after all messages have been processed on file open. Not used.
    void initFileFinish();

    //! Called per-message during file indexing. Not used.
    void initMsg(int index, QDltMsg& msg);

    //! Called per-message (decoded) during file indexing. Not used.
    void initMsgDecoded(int index, QDltMsg& msg);

    //! Called before a file update batch. Not used.
    void updateFileStart();

    //! Called per-message during a live update. Not used.
    void updateMsg(int index, QDltMsg& msg);

    //! Called per-message (decoded) during a live update. Not used.
    void updateMsgDecoded(int index, QDltMsg& msg);

    //! Called after a file update batch. Not used.
    void updateFileFinish();

    //! Called when the user selects a message in the table. Not used.
    void selectedIdxMsg(int index, QDltMsg& msg);

    //! Called when the user selects a message (decoded). Not used.
    void selectedIdxMsgDecoded(int index, QDltMsg& msg);

    /* QDltPluginControlInterface */

    //! Called once by DLT Viewer after the plugin is loaded.
    //! Stores the QDltControl pointer and starts the MCP server in a background thread.
    //! \param control  Interface for sending commands back to DLT Viewer
    //!                 (e.g. open file, jump to message, connect ECU).
    bool initControl(QDltControl* control);

    //! Called by DLT Viewer with the list of currently configured ECU connection strings.
    //! Can be used to track which ECUs are available.
    //! \param list  List of ECU connection descriptors.
    bool initConnections(QStringList list);

    //! Called when a DLT control/service message response is received.
    //! This is not the callback for every incoming live log message.
    //! \param index  Index of the connection where the control message was received.
    //! \param msg    The received control response message.
    bool controlMsg(int index, QDltMsg& msg);

    //! Called when the connection state of an ECU changes (online/offline/error).
    //! \param index            Index of the ECU in the project tree.
    //! \param connectionState  New connection state.
    //! \param hostname         Hostname/IP of the ECU.
    bool stateChanged(int index, QDltConnection::QDltConnectionState connectionState,
                      QString hostname);

    //! Called when the user toggles the autoscroll feature in DLT Viewer.
    //! \param enabled  True if autoscroll was just enabled.
    bool autoscrollStateChanged(bool enabled);

    //! Provides a message decoder that can decode raw QDltMsg objects using
    //! all currently active decoder plugins. Store and use this when returning
    //! decoded payloads to the AI.
    //! \param messageDecoder  Pointer to the shared decoder instance.
    void initMessageDecoder(QDltMessageDecoder* messageDecoder);

    //! Provides a pointer to the main message table view.
    //! Used by navigate_to_message to scroll and select a row in the UI.
    //! \param pTableView  Pointer to the main QTableView in DLT Viewer.
    void initMainTableView(QTableView* pTableView);

    //! Called when the active filter configuration changes (e.g. a new filter
    //! is applied or sort order changes). Any in-flight async tool jobs are
    //! cancelled because their captured indices may no longer refer to the
    //! same messages once the filtered view is rebuilt.
    void configurationChanged();

signals:
    //! Emitted by onServerMessage() once a JSON-RPC response has been built.
    //! Connected (queued) to DltMcpServer::sendResponseToClient() so the
    //! actual socket write happens on the server thread, not the GUI thread.
    //! \param clientId  Opaque MCP client session identifier received with the request.
    //! \param response  The completed JSON-RPC 2.0 response object.
    void responseReady(const QString& clientId, const QJsonObject& response);

    //! Emitted whenever plugin runtime/config/file state changes.
    //! Connected once to requestViewerRefresh() to decouple state producers
    //! from UI rendering and coalesce multiple updates into one refresh.
    void viewerStateChanged();

private slots:
    //! Receives error notifications from the MCP server thread and stores
    //! the message in errorText so DLT Viewer can retrieve it via error().
    void onServerError(const QString& error);

    //! Receives incoming JSON-RPC requests forwarded by the MCP server thread.
    //! Dispatches to the appropriate handler (search_logs, get_error_messages,
    //! navigate_to_message, …), builds the JSON-RPC response, and routes it
    //! back to the originating client via DltMcpServer::sendResponseToClient().
    //! \param clientId  Opaque identifier of the MCP client session that sent the request.
    //! \param message   The parsed JSON-RPC 2.0 request object.
    void onServerMessage(const QString& clientId, const QJsonObject& message);
    void onResponseReadyTracked(const QString& clientId, const QJsonObject& response);
    void requestViewerRefresh();
    void performViewerRefresh();
    void processFindMessageByTimestampChunk();
    void processSearchLogsAroundIndexChunk();
    void processSearchLogsChunk();
    void processGetErrorMessagesChunk();

private:
    struct DictionaryEntry {
        QString key;
        QStringList aliases;
        QString text;
    };

    //! Fields shared by every chunked async tool job. Carrying them in a
    //! dedicated envelope lets schedule/fail/finish helpers in the driver be
    //! written once against the envelope instead of once per tool.
    //!
    //! \c generation is used together with the per-job schedule helpers to
    //! drop stale chunk continuations after a newer request has replaced the
    //! job: on every reschedule the current \c generation is captured and the
    //! chunk only runs if \c active is still true and \c generation still
    //! matches.
    struct AsyncJobEnvelope {
        bool active = false;
        QString clientId;
        QJsonValue requestId;
        quint64 generation = 0;
        bool useFilter = true;
        int total = 0;
        int nextIndex = 0;
        int chunkSize = 250;
    };

    struct SearchLogsJob {
        AsyncJobEnvelope env;
        QString pattern;
        QString ecuid;
        QString apid;
        QString ctid;
        QString type;
        QString subtype;
        QString mode;
        QString sessionId;
        int startIndex = 0;
        int scanLimit = 0;
        int scannedMessages = 0;
        int maxResults = 100;
        bool completed = false;
        QJsonArray results;
    };

    struct GetErrorMessagesJob {
        AsyncJobEnvelope env;
        int maxResults = 100;
        QJsonArray results;
    };

    struct FindMessageByTimestampJob {
        AsyncJobEnvelope env;
        QString timestampText;
        bool timeOnly = false;
        QTime requestedTimeOfDay;
        QDateTime requestedDateTime;
        int bestIndex = -1;
        bool hasBest = false;
        qint64 bestDistanceMs = 0;
        QDateTime matchedDateTime;
        QDltMsg matchedMsg;
    };

    struct SearchLogsAroundIndexJob {
        enum Phase { ExpandingStart, ExpandingEnd, CollectingMatches };

        AsyncJobEnvelope env;
        QString pattern;
        QString ecuid;
        QString apid;
        QString ctid;
        QString type;
        QString subtype;
        QString mode;
        QString sessionId;
        int maxResults = 100;
        int bestIndex = -1;
        bool hasBest = false;
        QDateTime matchedDateTime;
        QDltMsg matchedMsg;
        qint64 windowBeforeMs = 5000;
        qint64 windowAfterMs = 5000;
        qint64 anchorMonotonic = 0;
        qint64 lowerBound = 0;
        qint64 upperBound = 0;
        bool likelySorted = false;
        int startIndex = -1;
        int endIndex = -1;
        int scannedMessages = 0;
        QJsonArray results;
        Phase phase = ExpandingStart;
    };

    //! Signature for a single MCP tool handler.
    //! \param clientId   Originating MCP client session id.
    //! \param message    The full JSON-RPC 2.0 request object (used by async
    //!                   tools to capture the request id for later response).
    //! \param arguments  The "arguments" sub-object of the tools/call params.
    //! \param response   Response object pre-populated with "jsonrpc" and "id".
    //!                   Sync handlers must set either "result" or "error".
    //!                   Async handlers manage their own emission via
    //!                   responseReady() and may leave \p response untouched.
    typedef void (DltMcpPlugin::*ToolHandler)(const QString& clientId, const QJsonObject& message,
                                              const QJsonObject& arguments, QJsonObject& response);

    //! Describes how onServerMessage should invoke a single MCP tool.
    struct ToolEntry {
        ToolHandler handler;
        //! When true, onServerMessage rejects the call with an error if no
        //! DLT file is currently open.
        bool requiresFile;
        //! When true, the handler is responsible for emitting responseReady()
        //! itself (either synchronously on early errors or later from an
        //! async job). onServerMessage will not emit a response for the call.
        bool async;
    };

    //! Returns the shared tool dispatch table, built lazily on first use.
    const QHash<QString, ToolEntry>& toolHandlerTable() const;

    //! Guards handlers that need access to QDltControl. If the interface
    //! pointer is null, populates \p response with a JSON-RPC InternalError
    //! describing the missing interface and returns false so the caller can
    //! `return` immediately.
    //! \return true if dltControl is usable; false otherwise.
    bool ensureDltControl(QJsonObject& response) const;

    //! Guards handlers that accept a caller-supplied index. If \p index is
    //! outside [0, \p size), populates \p response with a JSON-RPC
    //! InvalidParams error naming \p label (e.g. "Connection", "Filter",
    //! "Index") and returns false so the caller can `return` immediately.
    //! \return true if the index is within range; false otherwise.
    bool ensureIndexInRange(int index, int size, const QString& label, QJsonObject& response) const;

    // --- Sync tool handlers ---
    void handleGetContext(const QString&, const QJsonObject&, const QJsonObject&, QJsonObject&);
    void handleGetDictionary(const QString&, const QJsonObject&, const QJsonObject&, QJsonObject&);
    void handleListConnections(const QString&, const QJsonObject&, const QJsonObject&,
                               QJsonObject&);
    void handleGetConnectionStates(const QString&, const QJsonObject&, const QJsonObject&,
                                   QJsonObject&);
    void handleGetFileInfo(const QString&, const QJsonObject&, const QJsonObject&, QJsonObject&);
    void handleGetMessagesRange(const QString&, const QJsonObject&, const QJsonObject&,
                                QJsonObject&);
    void handleListFilters(const QString&, const QJsonObject&, const QJsonObject&, QJsonObject&);
    void handleAddFilter(const QString&, const QJsonObject&, const QJsonObject&, QJsonObject&);
    void handleRemoveFilter(const QString&, const QJsonObject&, const QJsonObject&, QJsonObject&);
    void handleSetFilterEnabled(const QString&, const QJsonObject&, const QJsonObject&,
                                QJsonObject&);
    void handleConnectEcu(const QString&, const QJsonObject&, const QJsonObject&, QJsonObject&);
    void handleDisconnectEcu(const QString&, const QJsonObject&, const QJsonObject&, QJsonObject&);
    void handleConnectAllEcus(const QString&, const QJsonObject&, const QJsonObject&, QJsonObject&);
    void handleDisconnectAllEcus(const QString&, const QJsonObject&, const QJsonObject&,
                                 QJsonObject&);
    void handleNavigateToMessage(const QString&, const QJsonObject&, const QJsonObject&,
                                 QJsonObject&);
    void handleClearLogs(const QString&, const QJsonObject&, const QJsonObject&, QJsonObject&);

    // --- Async tool handlers (they emit responseReady() themselves) ---
    void handleSearchLogs(const QString&, const QJsonObject&, const QJsonObject&, QJsonObject&);
    void handleFindMessageByTimestamp(const QString&, const QJsonObject&, const QJsonObject&,
                                      QJsonObject&);
    void handleSearchLogsAroundIndex(const QString&, const QJsonObject&, const QJsonObject&,
                                     QJsonObject&);
    void handleGetErrorMessages(const QString&, const QJsonObject&, const QJsonObject&,
                                QJsonObject&);

    QJsonObject makeTextResult(const QString& text, const QJsonObject& data = QJsonObject()) const;
    QJsonObject buildContextData() const;
    QString buildContextText() const;
    QString buildDictionaryText(const QString& term) const;
    QString messageTypeToString(QDltMsg::DltTypeDef type) const;
    QList<DictionaryEntry> builtInDictionaryEntries() const;
    bool readMessageAt(int index, bool useFilter, QDltMsg& msg) const;
    bool isMonotonicOrderLikelySorted(bool useFilter, int total) const;
    int findAnchorIndex(const QString& timestampText, bool useFilter, int total,
                        qint64& bestDistanceMs, QDateTime& matchedDateTime, QDltMsg& matchedMsg,
                        bool& timeOnly, QString& errorMessage) const;
    void finishFindMessageByTimestampJob();
    void finishSearchLogsAroundIndexJob();
    void finishSearchLogsJob();
    void finishGetErrorMessagesJob();

    //! Cancel every currently-active chunked async job with the given
    //! reason. Each cancellation emits a JSON-RPC error response to the
    //! originating client via \c failJob(). Jobs that are not active are
    //! left untouched (no response is sent for them).
    //!
    //! Used on DLT Viewer lifecycle events that invalidate all in-flight
    //! per-tool state at once: the filter/sort configuration changed, or
    //! the active DLT file was swapped. Each tool substitutes its own name
    //! prefix into \p reasonTemplate via \c QString::arg() so the client
    //! sees a message tailored to the cancelled tool.
    //!
    //! \param reasonTemplate A message template containing exactly one
    //!                       \c %1 placeholder; will be formatted with the
    //!                       tool name (e.g. "search_logs").
    void cancelAllActiveJobs(const QString& reasonTemplate);

    //! Latest-request-wins scheduler for chunked async tool jobs.
    //!
    //! Captures \c job.env.generation at schedule time; when the timer fires,
    //! the continuation is dropped if the job has been replaced (generation
    //! mismatch) or cancelled (\c env.active == false). This lets a newer
    //! request reuse the same job slot without stale chunks from the previous
    //! request clobbering it.
    //!
    //! \param job     Reference to the job member of this plugin. Must remain
    //!                valid for the lifetime of this QObject (it is a data
    //!                member, so this is always the case).
    //! \param process Pointer-to-member-function for the per-tool chunk body
    //!                (e.g. \c &DltMcpPlugin::processSearchLogsChunk).
    template <typename JobT> void scheduleChunk(const JobT& job, void (DltMcpPlugin::*process)()) {
        const quint64 gen = job.env.generation;
        const JobT* jobPtr = &job;
        QTimer::singleShot(0, this, [this, jobPtr, gen, process]() {
            if (!jobPtr->env.active || jobPtr->env.generation != gen)
                return;
            (this->*process)();
        });
    }

    //! Finish an active chunked async job successfully: emit a JSON-RPC
    //! success response wrapping \p resultText and reset the job slot to
    //! its default-constructed state. No-op if the job is not active.
    //!
    //! Each tool builds its own per-tool summary text (including an embedded
    //! JSON payload where applicable) and passes it in; this helper only
    //! handles the shared response-envelope construction and teardown that
    //! every finish-path used to duplicate verbatim.
    //!
    //! \param job        The job to finish; reset to \c JobT{} on exit.
    //! \param resultText The tool-specific human-readable summary text,
    //!                   passed straight to \c makeTextResult().
    template <typename JobT> void finishJobWithText(JobT& job, const QString& resultText) {
        if (!job.env.active)
            return;
        QJsonObject response;
        response["jsonrpc"] = "2.0";
        response["id"] = job.env.requestId;
        response["result"] = makeTextResult(resultText);
        const QString clientId = job.env.clientId;
        job = JobT{};
        emit viewerStateChanged();
        emit responseReady(clientId, response);
    }

    //! Cancel an active chunked async job by emitting a JSON-RPC error
    //! response and resetting the job slot to its default-constructed state.
    //! No-op if the job is not currently active.
    //!
    //! Used both for explicit cancellation (newer request replaces the job,
    //! DLT Viewer filter/sort configuration changed, active DLT file changed)
    //! and for recoverable internal errors that abort the job (missing
    //! \c dltFile, regex compile failure, etc.).
    //!
    //! \param job          The job to cancel; reset to \c JobT{} on exit.
    //! \param errorMessage Human-readable message sent to the MCP client as
    //!                     the JSON-RPC \c error.message.
    //! \param errorCode    JSON-RPC error code; defaults to
    //!                     \c JsonRpcError::InternalError to match the
    //!                     pre-refactor behaviour of every call site.
    template <typename JobT>
    void failJob(JobT& job, const QString& errorMessage,
                 int errorCode = JsonRpcError::InternalError) {
        if (!job.env.active)
            return;
        QJsonObject response;
        response["jsonrpc"] = "2.0";
        response["id"] = job.env.requestId;
        response["error"] = makeJsonRpcError(errorCode, errorMessage);
        const QString clientId = job.env.clientId;
        job = JobT{};
        emit viewerStateChanged();
        emit responseReady(clientId, response);
    }

    //! Centralised write-path for the three viewer-panel fields
    //! \c lastToolCallName, \c lastToolCallStatus, \c lastToolCallTime.
    //!
    //! Both the entry path (in \c onServerMessage when a request arrives) and
    //! the completion path (in \c onResponseReadyTracked when the response is
    //! emitted) call this helper so the field-update order and the
    //! \c viewerStateChanged() emission are defined in exactly one place.
    //! \param toolName MCP tool name (e.g. "search_logs").
    //! \param status   Free-form status string displayed in the viewer panel
    //!                 (e.g. "processing", "running", "ok", "error").
    void setLastToolCallStatus(const QString& toolName, const QString& status);

    void startServer();
    void stopServer();
    void tryStartServer();

    //! Loads the bearer token from e.g. ~/.dlt/mcp/token.txt or generates one on
    //! first use and persists it. Delete the file to rotate.
    void ensureAuthToken();

    //! Absolute path of the bearer token file (e.g. ~/.dlt/mcp/token.txt).
    QString authTokenPath() const;

    QString requestIdKey(const QJsonValue& id) const;
    QJsonArray connectedClientSnapshot() const;
    QString buildViewerPanelText();
    void refreshViewerPanel();

    //! Pointer to the DLT Viewer control interface.
    //! Used to send commands such as jumpToMsg, openFile, connectEcu back to the viewer.
    QDltControl* dltControl;

    //! Pointer to the shared message decoder provided by DLT Viewer.
    //! Allows decoding raw QDltMsg payloads through all active decoder plugins
    //! before returning them to the AI.
    QDltMessageDecoder* messageDecoder;

    //! Pointer to the main message table view in DLT Viewer.
    //! Used by the navigate_to_message MCP tool to scroll and highlight a row.
    QTableView* mainTableView;

    bool shuttingDown;

    //! The MCP server instance. Lives in serverThread.
    DltMcpServer* mcpServer;

    //! Stores the most recent error description for retrieval via error().
    QString errorText;

    //! Background thread that owns and runs the MCP server so that
    //! network I/O does not block the DLT Viewer main (GUI) thread.
    QThread* serverThread;

    //! Pointer to the currently open DLT file, provided by DLT Viewer via
    //! initFileStart(). MCP tools read from this file using either the
    //! filtered view (use_filter=true, the default — same scope the user
    //! sees in DLT Viewer) or the full unfiltered file (use_filter=false).
    //! Null until a file is opened.
    QDltFile* dltFile;

    //! Root widget shown in the plugin "Show" panel.
    QWidget* viewerWidget;

    //! Read-only text area in the plugin "Show" panel.
    QPlainTextEdit* viewerText;

    //! True while a queued panel refresh is pending.
    bool viewerRefreshPending;

    //! TCP port the MCP server listens on.
    //! Default: 3719. Persisted via loadConfig()/saveConfig().
    quint16 serverPort;

    //! When false (default), the server binds to 127.0.0.1 (loopback only).
    //! When true, the server binds to 0.0.0.0 (all interfaces), making it
    //! reachable from other machines — e.g. when DLT Viewer runs in a VM
    //! and the MCP client runs on the host. Use with caution.
    //! Persisted via loadConfig()/saveConfig().
    bool allowRemoteConnections;

    //! Maximum trace size for MCP-driven filter add/remove/enable changes.
    //! Default: 100000. Set to a negative value to disable the cap.
    //! Persisted via loadConfig()/saveConfig().
    int filterMutationMessageLimit;

    //! Bearer token required on every MCP request; loaded from e.g. ~/.dlt/mcp/token.txt.
    QByteArray authToken;

    QStringList configuredConnections;
    QMap<int, QDltConnection::QDltConnectionState> connectionStates;
    QMap<int, QString> connectionHostnames;
    QMap<QString, QString> pendingToolNamesByRequestId;
    //! Request-id key of the most recently started tool call. Used by
    //! \c onResponseReadyTracked to ignore late-arriving responses from
    //! older requests so they don't clobber the newest call's display.
    QString latestToolCallRequestKey;
    QString lastToolCallName;
    QString lastToolCallStatus;
    QDateTime lastToolCallTime;

    //! Custom glossary/dictionary entries loaded from the plugin XML config.
    //! Persisted via loadConfig()/saveConfig().
    QList<DictionaryEntry> customDictionaryEntries;

    FindMessageByTimestampJob findMessageByTimestampJob;
    SearchLogsAroundIndexJob searchLogsAroundIndexJob;
    SearchLogsJob searchLogsJob;
    GetErrorMessagesJob getErrorMessagesJob;

    //! Strictly increasing counter sourcing the \c env.generation value of
    //! every new chunked async job. Combined with the
    //! \c scheduleChunk()/failJob() cancellation mechanism, the counter
    //! guarantees that any in-flight chunk continuation whose captured
    //! generation no longer matches its job's current generation is dropped,
    //! even if the job slot has been reassigned multiple times in between.
    //!
    //! 64-bit: at any realistic job-start rate (interactive MCP usage is at
    //! most a few per minute) this will not overflow within the lifetime of
    //! any process. Resets to 1 on plugin construction.
    quint64 nextJobGeneration = 1;

    bool controlInitialized;
    bool configLoaded;
};

#endif // DLTMCPPLUGIN_H

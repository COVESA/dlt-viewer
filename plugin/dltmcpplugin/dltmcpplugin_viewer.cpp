// Implements the plugin status panel showing server state, connected clients, and tool activity.

#include "dltmcpplugin.h"
#include "dltmcpserver.h"

#include <QMetaObject>
#include <QPlainTextEdit>
#include <QThread>
#include <QVBoxLayout>

/* QDltPluginViewerInterface implementation */

QWidget* DltMcpPlugin::initViewer() {
    if (viewerWidget)
        return viewerWidget;

    viewerWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(viewerWidget);
    layout->setContentsMargins(8, 8, 8, 8);

    viewerText = new QPlainTextEdit(viewerWidget);
    viewerText->setReadOnly(true);
    viewerText->setLineWrapMode(QPlainTextEdit::NoWrap);
    layout->addWidget(viewerText);

    emit viewerStateChanged();
    return viewerWidget;
}

void DltMcpPlugin::requestViewerRefresh() {
    if (viewerRefreshPending)
        return;

    viewerRefreshPending = true;
    QMetaObject::invokeMethod(this, "performViewerRefresh", Qt::QueuedConnection);
}

void DltMcpPlugin::performViewerRefresh() {
    viewerRefreshPending = false;
    refreshViewerPanel();
}

QJsonArray DltMcpPlugin::connectedClientSnapshot() const {
    // Called on the GUI thread: copy client state on its owning server thread
    // instead of reading socket/session containers across threads directly.
    if (!mcpServer || !serverThread || !serverThread->isRunning())
        return QJsonArray();

    QJsonArray snapshot;
    const bool ok =
        QMetaObject::invokeMethod(mcpServer, "getConnectedClients", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(QJsonArray, snapshot));
    if (!ok)
        return QJsonArray();

    return snapshot;
}

QString DltMcpPlugin::buildViewerPanelText() {
    QStringList lines;
    const QJsonArray clients = connectedClientSnapshot();
    QStringList activeJobs;
    if (findMessageByTimestampJob.env.active)
        activeJobs << "find_message_by_timestamp";
    if (searchLogsAroundIndexJob.env.active)
        activeJobs << "search_logs_around_index";
    if (searchLogsJob.env.active)
        activeJobs << "search_logs";
    if (getErrorMessagesJob.env.active)
        activeJobs << "get_error_messages";
    if (activeJobs.isEmpty() && lastToolCallStatus == QLatin1String("running") &&
        !lastToolCallName.isEmpty())
        activeJobs << lastToolCallName;

    if (!authToken.isEmpty()) {
        lines << QString("MCP token: %1").arg(QString::fromUtf8(authToken));
        lines << QString("Token file: %1 (delete to rotate)").arg(authTokenPath());
        lines << "";
    }

    lines << QString("Status: %1").arg(mcpServer ? "running" : "stopped");
    lines << QString("Connected MCP clients: %1").arg(clients.size());
    lines << (activeJobs.isEmpty() ? QStringLiteral("Active job: none")
                                   : QString("Active job: %1").arg(activeJobs.join(", ")));
    if (!lastToolCallName.isEmpty()) {
        lines << QString("Last tool call: %1").arg(lastToolCallName);
        lines << QString("Last tool status: %1")
                     .arg(lastToolCallStatus.isEmpty() ? QStringLiteral("unknown")
                                                       : lastToolCallStatus);
        lines << QString("Last tool time: %1")
                     .arg(lastToolCallTime.isValid()
                              ? lastToolCallTime.toString("yyyy/MM/dd HH:mm:ss.zzz")
                              : QStringLiteral("unknown"));
    } else {
        lines << QStringLiteral("Last tool call: none");
    }

    if (!clients.isEmpty()) {
        lines << "";
        lines << "MCP Clients:";
        for (int i = 0; i < clients.size(); ++i) {
            const QJsonObject client = clients.at(i).toObject();
            const QString sessionId = client.value("sessionId").toString();
            const QString peerAddress = client.value("peerAddress").toString();
            const int peerPort = client.value("peerPort").toInt();
            const QString clientName = client.value("clientName").toString().trimmed();
            const QString clientVersion = client.value("clientVersion").toString().trimmed();
            const QString userAgent = client.value("userAgent").toString().trimmed();

            QString identity;
            if (!clientName.isEmpty()) {
                identity = clientName;
                if (!clientVersion.isEmpty())
                    identity += QString(" %1").arg(clientVersion);
            } else if (!userAgent.isEmpty()) {
                identity = userAgent;
            } else {
                identity = "unidentified client";
            }

            lines << QString("  [%1] %2").arg(i).arg(identity);
            lines << QString("      session=%1")
                         .arg(sessionId.isEmpty() ? QStringLiteral("<none>") : sessionId);
            lines << QString("      peer=%1:%2")
                         .arg(peerAddress.isEmpty() ? QStringLiteral("<unknown>") : peerAddress)
                         .arg(peerPort);
        }
    }

    if (!errorText.trimmed().isEmpty()) {
        lines << "";
        lines << "Last Error";
        lines << QString("  %1").arg(errorText.trimmed());
    }

    return lines.join("\n");
}

void DltMcpPlugin::refreshViewerPanel() {
    if (!viewerText)
        return;

    viewerText->setPlainText(buildViewerPanelText());
}

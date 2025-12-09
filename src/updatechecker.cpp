 #include <qtimer.h>
#include <updatechecker.h>
#include <version.h>

#include <QDebug>
#include <QUrl>
#include <QDesktopServices>
#include <QMessageBox>
#include <QPushButton>
#include <QNetworkAccessManager> // To send HTTP GET requests
#include <QNetworkRequest>       // To create a request
#include <QNetworkReply>         // To handle the network response
#include <QRegularExpression>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent),
      updateTimer(nullptr),
      manager(new QNetworkAccessManager(this))
{
}

void UpdateChecker::linkToUrl(){
    qDebug() << "Update check called";

    QString currentVersion = PACKAGE_VERSION;
    QNetworkRequest request(QUrl("https://api.github.com/repos/COVESA/dlt-viewer/releases/latest"));
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Failed to fetch version info:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QString response = reply->readAll();
        reply->deleteLater();

        QRegularExpression re(R"((\d+\.\d+(\.\d+)?))");
        QRegularExpressionMatch match = re.match(response);
        QString latestVersion = match.hasMatch() ? match.captured(1) : "";

        qDebug() << "Current Version:" << currentVersion;
        qDebug() << "Latest Version:" << latestVersion;

               // Compare versions
        if (isNewerVersion(latestVersion, currentVersion)) {
            showUpdatePopup(currentVersion, latestVersion);
        } else {
            showNoUpdatePopup();
        }

        reply->deleteLater();
    });

}

QDateTime UpdateChecker::getLastCheckTime()
{
    QSettings s("MyCompany", "DLTViewer");
    return s.value("updateCheck/lastCheck").toDateTime();
}

void UpdateChecker::updateLastCheckTime()
{

    QDateTime now = QDateTime::currentDateTime();
    qDebug() << "[UpdateTime] Saving last check time:" << now.toString();

    QSettings s("MyCompany", "DLTViewer");
    s.setValue("updateCheck/lastCheck", QDateTime::currentDateTime());
}

// interval: default = 3 months
bool UpdateChecker::isIntervalPassed()
{
    QSettings s("MyCompany", "DLTViewer");

    bool useCustom = s.value("updateCheck/useCustom", false).toBool();
    int customMonths = s.value("updateCheck/customMonths", DEFAULT_UPDATE_CHECK_MONTHS).toInt();
    int months = useCustom ? customMonths : DEFAULT_UPDATE_CHECK_MONTHS;

    QDateTime last = getLastCheckTime();

    qDebug() << "[IntervalCheck] Custom?" << useCustom
             << "Months =" << months;

    if (!last.isValid()){

        qDebug() << "[IntervalCheck] No last check found → FIRST RUN → PASS";
        return true;
    }

    bool passed = last.addMonths(months) <= QDateTime::currentDateTime();
    qDebug() << "[IntervalCheck] Interval Passed =" << passed;

    return passed;
}

// Start auto-check timer
void UpdateChecker::startAutoCheck()
{
    QSettings s("MyCompany", "DLTViewer");

    bool useCustom = s.value("updateCheck/useCustom", false).toBool();
    int minutes = s.value("updateCheck/customMinutes", 2).toInt();

    if (!useCustom)
        minutes = 120;

    qDebug() << "[AutoCheck] Starting auto update timer every" << minutes << "minutes";

    if (!updateTimer) {
        updateTimer = new QTimer(this);
        connect(updateTimer, &QTimer::timeout, this, &UpdateChecker::checkForUpdates);
    }

    updateTimer->start(minutes * 60 * 1000);
}

void UpdateChecker::checkForUpdates()
{
    if (!isIntervalPassed()) {
        qDebug() << "Interval not passed. Skipping update check.";
        return;
    }

    QString currentVersion = PACKAGE_VERSION;
    QNetworkRequest request(QUrl("https://api.github.com/repos/COVESA/dlt-viewer/releases/latest"));

    // Add GitHub-required header
    request.setRawHeader("User-Agent", "QtApp");

    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {

        qDebug() << "[Network] Reply received";

        updateLastCheckTime();

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Failed to fetch version info:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QString jsonResponse = reply->readAll();
        qDebug() << "[Network] JSON Received =" << jsonResponse.left(200) << "...";
        reply->deleteLater();

               // Parse JSON
        QJsonDocument doc = QJsonDocument::fromJson(jsonResponse.toUtf8());
        QJsonObject obj = doc.object();

        QString tag = obj["tag_name"].toString();   // Example: "v3.35.0"
        QString latestVersion = tag.startsWith('v') ? tag.mid(1) : tag;

        QString releaseUrl = obj["html_url"].toString();

        qDebug() << "Current Version:" << currentVersion;
        qDebug() << "Latest Version:" << latestVersion;
        qDebug() << "Release URL:" << releaseUrl;

        if (isNewerVersion(latestVersion, currentVersion)) {
            showUpdatePopup(currentVersion, latestVersion);   // You can open releaseUrl here
        } else {
            qDebug() << "No Latest Version available for DLT Viewer";
            // showNoUpdatePopup();
        }
    });
}

// Version comparison
bool UpdateChecker::isNewerVersion(const QString &latest, const QString &current)
{
    QStringList l = latest.split(".");
    QStringList c = current.split(".");

    for (int i = 0; i < qMax(l.size(), c.size()); ++i) {
        int latestVersion = (i < l.size()) ? l[i].toInt() : 0;
        int currentVersion = (i < c.size()) ? c[i].toInt() : 0;

        if (latestVersion > currentVersion) return true;
        if (latestVersion < currentVersion) return false;
    }
    return false;
}

// Popups
void UpdateChecker::showUpdatePopup(const QString &current, const QString &latest)
{
    QMessageBox msg;
    msg.setWindowTitle("Update Available");
    msg.setText(QString("Current Version: %1\nAvailable Version: %2")
                    .arg(current, latest));

    QPushButton *updateBtn = msg.addButton("Update Now", QMessageBox::AcceptRole);
    msg.addButton("Ignore", QMessageBox::RejectRole);

    msg.exec();

    if (msg.clickedButton() == updateBtn) {
        QDesktopServices::openUrl(QUrl("https://github.com/COVESA/dlt-viewer/releases/latest"));
    }
}

void UpdateChecker::showNoUpdatePopup()
{
    QMessageBox::information(nullptr,
                             "No Updates",
                             "No recent updates available.");
}

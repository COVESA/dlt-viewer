#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <settingsdialog.h>

class UpdateChecker : public QObject
{
    Q_OBJECT

  public:
    explicit UpdateChecker(QObject *parent = nullptr);

    void startAutoCheck();
    void checkForUpdates();
    void linkToUrl();

  private:
    void showUpdatePopup(const QString &current, const QString &latest);
    void showNoUpdatePopup();
    bool isNewerVersion(const QString &latest, const QString &current);

  private:
    QTimer *updateTimer;
    QNetworkAccessManager *manager;

           // used for 3-month interval logic
    void updateLastCheckTime();
    QDateTime getLastCheckTime();
    bool isIntervalPassed();

};

#endif // UPDATECHECKER_H

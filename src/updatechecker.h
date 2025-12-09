#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QNetworkAccessManager>

class updateChecker : public QObject
{
    Q_OBJECT

  public:
    explicit updateChecker(QObject *parent = nullptr);

    void startAutoCheck();          // starts the timer based on default/custom interval
    void checkForUpdates();         // actual update check logic
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

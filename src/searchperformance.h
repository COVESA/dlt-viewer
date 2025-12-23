#ifndef SEARCHPERFORMANCE_H
#define SEARCHPERFORMANCE_H

#include <QElapsedTimer>
#include <QString>

class SearchPerformance
{
public:
    SearchPerformance();

    //Start performance measurement
    void start(const QString& searchTerm = "");

    //Stop measurement and return results
    QString stop(qint64 messagesProcessed = 0);

    //Get current elapsed time without stopping
    qint64 elapsed() const;

    //Check if measurement is running
    bool isRunning() const;

private:

    //Get current CPU time in milliseconds
    qint64 getCpuTimeMs();

    QElapsedTimer m_timer;
    qint64 m_cpuTimeStart;
    bool m_isRunning;
    qint64 m_messagesProcessed;
    QString m_searchTerm;
};

#endif // SEARCHPERFORMANCE_H

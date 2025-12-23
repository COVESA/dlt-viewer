#include "searchperformance.h"
#include <QDebug>

#if defined(_MSC_VER)
#include <windows.h>
#else
#include <sys/time.h>
#include <sys/resource.h>
#endif

SearchPerformance::SearchPerformance()
    : m_isRunning(false)
    , m_messagesProcessed(0)
    , m_cpuTimeStart(0)
{
}

void SearchPerformance::start(const QString& searchTerm)
{
    m_searchTerm = searchTerm;
    m_timer.start();
    m_cpuTimeStart = getCpuTimeMs();
    m_isRunning = true;
    m_messagesProcessed = 0;
}

QString SearchPerformance::stop(qint64 messagesProcessed)
{
    if (!m_isRunning) {
        return "Performance measurement was not started";
    }

    m_messagesProcessed = messagesProcessed;
    qint64 elapsedMs = m_timer.elapsed();
    qint64 cpuTimeMs = getCpuTimeMs() - m_cpuTimeStart;

    m_isRunning = false;

    // Convert to nanoseconds for Google Benchmark compatibility
    qint64 elapsedNs = elapsedMs * 1000000LL;
    qint64 cpuTimeNs = cpuTimeMs * 1000000LL;

    // Calculate metrics
    double cpuPercent = (elapsedMs > 0) ? (cpuTimeMs * 100.0 / elapsedMs) : 0.0;
    double throughput = (elapsedMs > 0) ? (messagesProcessed * 1000.0 / elapsedMs) : 0.0;

    // Format throughput with units (like Google Benchmark)
    QString throughputStr;
    if (throughput >= 1000000.0) {
        throughputStr = QString::number(throughput / 1000000.0, 'f', 3) + "M items/s";
    } else if (throughput >= 1000.0) {
        throughputStr = QString::number(throughput / 1000.0, 'f', 3) + "k items/s";
    } else {
        throughputStr = QString::number(throughput, 'f', 3) + " items/s";
    }

    // Format output like Google Benchmark table
    QString result;
    result += "----------------------------------------------------------------------------------------------\n";
    result += QString("Benchmark                      Time(ns)      CPU(ns)    Messages     CPU%   throughput\n");
    result += "------------------------------------------------------------------------------------------------\n";

    // Use search term if provided
    QString benchmarkName = m_searchTerm.isEmpty() ? "DLT_Search" : m_searchTerm;
    // Search term formatting (remove special chars, limit length)
    benchmarkName = benchmarkName.left(20).replace(" ", "_");

    // Format benchmark name with message count
    QString fullBenchmarkName = QString("%1").arg(benchmarkName);

    // Build the formatted line using QString methods
    result += QString("%1 %2 %3 %4        %5  %6\n")
        .arg(fullBenchmarkName, -25)      // Left-align, 25 chars wide
        .arg(elapsedNs, 12)               // Right-align, 12 chars wide
        .arg(cpuTimeNs, 12)               // Right-align, 12 chars wide
        .arg(messagesProcessed, 8)        // Right-align, 8 chars wide
        .arg(cpuPercent, 6, 'f', 2)       // 6 chars wide, 2 decimal places
        .arg(throughputStr);              // Variable length
    result += "---------------------------------------------------------------------------------------------";

    return result;
}

qint64 SearchPerformance::elapsed() const
{
    return m_timer.elapsed();
}

bool SearchPerformance::isRunning() const
{
    return m_isRunning;
}

qint64 SearchPerformance::getCpuTimeMs()
{
#if defined(_MSC_VER)
    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime)) {
        ULARGE_INTEGER kernel, user;
        kernel.LowPart = kernelTime.dwLowDateTime;
        kernel.HighPart = kernelTime.dwHighDateTime;
        user.LowPart = userTime.dwLowDateTime;
        user.HighPart = userTime.dwHighDateTime;

        // Convert 100-nanosecond intervals to milliseconds
        return static_cast<qint64>((kernel.QuadPart + user.QuadPart) / 10000);
    }
    return 0;
#else
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1000LL +
               (usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000LL;
    }
    return 0;
#endif
}

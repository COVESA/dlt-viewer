#include "dltfileexporter.h"
#include "qdltmsg.h"

#include <ctime>
#include <cstddef>
#include <qdltfilterlist.h>
#include <qdltfile.h>

#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <stdexcept>

namespace {

std::optional<std::pair<QDltMsg, QByteArray>> getMessage(const QDltFile& file, int index) {
    QByteArray buf = file.getMsg(index);
    if(buf.isEmpty())
    {
        qDebug() << "Message buffer empty in for msg with index" << index;
        return std::nullopt;
    }
    QDltMsg msg;
    msg.setMsg(buf);
    msg.setIndex(index);

    return std::make_pair(std::move(msg), buf);
}

class SimpleWriter {
public:
    SimpleWriter(const QString& outputPath) {
        m_output.setFileName(outputPath);
        if (!m_output.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qDebug() << "Couldn't open output file: " << m_output.fileName();
            throw std::runtime_error("File couldn't be opened for writing");
        }
    }

    void write(const QByteArray& buf, const time_t&) {
        m_output.write(buf);
    }

private:
    QFile m_output;
};

class SplitWriter {
public:
    SplitWriter(const QString& basePath, std::size_t maxOutputSize)
      : m_basePath(basePath), m_bytesWritten{maxOutputSize}, m_maxOutputSize{maxOutputSize} {}

    ~SplitWriter() {
        if (m_output.isOpen()) {
            // rename very last file
            m_output.rename(nextFileName());
        }
    }

    void write(const QByteArray& buf, const time_t& ts) {
        if (m_bytesWritten >= m_maxOutputSize) {
            if (m_output.isOpen()) {
                m_output.rename(nextFileName());
                m_output.close();
            }

            m_output.setFileName(m_basePath + "_tmp.dlt");
            if (!m_output.open(QIODevice::WriteOnly)) {
                qDebug() << "Couldn't open output file: " << m_output.fileName();
                throw std::runtime_error("File couldn't be opened for writing");
            }
            m_bytesWritten = 0;
            ++m_fileCounter;
            m_timestampBegin = formatTimestamp(ts);
        }
        m_output.write(buf);
        m_bytesWritten += buf.size();
        m_timestampEnd = formatTimestamp(ts);
    }
private:
    QFile m_output;
    QString m_basePath;
    std::size_t m_bytesWritten;
    std::size_t m_fileCounter{0};
    std::size_t m_maxOutputSize;

    QString m_timestampBegin;
    QString m_timestampEnd;

    QString nextFileName() {
        return m_basePath + "_" + m_timestampBegin + "-" + m_timestampEnd + "_" +
               QString::number(m_fileCounter) + ".dlt";
    }

    QString formatTimestamp(const time_t& timestamp) {
        char strtime[256];
        struct tm *timeTm;
        timeTm = localtime(&timestamp);
        if(timeTm)
            strftime(strtime, 256, "%Y-%m-%d_%H-%M-%S", timeTm);
        return QString(strtime);
    }
};

template <typename Writer>
void processMessages(const QDltFile& m_input, QDltFilterList& filterList, Writer& writer) {
    for (int i = 0; i < m_input.size(); ++i) {
        auto res = getMessage(m_input, i);
        if (!res) {
            continue;
        }
        auto [msg, buf] = *res;
        if (filterList.isEmpty() || filterList.checkFilter(msg)) {
            bool isApplied = m_input.applyRegExStringMsg(msg);
            if(isApplied) msg.getMsg(buf,true);
            writer.write(buf, msg.getTime());
        }
    }
}
}

DltFileExporter::DltFileExporter(const QDltFile& input) : m_input(input) {}

void DltFileExporter::setFilterList(const QStringList& filterList, bool splitByFilter)
{
    m_filters = filterList;
    m_splitByFilter = splitByFilter;
}

void DltFileExporter::setMaxOutputSize(std::size_t sz)
{
    m_maxOutputSize = sz;
}

void DltFileExporter::exportMessages(const QString& outputName)
{
    if (m_splitByFilter) {
        const QFileInfo outputInfo(outputName);
        const auto outputDir = outputInfo.absolutePath() + "/" + outputInfo.baseName();
        if (!QDir(outputDir).exists() && !QDir().mkpath(outputDir)) {
            qDebug() << "Couldn't create output directory: " << outputDir;
            return;
        }
        for (const auto& filterFilepath : m_filters) {
            QDltFilterList filterList;
            if(!filterList.LoadFilter(filterFilepath, true)) {
                qDebug() << "Export: Open filter file " << filterFilepath << " failed!";
                continue;
            }

            const QFileInfo filterInfo(filterFilepath);
            if (m_maxOutputSize) {
                SplitWriter writer(outputDir + "/" + filterInfo.baseName(), *m_maxOutputSize);
                processMessages(m_input, filterList, writer);
            } else {
                SimpleWriter writer(outputDir + "/" + filterInfo.baseName() + ".dlt");
                processMessages(m_input, filterList, writer);
            }
        }
    } else {
        QDltFilterList filterList;
        for (const auto& filterFilepath : m_filters) {
            if(!filterList.LoadFilter(filterFilepath, false)) {
                qDebug() << "Export: Open filter file " << filterFilepath << " failed!";
            }
        }

        const QFileInfo info(outputName);
        if (m_maxOutputSize) {
            SplitWriter writer(info.absolutePath() + "/" + info.baseName(), *m_maxOutputSize);
            processMessages(m_input, filterList, writer);
        } else {
            SimpleWriter writer(outputName);
            processMessages(m_input, filterList, writer);
        }
    }
}

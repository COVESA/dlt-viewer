#include "dltfileexporter.h"
#include "qdltmsg.h"

#include <cstddef>
#include <qdltfilterlist.h>
#include <qdltfile.h>

#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <stdexcept>
#include <variant>

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
    bool isApplied = file.applyRegExStringMsg(msg);
    if(isApplied) msg.getMsg(buf,true);

    return std::make_pair(std::move(msg), buf);
}

class SimpleWriter {
public:
    SimpleWriter(const QString& outputPath) {
        m_output.setFileName(outputPath);
    }

    void write(const QByteArray& buf) {
        m_output.write(buf);
    }

private:
    QFile m_output;
};

class SplitWriter {
public:
    SplitWriter(const QString& basePath, std::size_t maxOutputSize)
      : m_basePath(basePath), m_bytesWritten{maxOutputSize}, m_maxOutputSize{maxOutputSize} {}

    void write(const QByteArray& buf) {
        if (m_bytesWritten >= m_maxOutputSize) {
            m_output.close();
            m_output.setFileName(m_basePath + QString::number(m_fileCounter) + ".dlt");
            if (!m_output.open(QIODevice::WriteOnly)) {
                qDebug() << "Couldn't open output file: " << m_output.fileName();
                throw std::runtime_error("File couldn't be opened for writing");
            }
            m_bytesWritten = 0;
            ++m_fileCounter;
        }
        m_output.write(buf);
        m_bytesWritten += buf.size();
    }
private:
    QFile m_output;
    QString m_basePath;
    std::size_t m_bytesWritten;
    std::size_t m_fileCounter{1};
    std::size_t m_maxOutputSize;
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
            writer.write(buf);
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
                SplitWriter writer(outputDir + "/" + filterInfo.baseName() + "_", *m_maxOutputSize);
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
            SplitWriter writer(info.absolutePath() + "/" + info.baseName() + "_", *m_maxOutputSize);
            processMessages(m_input, filterList, writer);
        } else {
            SimpleWriter writer(outputName);
            processMessages(m_input, filterList, writer);
        }
    }
}

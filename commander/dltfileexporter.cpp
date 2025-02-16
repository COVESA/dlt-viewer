#include "dltfileexporter.h"
#include "qdltmsg.h"

#include <qdltfilterlist.h>
#include <qdltfile.h>

#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

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

            std::size_t fileCounter = 1;
            std::size_t bytesCount = m_maxOutputSize.value_or(0);

            QFile output;
            for (int i = 0; i < m_input.size(); ++i) {
                if (m_maxOutputSize && (bytesCount >= *m_maxOutputSize)) {
                    output.close();

                    const auto outputName = outputDir + "/" + filterInfo.baseName() + "_" + QString::number(fileCounter) + ".dlt";
                    output.setFileName(outputName);
                    if (!output.open(QIODevice::WriteOnly))
                    {
                        qDebug() << "ERROR: Couldn't open output file: " << outputName;
                        return;
                    }
                    bytesCount = 0;
                    ++fileCounter;
                }

                auto res = getMessage(m_input, i);
                if (!res) {
                    continue;
                }
                auto [msg, buf] = *res;

                if (filterList.isEmpty() || filterList.checkFilter(msg)) {
                    bytesCount += buf.size();
                    output.write(buf);
                }
            }
        }
    } else {

        QDltFilterList filterList;
        for (const auto& filterFilepath : m_filters) {
            if(!filterList.LoadFilter(filterFilepath, false)) {
                qDebug() << "Export: Open filter file " << filterFilepath << " failed!";
            }
        }

        std::size_t fileCounter = 1;
        const QFileInfo info(outputName);
        std::size_t bytesCount = m_maxOutputSize.value_or(0);

        QFile output;
        for (int i = 0; i < m_input.size(); ++i) {
            if (m_maxOutputSize && (bytesCount >= *m_maxOutputSize)) {
                output.close();

                const auto outputName = info.absolutePath() + "/" + info.baseName() + "_" + QString::number(fileCounter) + ".dlt";
                output.setFileName(outputName);
                if (!output.open(QIODevice::WriteOnly))
                {
                    qDebug() << "ERROR: Couldn't open output file: " << outputName;
                    return;
                }
                bytesCount = 0;
                ++fileCounter;
            }

            auto res = getMessage(m_input, i);
            if (!res) {
                continue;
            }
            auto [msg, buf] = *res;

            if (filterList.isEmpty() || filterList.checkFilter(msg)) {
                bytesCount += buf.size();
                output.write(buf);
            }
        }
    }
}

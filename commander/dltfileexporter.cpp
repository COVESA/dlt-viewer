#include "dltfileexporter.h"

#include <qdltfilterlist.h>
#include <qdltfile.h>

#include <QFile>
#include <QDebug>
#include <QFileInfo>

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
        for (const auto& filterFilepath : m_filters) {
            QFileInfo info(filterFilepath);
            QDltFilterList filterList;
            if(!filterList.LoadFilter(filterFilepath, true)) {
                qDebug() << "Export: Open filter file " << filterFilepath << " failed!";
                continue;
            }

            QFile file(outputName + "/" + info.baseName() + ".dlt");
            if (!file.open(QIODevice::WriteOnly))
            {
                qDebug() << "ERROR: Couldn't open output file: " << outputName;
                continue;
            }

            for (int i = 0; i < m_input.size(); ++i) {
                QByteArray buf = m_input.getMsg(i);
                if(buf.isEmpty())
                {
                    qDebug() << "Buffer empty in" << __FILE__ << __LINE__;
                    continue;
                }
                QDltMsg msg;
                msg.setMsg(buf);
                msg.setIndex(i);
                bool isApplied = m_input.applyRegExStringMsg(msg);
                if(isApplied) msg.getMsg(buf,true);

                if (filterList.isEmpty() || filterList.checkFilter(msg))
                    file.write(buf);
            }
        }
    } else {

        QDltFilterList filterList;
        for (const auto& filterFilepath : m_filters) {
            if(!filterList.LoadFilter(filterFilepath, false)) {
                qDebug() << "Export: Open filter file " << filterFilepath << " failed!";
            }
        }

        // TODO: should we break and create a new file
        QFile output(outputName);
        if (!output.open(QIODevice::WriteOnly))
        {
            qDebug() << "ERROR: Couldn't open output file: " << outputName;
            return;
        }

        for (int i = 0; i < m_input.size(); ++i) {
            QByteArray buf = m_input.getMsg(i);
            if(buf.isEmpty())
            {
                qDebug() << "Buffer empty in" << __FILE__ << __LINE__;
                continue;
            }
            QDltMsg msg;
            msg.setMsg(buf);
            msg.setIndex(i);
            bool isApplied = m_input.applyRegExStringMsg(msg);
            if(isApplied) msg.getMsg(buf,true);

            if (filterList.isEmpty() || filterList.checkFilter(msg))
                output.write(buf);
        }
    }
}

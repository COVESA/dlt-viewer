#ifndef DLTFILEEXPORTER_H
#define DLTFILEEXPORTER_H

#include <QString>
#include <QStringList>

#include <optional>

class QDltFile;

class DltFileExporter
{
public:
    DltFileExporter(const QDltFile& input);

    void setFilterList(const QStringList& filters, bool splitByFilter);
    void setMaxOutputSize(std::size_t sz);

    void exportMessages(const QString& output);

private:
    const QDltFile& m_input;
    QStringList m_filters;
    bool m_splitByFilter{false};
    std::optional<std::size_t> m_maxOutputSize;
};

#endif // DLTFILEEXPORTER_H

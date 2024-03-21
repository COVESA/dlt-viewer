#ifndef QDLTEXPORTER_H
#define QDLTEXPORTER_H

#include <QObject>
#include <QFile>
#include <QModelIndexList>
//#include <QTreeWidget>

#include "export_rules.h"
#include "qdltfile.h"
#include "qdltmsg.h"
#include "qdltpluginmanager.h"
//#include "project.h"

class QDLT_EXPORT QDltExporter : public QObject
{
    Q_OBJECT

public:

    typedef enum { FormatDlt,FormatAscii,FormatCsv,FormatClipboard,FormatClipboardPayloadOnly,FormatDltDecoded,FormatUTF8,
                   FormatClipboardJiraTable, FormatClipboardJiraTableHead} DltExportFormat;

    typedef enum { SelectionAll,SelectionFiltered,SelectionSelected } DltExportSelection;

private:

    /* Add double quotes to a value.
     * Also escape any exiting double quotes with another double quote.
     * \param arg string to escape
     * \return QString which is escaped version of arg
     */
    QString escapeCSVValue(QString arg);

    /* Write the first line of CSV. This is just the names of the fields
     * \param file outputfile to write to
     * \return True if writing was succesfull, false if error occured
     */
    bool writeCSVHeader(QFile *file);

    /* Write the message out to an open file
     * \param index True index to QDltFile of the message
     * \param to File to write to
     * \param msg msg to get the data from
     */
    void writeCSVLine(int index, QFile *to, QDltMsg msg);

    bool start();
    bool finish();
    bool getMsg(unsigned long int num, QDltMsg &msg, QByteArray &buf);
    bool exportMsg(unsigned long int num, QDltMsg &msg,QByteArray &buf);

public:

    /* Default QT constructor.
     * Please pass a window as a parameter to parent dialogs correctly.
     */
    explicit QDltExporter(int _automaticTimeSettings,qlonglong _utcOffset,int _dst,QObject *parent = 0);

    /* Export some messages from QDltFile to a CSV file.
     * \param from QDltFile to pull messages from
     * \param to Regular file to export to
     * \param pluginManager The treewidget representing plugins. Needed to run decoders.
     * \param exportFormat
     * \param exportSelection
     * \param selection Limit export to these messages. Leave to NULL to export everything,
     */
    void exportMessages(QDltFile *from, QFile *to, QDltPluginManager *pluginManager,
                        QDltExporter::DltExportFormat exportFormat,
                        QDltExporter::DltExportSelection exportSelection, QModelIndexList *selection = 0);

    void exportMessageRange(unsigned long start, unsigned long stop);

signals:

public slots:

private:
    unsigned long int size;
    unsigned long int starting_index;
    unsigned long int stoping_index;
    QDltFile *from;
    QFile *to;
    QString clipboardString;
    QDltPluginManager *pluginManager;
    QModelIndexList *selection;
    QList<int> selectedRows;
    QDltExporter::DltExportFormat exportFormat;
    QDltExporter::DltExportSelection exportSelection;
    //Project *project;
    int automaticTimeSettings; // project and local setting
    qlonglong utcOffset; // project and local setting
    int dst; // project and local setting

};

#endif // QDLTEXPORTER_H

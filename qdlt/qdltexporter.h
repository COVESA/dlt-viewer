#ifndef QDLTEXPORTER_H
#define QDLTEXPORTER_H

#include <QObject>
#include <QThread>
#include <QFile>
#include <QModelIndexList>

#include "export_rules.h"
#include "qdltfile.h"
#include "qdltmsg.h"
#include "qdltpluginmanager.h"


#define QDLT_DEFAULT_EXPORT_SIGNATURE "ITSOEACNYUMRP"
#define QDLT_DEFAULT_EXPORT_DELIMITER ','

class QDLT_EXPORT QDltExporter : public QThread
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
     * \return True if writing was successful, false if error occurred
     */
    bool writeCSVHeader();

    /* Write the message out to an open file
     * \param index True index to QDltFile of the message
     * \param to File to write to
     * \param msg msg to get the data from
     */
    void writeCSVLine(int index, QDltMsg msg,QFile &to);

    bool startExport();
    bool finish();
    bool getMsg(unsigned long int num, QDltMsg &msg, QByteArray &buf);
    bool exportMsg(unsigned long int num, QDltMsg &msg,QByteArray &buf,QFile &to);

public:

    /* Default QT constructor.
     * Please pass a window as a parameter to parent dialogs correctly.
     */
    explicit QDltExporter(QDltFile *from, QString outputfileName, QDltPluginManager *pluginManager,
                        QDltExporter::DltExportFormat exportFormat,
                        QDltExporter::DltExportSelection exportSelection, QModelIndexList *selection, int _automaticTimeSettings,qlonglong _utcOffset,int _dst,char delimiter=QDLT_DEFAULT_EXPORT_DELIMITER,QString signature=QDLT_DEFAULT_EXPORT_SIGNATURE,QObject *parent = 0);

    void run() override;

    /* Export some messages from QDltFile to a CSV file.
     * \param from QDltFile to pull messages from
     * \param to Regular file to export to
     * \param pluginManager The treewidget representing plugins. Needed to run decoders.
     * \param exportFormat
     * \param exportSelection
     * \param selection Limit export to these messages. Leave to NULL to export everything,
     */
    void exportMessages();

    void exportMessageRange(unsigned long start, unsigned long stop);

    /* If a filter list is set, an additional filter is applied when exporting
     * \param filterList Copy of filter list
     */
    void setFilterList(QDltFilterList &filterList);

    /* Multifilter will be used if set to export to separate file for each Filter
     * \param filterListmultifilterNames All filenames of the filters
     */
    void setMultifilterFilenames(QStringList multifilterFilenames);

  signals:

    void clipboard(QString text);
    void progress(QString name,int status, int progress);
    void resultReady(const QString &s);

public slots:

private:
    unsigned long int size;
    unsigned long int starting_index;
    unsigned long int stoping_index;
    QDltFile *from;
    QFile to;
    QString clipboardString;
    QDltPluginManager *pluginManager;
    QModelIndexList *selection;
    QList<int> selectedRows;
    QDltExporter::DltExportFormat exportFormat;
    QDltExporter::DltExportSelection exportSelection;
    int automaticTimeSettings; // project and local setting
    qlonglong utcOffset; // project and local setting
    int dst; // project and local setting
    char delimiter;
    QDltFilterList filterList;
    QStringList multifilterFilenames;
    QList<QFile*> multifilterFilesList;
    QList<QDltFilterList*> multifilterFilterList;
    QString signature;
};

#endif // QDLTEXPORTER_H

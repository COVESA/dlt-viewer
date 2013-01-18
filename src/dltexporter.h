#ifndef DLTEXPORTER_H
#define DLTEXPORTER_H

#include <QObject>
#include <QFile>
#include <QModelIndexList>
#include <QTreeWidget>

#include "qdlt.h"

class DltExporter : public QObject
{
    Q_OBJECT
private:
    /* This is a re-implementation from MainWindow.
     * Iterate through plugins and apply the first one.
     * \param plugin TreeWidget from UI, containing all the plugins
     * \param msg QDltMessage to feed to plugin and receive the result in
     * \param triggeredByUser If this was triggered by user or framework
     */
    void iterateDecodersForMsg(QTreeWidget *plugins, QDltMsg &msg, int triggeredByUser);

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

    /* Check that the parameters are valid, opens the file, and writes the header.
     * \param from QDltFile to pull messages from
     * \param to Regular file to export to
     * \param plugins The treewidget representing plugins. Needed to run decoders.
     * \param limit Limit export to these messages. Leave to NULL to export everything
     * \return true if everything is ok. False if some parameters are unacceptable.
     * \sa exportCSV
     */
    bool prepareCSVExport(QDltFile *from, QFile *to, QTreeWidget *plugins, QModelIndexList *selection = NULL);

    /* Fill a message with data from QDltFile.
     * \param file QDltFile to get the message from
     * \param msg QDltMsg where to put the result
     * \param which Index to retrieve. Either of selection or complete file
     * \param selection Selection, if any, to get index from. Pass null to use direct index.
     * \return True index of retrieved msg in file. -1 if there was no data to retrieve. Not necessarily an error.
     */
    int getMsg(QDltFile *file, QDltMsg &msg, int which, QModelIndexList *selection);

public:

    /* Default QT constructor.
     * Please pass a window as a parameter to parent dialogs correctly.
     */
    explicit DltExporter(QObject *parent = 0);

    /* Export some messages from QDltFile to a CSV file.
     * \param from QDltFile to pull messages from
     * \param to Regular file to export to
     * \param plugins The treewidget representing plugins. Needed to run decoders.
     * \param limit Limit export to these messages. Leave to NULL to export everything
     */
    void exportCSV(QDltFile *from, QFile *to, QTreeWidget *plugins, QModelIndexList *selection = NULL);

signals:
    
public slots:
    
};

#endif // DLTEXPORTER_H

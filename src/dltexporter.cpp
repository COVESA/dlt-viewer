#include <QProgressDialog>
#include <QMessageBox>

#include "dltexporter.h"
#include "fieldnames.h"
#include "project.h"

DltExporter::DltExporter(QObject *parent) :
    QObject(parent)
{
}

void DltExporter::iterateDecodersForMsg(QTreeWidget *plugins, QDltMsg &msg, int triggeredByUser)
{
    for(int i = 0; i < plugins->topLevelItemCount (); i++)
    {
        PluginItem *item = (PluginItem*)plugins->topLevelItem(i);

        if(item->getMode() != item->ModeDisable &&
                item->plugindecoderinterface &&
                item->plugindecoderinterface->isMsg(msg,triggeredByUser))
        {
            item->plugindecoderinterface->decodeMsg(msg,triggeredByUser);
            break;
        }
    }
}

QString DltExporter::escapeCSVValue(QString arg)
{
    QString retval = arg.replace(QChar('\"'), QString("\"\""));
    retval = QString("\"%1\"").arg(retval);
    return retval;
}

bool DltExporter::writeCSVHeader(QFile *file)
{
    QString header("\"%1\",\"%2\",\"%3\",\"%4\",\"%5\",\"%6\",\"%7\",\"%8\",\"%9\",\"%10\",\"%11\",\"%12\"\r\n");
    header = header.arg(FieldNames::getName(FieldNames::Index))
                    .arg(FieldNames::getName(FieldNames::Time))
                    .arg(FieldNames::getName(FieldNames::TimeStamp))
                    .arg(FieldNames::getName(FieldNames::Counter))
                    .arg(FieldNames::getName(FieldNames::EcuId))
                    .arg(FieldNames::getName(FieldNames::AppId))
                    .arg(FieldNames::getName(FieldNames::ContextId))
                    .arg(FieldNames::getName(FieldNames::Type))
                    .arg(FieldNames::getName(FieldNames::Subtype))
                    .arg(FieldNames::getName(FieldNames::Mode))
                    .arg(FieldNames::getName(FieldNames::ArgCount))
                    .arg(FieldNames::getName(FieldNames::Payload));
    return file->write(header.toAscii().constData()) < 0 ? false : true;
}

void DltExporter::writeCSVLine(int index, QFile *to, QDltMsg msg)
{
    QString text("");

    text += escapeCSVValue(QString("%1").arg(index)).append(",");
    text += escapeCSVValue(QString("%1.%2").arg(msg.getTimeString()).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'))).append(",");
    text += escapeCSVValue(QString("%1.%2").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0'))).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getMessageCounter())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getEcuid())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getApid())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getCtid())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getTypeString())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getSubtypeString())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getModeString())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getNumberOfArguments())).append(",");
    text += escapeCSVValue(msg.toStringPayload());
    text += "\r\n";

    to->write(text.toAscii().constData());
}


bool DltExporter::prepareCSVExport(QDltFile *from, QFile *to, QTreeWidget *plugins, QModelIndexList *selection)
{
    /* Check that we actually have input file */
    if(from == NULL)
    {
        QMessageBox::critical(qobject_cast<QWidget *>(parent()), QString("DLT Viewer"),
                              QString("Dlt file not present in DltExporter."));
        return false;
    }

    /* The TreeWidget must exist, even if it is empty */
    if(plugins == NULL)
    {
        QMessageBox::critical(qobject_cast<QWidget *>(parent()), QString("DLT Viewer"),
                              QString("No plugins passed to DltExporter."));
        return false;
    }
    /* If we have selection list. It must contain something */
    if(selection != NULL && selection->count() <= 0)
    {
        QMessageBox::critical(qobject_cast<QWidget *>(parent()), QString("DLT Viewer"),
                              QString("No messages selected."));
        return false;
    }

    /* Sort the selection list. */
    if(selection != NULL)
    {
        qSort(selection->begin(), selection->end());
    }

    /* Try to open the export file */
    if(!to->open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(qobject_cast<QWidget *>(parent()), QString("DLT Viewer"),
                              QString("Cannot open the export file."));
        return false;
    }

    /* Write the first line of CSV file */
    if(!writeCSVHeader(to))
    {
        QMessageBox::critical(qobject_cast<QWidget *>(parent()), QString("DLT Viewer"),
                              QString("Cannot write to export file."));
        return false;
    }
    return true;
}

int DltExporter::getMsg(QDltFile *file, QDltMsg &msg, int which, QModelIndexList *selection)
{
    // Simply get directly from index if theres no selection
    if(selection == NULL)
    {
        return file->getMsg(which, msg) ? which : -1;
    }

    // Retrieve one index from model...
    QModelIndex index = selection->at(which);

    // On new row...
    if(index.column() == 0)
    {
        // Get msg data from file that refers to this row
        QByteArray data = file->getMsgFilter(index.row());
        if(data.isEmpty())
        {
            return -1;
        }
        msg.setMsg(data);
        return file->getMsgFilterPos(index.row());
    }
    else
    {
        return -1;
    }
}


void DltExporter::exportCSV(QDltFile *from, QFile *to, QTreeWidget *plugins, QModelIndexList *selection)
{
    if(!prepareCSVExport(from, to, plugins, selection))
    {
        return;
    }

    int maxProgress = 0;
    if(selection == NULL)
    {
        maxProgress = from->size();
    }
    else
    {
        maxProgress = selection->count();
    }

    QProgressDialog fileprogress("Export...", "Cancel", 0, maxProgress, qobject_cast<QWidget *>(parent()));
    fileprogress.setWindowTitle("DLT Viewer");
    fileprogress.setWindowModality(Qt::WindowModal);
    fileprogress.show();

    QDltMsg currentMessage;

    for(int i = 0; i < maxProgress;i++)
    {
        /* Update progress dialog every 0.5% */
        if((i%(maxProgress/200+1)) == 0)
        {
            fileprogress.setValue(i);
        }
        int msgIndex = getMsg(from, currentMessage, i, selection);
        if(msgIndex < 0)
        {
            // Skip index if no data was retrieved
            continue;
        }
        iterateDecodersForMsg(plugins, currentMessage, 1);
        writeCSVLine(msgIndex, to, currentMessage);
    }
    to->close();
}

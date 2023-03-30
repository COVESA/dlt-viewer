#include <algorithm>
#include <QProgressDialog>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QDebug>

#include "dltexporter.h"
#include "fieldnames.h"
#include "qdltoptmanager.h"

DltExporter::DltExporter(QObject *parent) :
    QObject(parent)
{
    size = 0;
    from = NULL;
    to = NULL;
    pluginManager = NULL;
    selection = NULL;
    exportFormat = FormatDlt;
    exportSelection = SelectionAll;
    starting_index=0;
    stoping_index=0;
}

QString DltExporter::escapeCSVValue(QString arg)
{
    QString retval = arg.replace(QChar('\"'), QString("\"\""));
    retval = QString("\"%1\"").arg(retval);
    return retval;
}

bool DltExporter::writeCSVHeader(QFile *file)
{
    QString header("\"%1\",\"%2\",\"%3\",\"%4\",\"%5\",\"%6\",\"%7\",\"%8\",\"%9\",\"%10\",\"%11\",\"%12\",\"%13\"\n");
    header = header.arg(FieldNames::getName(FieldNames::Index))
                    .arg(FieldNames::getName(FieldNames::Time))
                    .arg(FieldNames::getName(FieldNames::TimeStamp))
                    .arg(FieldNames::getName(FieldNames::Counter))
                    .arg(FieldNames::getName(FieldNames::EcuId))
                    .arg(FieldNames::getName(FieldNames::AppId))
                    .arg(FieldNames::getName(FieldNames::ContextId))
                    .arg(FieldNames::getName(FieldNames::SessionId))
                    .arg(FieldNames::getName(FieldNames::Type))
                    .arg(FieldNames::getName(FieldNames::Subtype))
                    .arg(FieldNames::getName(FieldNames::Mode))
                    .arg(FieldNames::getName(FieldNames::ArgCount))
                    .arg(FieldNames::getName(FieldNames::Payload));
    return file->write(header.toLatin1().constData()) < 0 ? false : true;
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
    text += escapeCSVValue(QString("%1").arg(msg.getSessionid())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getTypeString())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getSubtypeString())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getModeString())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getNumberOfArguments())).append(",");
    text += escapeCSVValue(msg.toStringPayload().simplified());
    text += "\n";

    to->write(text.toLatin1().constData());
}

bool DltExporter::start()
{
    /* Sort the selection list and create Row list */
    if(exportSelection == DltExporter::SelectionSelected && selection != NULL)
    {
        std::sort(selection->begin(), selection->end());
        selectedRows.clear();
        for(int num=0;num<selection->count();num++)
        {
            QModelIndex index = selection->at(num);
            if(index.column() == 0)
                selectedRows.append(index.row());
        }
    }

    /* open the export file */
    if(exportFormat == DltExporter::FormatAscii ||
       exportFormat == DltExporter::FormatUTF8 ||
       exportFormat == DltExporter::FormatCsv)
    {
        if(!to->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            if ( true == QDltOptManager::getInstance()->issilentMode() )
             {
             qDebug() << QString("ERROR - cannot open the export file %1").arg(to->fileName());
             }
            else
            QMessageBox::critical(qobject_cast<QWidget *>(parent()), QString("DLT Viewer"),
                                  QString("Cannot open the export file %1").arg(to->fileName()));
            return false;
        }
    }
    else if((exportFormat == DltExporter::FormatDlt)||(exportFormat == DltExporter::FormatDltDecoded))
    {
        if(!to->open(QIODevice::WriteOnly))
        {
            if ( true == QDltOptManager::getInstance()->issilentMode() )
             {
             qDebug() << QString("ERROR - cannot open the export file %1").arg(to->fileName());
             }
            else
            QMessageBox::critical(qobject_cast<QWidget *>(parent()), QString("DLT Viewer"),
                                  QString("Cannot open the export file %1").arg(to->fileName()));
            return false;
        }
    }

    /* write CSV header if CSV export */
    if(exportFormat == DltExporter::FormatCsv)
    {
        /* Write the first line of CSV file */
        if(!writeCSVHeader(to))
        {
            if ( true == QDltOptManager::getInstance()->issilentMode() )
             {
             qDebug() << QString("ERROR - cannot open the export file %1").arg(to->fileName());
             }
            else
            QMessageBox::critical(qobject_cast<QWidget *>(parent()), QString("DLT Viewer"),
                                  QString("Cannot open the export file %1").arg(to->fileName()));
            return false;
        }
    }

    /* calculate size */
    if(exportSelection == DltExporter::SelectionAll)
        size = from->size();
    else if(exportSelection == DltExporter::SelectionFiltered)
        size = from->sizeFilter();
    else if(exportSelection == DltExporter::SelectionSelected)
        size = selectedRows.size();
    else
        return false;

    if(exportFormat == DltExporter::FormatClipboardJiraTableHead)
    {
        clipboardString = "||" "Index"
                          "||" "Time" "||" "Timestamp"
                          "||" "EcuID"
                          "||" "AppID" "||" "CtxID"
                          "||" "Payload"
                          "||" "Comment"
                          "||\n";
    }
    /* success */
    return true;
}

bool DltExporter::finish()
{

    if(exportFormat == DltExporter::FormatAscii ||
       exportFormat == DltExporter::FormatUTF8 ||
       exportFormat == DltExporter::FormatCsv ||
       exportFormat == DltExporter::FormatDlt ||
       exportFormat == DltExporter::FormatDltDecoded)
    {
        /* close output file */
        to->close();
    }
    else if (exportFormat == DltExporter::FormatClipboard ||
             exportFormat == DltExporter::FormatClipboardPayloadOnly ||
             exportFormat == DltExporter::FormatClipboardJiraTable ||
             exportFormat == DltExporter::FormatClipboardJiraTableHead)
    {
        /* export to clipboard */
        QClipboard *clipboard = QApplication::clipboard();

        /*remove '\n' from the string*/
        if (clipboardString.endsWith('\n'))
        {
            clipboardString.resize(clipboardString.size() - 1);
        }
        clipboard->setText(clipboardString);
    }

    return true;
}

bool DltExporter::getMsg(unsigned long int num,QDltMsg &msg,QByteArray &buf)
{
    bool result;
    buf.clear();
    if(exportSelection == DltExporter::SelectionAll)
    {
        buf = from->getMsg(num);
        if( true == buf.isEmpty())
        {
            qDebug() << "Buffer empty in" << __FILE__ << __LINE__;
            return false;
        }
        result =  msg.setMsg(buf);
        msg.setIndex(num);
    }
    else if(exportSelection == DltExporter::SelectionFiltered)
    {
        buf = from->getMsgFilter(num);
        if( true == buf.isEmpty())
        {
            qDebug() << "Buffer empty in" << __FILE__ << __LINE__;
            return false;
        }
        result =  msg.setMsg(buf);
        msg.setIndex(from->getMsgFilterPos(num));
    }
    else if(exportSelection == DltExporter::SelectionSelected)
    {
        buf = from->getMsgFilter(selectedRows[num]);
        if( true == buf.isEmpty())
        {
            qDebug() << "Buffer empty in" << __FILE__ << __LINE__;
            return false;
        }
        result =  msg.setMsg(buf);
        msg.setIndex(from->getMsgFilterPos(selectedRows[num]));
    }
    else
    {
        qDebug() << "Unhandled error in" << __FILE__ << __LINE__;
        return false;
    }

    return result;
}

bool DltExporter::exportMsg(unsigned long int num, QDltMsg &msg, QByteArray &buf)
{
    if((exportFormat == DltExporter::FormatDlt)||(exportFormat == DltExporter::FormatDltDecoded))
    {
        to->write(buf);
    }
    else if(exportFormat == DltExporter::FormatAscii ||
            exportFormat == DltExporter::FormatUTF8  ||
            exportFormat == DltExporter::FormatClipboard ||
            exportFormat == DltExporter::FormatClipboardPayloadOnly)
    {
        QString text;

        /* get message ASCII text */
        if(exportFormat != DltExporter::FormatClipboardPayloadOnly)
        {
            if(exportSelection == DltExporter::SelectionAll)
                text += QString("%1 ").arg(num);
            else if(exportSelection == DltExporter::SelectionFiltered)
                text += QString("%1 ").arg(from->getMsgFilterPos(num));
            else if(exportSelection == DltExporter::SelectionSelected)
                text += QString("%1 ").arg(from->getMsgFilterPos(selectedRows[num]));
            else
                return false;
            text += msg.toStringHeader();
            text += " ";
        }
        text += msg.toStringPayload().trimmed();
        text += "\n";
        try
         {
            if(exportFormat == DltExporter::FormatAscii)
                /* write to file */
                to->write(text.toLatin1().constData());
            else if (exportFormat == DltExporter::FormatUTF8)
                to->write(text.toUtf8().constData());
            else if(exportFormat == DltExporter::FormatClipboard ||
                    exportFormat == DltExporter::FormatClipboardPayloadOnly)
                clipboardString += text;
         }
        catch (...)
         {
         }
    }
    else if(exportFormat == DltExporter::FormatCsv)
    {
        if(exportSelection == DltExporter::SelectionAll)
            writeCSVLine(num, to, msg);
        else if(exportSelection == DltExporter::SelectionFiltered)
            writeCSVLine(from->getMsgFilterPos(num), to, msg);
        else if(exportSelection == DltExporter::SelectionSelected)
            writeCSVLine(from->getMsgFilterPos(selectedRows[num]), to, msg);
        else
            return false;
    }
    else if ((exportFormat == DltExporter::FormatClipboardJiraTable) ||
             (exportFormat == DltExporter::FormatClipboardJiraTableHead) )
    {
        QString text = "|";

        if(exportSelection == DltExporter::SelectionAll)
            text += QString("%1").arg(num);
        else if(exportSelection == DltExporter::SelectionFiltered)
            text += QString("%1").arg(from->getMsgFilterPos(num));
        else if(exportSelection == DltExporter::SelectionSelected)
            text += QString("%1").arg(from->getMsgFilterPos(selectedRows[num]));
        else
            return false;

        text += "|" + QString("%1.%2").arg(msg.getTimeString()).arg(msg.getMicroseconds(),4,10,QLatin1Char('0')) +
                "|" + QString("%1.%2").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0')) +
                "|" + msg.getEcuid() +
                "|" + msg.getApid() +
                "|" + msg.getCtid() +
                "|" + msg.toStringPayload().trimmed().replace('|', "\\|").replace('#', "\\#").replace('*', "\\*") +
                "| |\n";
        clipboardString += text;
    }
    return true;
}


void DltExporter::exportMessageRange(unsigned long start, unsigned long stop)
{
    this->starting_index=start;
    this->stoping_index=stop;
}

void DltExporter::exportMessages(QDltFile *from, QFile *to, QDltPluginManager *pluginManager,
                                 DltExporter::DltExportFormat exportFormat,
                                 DltExporter::DltExportSelection exportSelection, QModelIndexList *selection)
{
    QDltMsg msg;
    QByteArray buf;
    float percent=0;
    QString qszPercent;
    /* initialise values */
    int readErrors=0;
    int exportErrors=0;
    int exportCounter=0;
    int startFinishError=0;
    this->from = from;
    this->to = to;
    clipboardString.clear();
    this->pluginManager = pluginManager;
    this->selection = selection;
    this->exportFormat = exportFormat;
    this->exportSelection = exportSelection;
    unsigned long int starting = 0;
    unsigned long int stoping = this->size;
    /* start export */
    if(false == start())
    {
        qDebug() << "DLT Export start() failed";
        startFinishError++;
        return;
    }


    bool silentMode = !QDltOptManager::getInstance()->issilentMode();

    if ( this->stoping_index == 0 || this->stoping_index > this->size || this->stoping_index < this->starting_index )
    {
        stoping = this->size;
        starting = 0;
        qDebug() << "Start DLT export of" << this->size << "messages" << ",silent mode" << !silentMode;
    }
    else
    {
        stoping = this->stoping_index;
        starting = this->starting_index;
        qDebug() << "Start DLT export" << stoping - starting << "messages" << "of" << this->size << "range: " << starting << "-" << stoping << ",silent mode" << !silentMode;
    }

    /* init fileprogress */

    QProgressDialog fileprogress("Export ...", "Cancel", 0, stoping, qobject_cast<QWidget *>(parent()));
    if (silentMode == true)
     {
      fileprogress.setWindowTitle("DLT Viewer");
      fileprogress.setWindowModality(Qt::WindowModal);
      fileprogress.show();
     }

    for(starting;starting<stoping;starting++)
    {
        // Update progress dialog every 1000 lines

        if( 0 == (starting%1000))
        {
          percent=(( starting * 100.0 ) /stoping );
          if (silentMode == true)
             {
              fileprogress.setValue(starting);
             }
          if( 0 == (starting%1000000))
          {
           qszPercent = QString("Exported: %1 %").arg(percent, 0, 'f',2);
           qDebug().noquote() << qszPercent;
          }
        }

        if (fileprogress.wasCanceled() == true)
        {
            qDebug().noquote() << "Export canceled !";
            return;
        }

        // get message
        if(false == getMsg(starting,msg,buf))
        {
        //  finish();
        //qDebug() << "DLT Export getMsg failed on msg index" << starting;
        readErrors++;
        continue;
        //  return;
        }
        // decode message if needed
        if(exportFormat != DltExporter::FormatDlt)
        {
            pluginManager->decodeMsg(msg,silentMode);
            if (exportFormat == DltExporter::FormatDltDecoded)
            {
                msg.setNumberOfArguments(msg.sizeArguments());
                msg.getMsg(buf,true);
            }
        }

        // export message
        if(!exportMsg(starting,msg,buf))
        {
            // finish();
          //qDebug() << "DLT Export exportMsg() failed";
          exportErrors++;
          continue;
        }

     else
        exportCounter++;
    } // for loop

    if (silentMode == true)
    {
     fileprogress.close();
    }


    if (!finish())
    {
        startFinishError++;
    }


    if ( startFinishError>0 || readErrors>0 || exportErrors>0 )
    {
       //qDebug() << "DLT Export finish() failed";
       if (silentMode == true ) // reversed login in this case !
       {
        QMessageBox::warning(NULL,"Export Errors!",QString("Exported successful: %1 / %2\n\nReadErrors:%3\nWriteErrors:%4\nStart/Finish errors:%5").arg(exportCounter).arg(size).arg(readErrors).arg(exportErrors).arg(startFinishError));
       }
       return;
    }
    if ( stoping != 0 )
    {
     percent=(( starting * 100.0 ) / stoping );
    }
    else
    {
     percent = 0;
    }

    qDebug() << percent << "%" << "DLT export done for" << exportCounter << "messages with result" << startFinishError;// << __FILE__ << __LINE__;
}

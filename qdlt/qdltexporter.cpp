#include <algorithm>
#include <QDebug>
#include <QFile>

#include "qdltexporter.h"
#include "fieldnames.h"
#include "qdltoptmanager.h"

QDltExporter::QDltExporter(int _automaticTimeSettings,qlonglong _utcOffset,int _dst,QObject *parent) :
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
    automaticTimeSettings=_automaticTimeSettings;
    utcOffset=_utcOffset;
    dst=_dst;
}

QString QDltExporter::escapeCSVValue(QString arg)
{
    QString retval = arg.replace(QChar('\"'), QString("\"\""));
    retval = QString("\"%1\"").arg(retval);
    return retval;
}

bool QDltExporter::writeCSVHeader(QFile *file)
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

void QDltExporter::writeCSVLine(int index, QFile *to, QDltMsg msg)
{
    QString text("");

    text += escapeCSVValue(QString("%1").arg(index)).append(",");
    if( automaticTimeSettings == 0 )
       text += escapeCSVValue(QString("%1.%2").arg(msg.getGmTimeWithOffsetString(utcOffset,dst)).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'))).append(",");
    else
       text += escapeCSVValue(QString("%1.%2").arg(msg.getTimeString()).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'))).append(",");
    text += escapeCSVValue(QString("%1.%2").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0'))).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getMessageCounter())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getEcuid().simplified())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getApid().simplified())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getCtid().simplified())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getSessionid())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getTypeString())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getSubtypeString())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getModeString())).append(",");
    text += escapeCSVValue(QString("%1").arg(msg.getNumberOfArguments())).append(",");
    text += escapeCSVValue(msg.toStringPayload().simplified().remove(QChar::Null));
    text += "\n";

    to->write(text.toLatin1().constData());
}

bool QDltExporter::start()
{
    /* Sort the selection list and create Row list */
    if(exportSelection == QDltExporter::SelectionSelected && selection != NULL)
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
    if(exportFormat == QDltExporter::FormatAscii ||
       exportFormat == QDltExporter::FormatUTF8 ||
       exportFormat == QDltExporter::FormatCsv)
    {
        if(!to->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            if ( true == QDltOptManager::getInstance()->issilentMode() )
             {
             qDebug() << QString("ERROR - cannot open the export file %1").arg(to->fileName());
             }
            else
                ;//QMessageBox::critical(qobject_cast<QWidget *>(parent()), QString("DLT Viewer"),
                 //                 QString("Cannot open the export file %1").arg(to->fileName()));
            return false;
        }
    }
    else if((exportFormat == QDltExporter::FormatDlt)||(exportFormat == QDltExporter::FormatDltDecoded))
    {
        if(!to->open(QIODevice::WriteOnly))
        {
            if ( true == QDltOptManager::getInstance()->issilentMode() )
             {
             qDebug() << QString("ERROR - cannot open the export file %1").arg(to->fileName());
             }
            else
                ;//QMessageBox::critical(qobject_cast<QWidget *>(parent()), QString("DLT Viewer"),
                 //                 QString("Cannot open the export file %1").arg(to->fileName()));
            return false;
        }
    }

    /* write CSV header if CSV export */
    if(exportFormat == QDltExporter::FormatCsv)
    {
        /* Write the first line of CSV file */
        if(!writeCSVHeader(to))
        {
            if ( true == QDltOptManager::getInstance()->issilentMode() )
             {
             qDebug() << QString("ERROR - cannot open the export file %1").arg(to->fileName());
             }
            else
                ;//QMessageBox::critical(qobject_cast<QWidget *>(parent()), QString("DLT Viewer"),
                 //                 QString("Cannot open the export file %1").arg(to->fileName()));
            return false;
        }
    }

    /* calculate size */
    if(exportSelection == QDltExporter::SelectionAll)
        size = from->size();
    else if(exportSelection == QDltExporter::SelectionFiltered)
        size = from->sizeFilter();
    else if(exportSelection == QDltExporter::SelectionSelected)
        size = selectedRows.size();
    else
        return false;

    if(exportFormat == QDltExporter::FormatClipboardJiraTableHead)
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

bool QDltExporter::finish()
{

    if(exportFormat == QDltExporter::FormatAscii ||
       exportFormat == QDltExporter::FormatUTF8 ||
       exportFormat == QDltExporter::FormatCsv ||
       exportFormat == QDltExporter::FormatDlt ||
       exportFormat == QDltExporter::FormatDltDecoded)
    {
        /* close output file */
        to->close();
    }
    else if (exportFormat == QDltExporter::FormatClipboard ||
             exportFormat == QDltExporter::FormatClipboardPayloadOnly ||
             exportFormat == QDltExporter::FormatClipboardJiraTable ||
             exportFormat == QDltExporter::FormatClipboardJiraTableHead)
    {
        /*remove '\n' from the string*/
        if (clipboardString.endsWith('\n'))
        {
            clipboardString.resize(clipboardString.size() - 1);
        }
        /* remove null characters */
        clipboardString.remove(QChar::Null);
        emit clipboard(clipboardString);
    }

    return true;
}

bool QDltExporter::getMsg(unsigned long int num,QDltMsg &msg,QByteArray &buf)
{
    bool result;
    buf.clear();
    if(exportSelection == QDltExporter::SelectionAll)
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
    else if(exportSelection == QDltExporter::SelectionFiltered)
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
    else if(exportSelection == QDltExporter::SelectionSelected)
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

bool QDltExporter::exportMsg(unsigned long int num, QDltMsg &msg, QByteArray &buf)
{
    if((exportFormat == QDltExporter::FormatDlt)||(exportFormat == QDltExporter::FormatDltDecoded))
    {
        to->write(buf);
    }
    else if(exportFormat == QDltExporter::FormatAscii ||
            exportFormat == QDltExporter::FormatUTF8  ||
            exportFormat == QDltExporter::FormatClipboard ||
            exportFormat == QDltExporter::FormatClipboardPayloadOnly)
    {
        QString text;

        /* get message ASCII text */
        if(exportFormat != QDltExporter::FormatClipboardPayloadOnly)
        {
            if(exportSelection == QDltExporter::SelectionAll)
                text += QString("%1 ").arg(num);
            else if(exportSelection == QDltExporter::SelectionFiltered)
                text += QString("%1 ").arg(from->getMsgFilterPos(num));
            else if(exportSelection == QDltExporter::SelectionSelected)
                text += QString("%1 ").arg(from->getMsgFilterPos(selectedRows[num]));
            else
                return false;
            if( automaticTimeSettings == 0 )
               text += QString("%1.%2").arg(msg.getGmTimeWithOffsetString(utcOffset,dst)).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'));
            else
               text += QString("%1.%2").arg(msg.getTimeString()).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'));
            text += QString(" %1.%2").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0'));
            text += QString(" %1").arg(msg.getMessageCounter());
            text += QString(" %1").arg(msg.getEcuid());
            text += QString(" %1").arg(msg.getApid());
            text += QString(" %1").arg(msg.getCtid());
            text += QString(" %1").arg(msg.getSessionid());
            text += QString(" %2").arg(msg.getTypeString());
            text += QString(" %2").arg(msg.getSubtypeString());
            text += QString(" %2").arg(msg.getModeString());
            text += QString(" %1").arg(msg.getNumberOfArguments());

            text += " ";
        }
        text += msg.toStringPayload().simplified().remove(QChar::Null);
        text += "\n";
        try
         {
            if(exportFormat == QDltExporter::FormatAscii)
                /* write to file */
                to->write(text.toLatin1().constData());
            else if (exportFormat == QDltExporter::FormatUTF8)
                to->write(text.toUtf8().constData());
            else if(exportFormat == QDltExporter::FormatClipboard ||
                    exportFormat == QDltExporter::FormatClipboardPayloadOnly)
                clipboardString += text;
         }
        catch (...)
         {
         }
    }
    else if(exportFormat == QDltExporter::FormatCsv)
    {
        if(exportSelection == QDltExporter::SelectionAll)
            writeCSVLine(num, to, msg);
        else if(exportSelection == QDltExporter::SelectionFiltered)
            writeCSVLine(from->getMsgFilterPos(num), to, msg);
        else if(exportSelection == QDltExporter::SelectionSelected)
            writeCSVLine(from->getMsgFilterPos(selectedRows[num]), to, msg);
        else
            return false;
    }
    else if ((exportFormat == QDltExporter::FormatClipboardJiraTable) ||
             (exportFormat == QDltExporter::FormatClipboardJiraTableHead) )
    {
        QString text = "|";

        if(exportSelection == QDltExporter::SelectionAll)
            text += QString("%1").arg(num);
        else if(exportSelection == QDltExporter::SelectionFiltered)
            text += QString("%1").arg(from->getMsgFilterPos(num));
        else if(exportSelection == QDltExporter::SelectionSelected)
            text += QString("%1").arg(from->getMsgFilterPos(selectedRows[num]));
        else
            return false;

        if( automaticTimeSettings == 0 )
           text += "|" + QString("%1.%2").arg(msg.getGmTimeWithOffsetString(utcOffset,dst)).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'));
        else
           text += "|" + QString("%1.%2").arg(msg.getTimeString()).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'));
        text += "|" + QString("%1.%2").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0')) +
                "|" + msg.getEcuid() +
                "|" + msg.getApid() +
                "|" + msg.getCtid() +
                "|" + msg.toStringPayload().simplified().remove(QChar::Null).replace('|', "\\|").replace('#', "\\#").replace('*', "\\*") +
                "| |\n";
        clipboardString += text;
    }
    return true;
}


void QDltExporter::exportMessageRange(unsigned long start, unsigned long stop)
{
    this->starting_index=start;
    this->stoping_index=stop;
}

void QDltExporter::exportMessages(QDltFile *from, QFile *to, QDltPluginManager *pluginManager,
                                 QDltExporter::DltExportFormat exportFormat,
                                 QDltExporter::DltExportSelection exportSelection, QModelIndexList *selection)
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

    int progressCounter = 1;
    emit progress("Exp",1,0);

    for(;starting<stoping;starting++)
    {
        int percent = (( starting * 100.0 ) /stoping );
        if(percent>=progressCounter)
        {
            progressCounter += 1;
            emit progress("MF4:",2,percent); // every 1%
            if((percent>0) && ((percent%10)==0))
                qDebug() << "Exported:" << percent << "%"; // every 10%
        }

        // TODO: Handle cancel operation

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
        if(exportFormat != QDltExporter::FormatDlt)
        {
            if(pluginManager)
                pluginManager->decodeMsg(msg,silentMode);
            if (exportFormat == QDltExporter::FormatDltDecoded)
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

    emit progress("",3,100);

    if (!finish())
    {
        startFinishError++;
    }


    if ( startFinishError>0 || readErrors>0 || exportErrors>0 )
    {
       //qDebug() << "DLT Export finish() failed";
       if (silentMode == true ) // reversed login in this case !
       {
           ;//QMessageBox::warning(NULL,"Export Errors!",QString("Exported successful: %1 / %2\n\nReadErrors:%3\nWriteErrors:%4\nStart/Finish errors:%5").arg(exportCounter).arg(size).arg(readErrors).arg(exportErrors).arg(startFinishError));
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

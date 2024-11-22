#include <algorithm>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

#include "qdltexporter.h"
#include "fieldnames.h"
#include "qdltoptmanager.h"

QDltExporter::QDltExporter(QDltFile *from, QString outputfileName, QDltPluginManager *pluginManager,
                           QDltExporter::DltExportFormat exportFormat,
                           QDltExporter::DltExportSelection exportSelection, QModelIndexList *selection, int _automaticTimeSettings,qlonglong _utcOffset,int _dst,char _delimiter,QObject *parent) :
    QThread(parent)
{
    size = 0;
    starting_index=0;
    stoping_index=0;
    automaticTimeSettings=_automaticTimeSettings;
    utcOffset=_utcOffset;
    dst=_dst;
    delimiter=_delimiter;

    this->from = from;
    to.setFileName(outputfileName);
    this->pluginManager = pluginManager;
    this->selection = selection;
    this->exportFormat = exportFormat;
    this->exportSelection = exportSelection;
    this->selection = selection;
}

void QDltExporter::run()
{
    QString result;
    exportMessages();
    emit resultReady(result);
}

QString QDltExporter::escapeCSVValue(QString arg)
{
    QString retval = arg.replace(QChar('\"'), QString("\"\""));
    retval = QString("\"%1\"").arg(retval);
    return retval;
}

bool QDltExporter::writeCSVHeader()
{
    QString header = QString("\"%1\"")+delimiter+QString("\"%2\"")+delimiter+QString("\"%3\"")+delimiter+QString("\"%4\"")+delimiter+QString("\"%5\"")+delimiter+QString("\"%6\"")+delimiter+QString("\"%7\"")+delimiter+QString("\"%8\"")+delimiter+QString("\"%9\"")+delimiter+QString("\"%10\"")+delimiter+QString("\"%11\"")+delimiter+QString("\"%12\"")+delimiter+QString("\"%13\"\n");
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
    if(multifilterFilenames.isEmpty())
        to.write(header.toLatin1().constData());
    else
    {
        for(auto file: multifilterFilesList)
            file->write(header.toLatin1().constData());
    }
    return true;
}

void QDltExporter::writeCSVLine(int index, QDltMsg msg,QFile &to)
{
    QString text("");

    text += escapeCSVValue(QString("%1").arg(index)).append(delimiter);
    if( automaticTimeSettings == 0 )
       text += escapeCSVValue(QString("%1.%2").arg(msg.getGmTimeWithOffsetString(utcOffset,dst)).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'))).append(delimiter);
    else
       text += escapeCSVValue(QString("%1.%2").arg(msg.getTimeString()).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'))).append(delimiter);
    text += escapeCSVValue(QString("%1.%2").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0'))).append(delimiter);
    text += escapeCSVValue(QString("%1").arg(msg.getMessageCounter())).append(delimiter);
    text += escapeCSVValue(QString("%1").arg(msg.getEcuid().simplified())).append(delimiter);
    text += escapeCSVValue(QString("%1").arg(msg.getApid().simplified())).append(delimiter);
    text += escapeCSVValue(QString("%1").arg(msg.getCtid().simplified())).append(delimiter);
    text += escapeCSVValue(QString("%1").arg(msg.getSessionid())).append(delimiter);
    text += escapeCSVValue(QString("%1").arg(msg.getTypeString())).append(delimiter);
    text += escapeCSVValue(QString("%1").arg(msg.getSubtypeString())).append(delimiter);
    text += escapeCSVValue(QString("%1").arg(msg.getModeString())).append(delimiter);
    text += escapeCSVValue(QString("%1").arg(msg.getNumberOfArguments())).append(delimiter);
    QString payload = msg.toStringPayload().simplified().remove(QChar::Null);
    if(from) from->applyRegExString(msg,payload);
    text += escapeCSVValue(payload);
    text += "\n";

    to.write(text.toLatin1().constData());
}

bool QDltExporter::startExport()
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
        if(multifilterFilenames.isEmpty())
        {
            if(!to.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                if (QDltOptManager::getInstance()->issilentMode())
                    qDebug() << QString("ERROR - cannot open the export file %1").arg(to.fileName());
                return false;
            }
        }
        else
        {
            for(auto filename : multifilterFilenames)
            {
                QFileInfo info(filename);
                info.baseName();
                QFile *file;
                if(exportFormat == QDltExporter::FormatAscii)
                    file = new QFile(to.fileName()+"/"+info.baseName()+".txt");
                else if(exportFormat == QDltExporter::FormatUTF8)
                    file = new QFile(to.fileName()+"/"+info.baseName()+".txt");
                else if(exportFormat == QDltExporter::FormatCsv)
                    file = new QFile(to.fileName()+"/"+info.baseName()+".csv");
                QDltFilterList *filterList = new QDltFilterList();
                if(!filterList->LoadFilter(filename,true))
                    qDebug() << "Export: Open filter file " << filename << " failed!";
                if(!filterList->isEmpty() && file->open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    qDebug() << "Multifilter export filename: " << file->fileName();
                    multifilterFilesList.append(file);
                    multifilterFilterList.append(filterList);
                }
                else
                {
                    qDebug() << "Export: Export multifilter file " << file->fileName() << " failed!";
                    delete file;
                    delete filterList;
                }
            }

        }
    }
    else if((exportFormat == QDltExporter::FormatDlt)||(exportFormat == QDltExporter::FormatDltDecoded))
    {
        if(multifilterFilenames.isEmpty())
        {
            if(!to.open(QIODevice::WriteOnly))
            {
                if (QDltOptManager::getInstance()->issilentMode())
                    qDebug() << QString("ERROR - cannot open the export file %1").arg(to.fileName());
                return false;
            }
        }
        else
        {
            for(auto filename : multifilterFilenames)
            {
                QFileInfo info(filename);
                info.baseName();
                QFile *file;
                file = new QFile(to.fileName()+"/"+info.baseName()+".dlt");
                QDltFilterList *filterList = new QDltFilterList();
                if(!filterList->LoadFilter(filename,true))
                    qDebug() << "Export: Open filter file " << filename << " failed!";
                qDebug() << "Multifilter export filename: " << file->fileName();
                if(!filterList->isEmpty() && file->open(QIODevice::WriteOnly))
                {
                    multifilterFilesList.append(file);
                    multifilterFilterList.append(filterList);
                }
                else
                {
                    delete file;
                    delete filterList;
                    qDebug() << "Export: Export multifilter file " << filename << " failed!";
                }
            }

        }
    }

    /* write CSV header if CSV export */
    if(exportFormat == QDltExporter::FormatCsv)
    {
        /* Write the first line of CSV file */
        if(!writeCSVHeader())
        {
            if(QDltOptManager::getInstance()->issilentMode())
            {
                qDebug() << QString("ERROR - cannot open the export file %1").arg(to.fileName());
            }
            //else
                //QMessageBox::critical(qobject_cast<QWidget *>(parent()), QString("DLT Viewer"),
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
        if(multifilterFilenames.isEmpty())
            to.close();
        else
        {
            for(auto file : multifilterFilesList)
            {
                file->close();
                delete file;
            }
            for(auto filterList : multifilterFilterList)
            {
                delete filterList;
            }
            multifilterFilterList.clear();
            multifilterFilesList.clear();
        }
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

bool QDltExporter::exportMsg(unsigned long int num, QDltMsg &msg, QByteArray &buf,QFile &to)
{
    if((exportFormat == QDltExporter::FormatDlt)||(exportFormat == QDltExporter::FormatDltDecoded))
    {
        to.write(buf);
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
        QString payload = msg.toStringPayload().simplified().remove(QChar::Null);
        if(from) from->applyRegExString(msg,payload);
        text += payload;
        text += "\n";
        try
         {
            if(exportFormat == QDltExporter::FormatAscii)
                /* write to file */
                to.write(text.toLatin1().constData());
            else if (exportFormat == QDltExporter::FormatUTF8)
                to.write(text.toUtf8().constData());
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
            writeCSVLine(num, msg,to);
        else if(exportSelection == QDltExporter::SelectionFiltered)
            writeCSVLine(from->getMsgFilterPos(num), msg,to);
        else if(exportSelection == QDltExporter::SelectionSelected)
            writeCSVLine(from->getMsgFilterPos(selectedRows[num]), msg,to);
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
        QString payload = msg.toStringPayload().simplified().remove(QChar::Null);
        if(from) from->applyRegExString(msg,payload);
        text += "|" + QString("%1.%2").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0')) +
                "|" + msg.getEcuid() +
                "|" + msg.getApid() +
                "|" + msg.getCtid() +
                "|" + payload.replace('|', "\\|").replace('#', "\\#").replace('*', "\\*") +
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

void QDltExporter::setFilterList(QDltFilterList &filterList)
{
    this->filterList = filterList;
}

void QDltExporter::setMultifilterFilenames(QStringList multifilterFilenames)
{
    this->multifilterFilenames=multifilterFilenames;
}

void QDltExporter::exportMessages()
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
    clipboardString.clear();
    unsigned long int starting = 0;
    unsigned long int stoping = this->size;

    /* start export */
    if(false == startExport())
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
            emit progress("Exp:",2,percent); // every 1%
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
            //FIXME: The following does not work for non verbose messages, must be fixed
            if(pluginManager)
                pluginManager->decodeMsg(msg,silentMode);
            if (exportFormat == QDltExporter::FormatDltDecoded)
            {
                msg.setNumberOfArguments(msg.sizeArguments());
                msg.getMsg(buf,true);
            }
        }

        // apply Regex if needed
        if(exportFormat == QDltExporter::FormatDlt || exportFormat == QDltExporter::FormatDltDecoded)
        {
            //FIXME: The following does not work for non verbose messages, must be fixed to enable RegEx for DLT Export again
            //msg.setNumberOfArguments(msg.sizeArguments());
            bool isApplied = false;
            if(from) isApplied = from->applyRegExStringMsg(msg);
            if(isApplied) msg.getMsg(buf,true);
        }

        if(filterList.isEmpty() || filterList.checkFilter(msg))
        {
            // export message
            if(multifilterFilenames.isEmpty())
            {
                if(!exportMsg(starting,msg,buf,to))
                {
                    // finish();
                  //qDebug() << "DLT Export exportMsg() failed";
                  exportErrors++;
                  continue;
                }
                else
                   exportCounter++;
            }
            else
            {
                for(int num=0;num<multifilterFilterList.size();num++)
                {
                    if(multifilterFilterList[num]->checkFilter(msg))
                    {
                        if(!exportMsg(starting,msg,buf,*multifilterFilesList[num]))
                            exportErrors++;
                        else
                            exportCounter++;
                    }
                }
            }
        }

    } // for loop

    emit progress("",3,100);
    qDebug() << "Exported:" << 100 << "%";

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

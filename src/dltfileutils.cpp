#include "dltfileutils.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDir>

DltFileUtils::DltFileUtils()
{
}

QString DltFileUtils::createTempFile(QDir path)
{
    QString fn = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss")
            .append("_dlt-viewer-tmpfile.dlt");
    QFile file(path.absolutePath().append("/").append(fn));
    if(file.exists())
    {
        QMessageBox::critical(0, QString("DLT Viewer"),
                          QString("Temporary file already exists\n%1")
                          .arg(file.fileName()));
        return "";
    }
    if(!file.open(QIODevice::ReadWrite))
    {
        QMessageBox::critical(0, QString("DLT Viewer"),
                          QString("Can not open temporary file!\n%1")
                          .arg(file.fileName()));
        return "";
    }
    file.close();
    return file.fileName();
}

QDir DltFileUtils::getTempPath(SettingsDialog *settings)
{
    QDir temp_path;
    bool dir_ok = true;

    if(settings->tempUseSystem)
    {
        temp_path = QDir(QDir::tempPath());
        if(!temp_path.exists())
        {
            QMessageBox::critical(0, QString("DLT Viewer"),
                                  QString("System temporary directory does not exist!\n%1")
                                  .arg(temp_path.absolutePath()));
            dir_ok = false;
        }
    }
    else if(settings->tempUseOwn)
    {
        temp_path = QDir(settings->tempOwnPath);
        if(!temp_path.exists())
        {
            if(!temp_path.mkpath("."))
            {
                QMessageBox::critical(0, QString("DLT Viewer"),
                                  QString("Could not create temporary path!\n%1")
                                  .arg(temp_path.absolutePath()));
                dir_ok = false;
            }
        }

    }
    if(dir_ok)
    {
        return temp_path;
    }
    // Fallback, use current directory.
    return QDir("");
}

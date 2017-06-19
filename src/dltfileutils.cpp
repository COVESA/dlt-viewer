#include "dltfileutils.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

#if defined(_MSC_VER)
#include <process.h>
#else
#include <unistd.h>
#endif




DltFileUtils::DltFileUtils()
{
}

QString DltFileUtils::createTempFile(QDir path,  bool silentmode)
{
    #if defined(_MSC_VER)
    int a = _getpid();
    #else
    pid_t a = getpid();
    #endif

    QString fn = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss")
               .append("_")
               .append(QString::number(a))
               .append("_dlt-viewer-tmpfile.dlt");
    QFile file(path.absolutePath().append("/").append(fn));
    //qDebug() << "Create " << file.fileName() << a;
    if(file.exists())
    {
		if ( silentmode == true )
		{
	     qDebug() << "Temporary file " << file.fileName() << "already exists\n";
	    }
		else
		{
        QMessageBox::critical(0, QString("DLT Viewer"),
                          QString("Temporary file already exists\n%1")
                          .arg(file.fileName()));
        }
        return "";
    }
    if(!file.open(QIODevice::ReadWrite))
    {
	if ( silentmode == true )
		{
	     qDebug() << "Can not open temporary file " << file.fileName();
	    }
		else
		{
        QMessageBox::critical(0, QString("DLT Viewer"),
                          QString("Can not open temporary file!\n%1")
                          .arg(file.fileName()));
        }
        return "";
    }
    file.close();
    //qDebug() << "Create " << file.fileName() << "OK";
    return file.fileName();
}

QDir DltFileUtils::getTempPath(SettingsDialog *settings)
{
    QDir temp_path;
    bool dir_ok = true;

    if(settings->tempUseSystem)
    {
        QString temp_path_string = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        temp_path = QDir(temp_path_string);
        if (!temp_path.exists())
        {
            if(!temp_path.mkpath(temp_path_string))
            {
                QMessageBox::critical(0, QString("DLT Viewer"),
                                  QString("System temporary directory does not exist!\n%1")
                                  .arg(temp_path.absolutePath()));
                dir_ok = false;
            }
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

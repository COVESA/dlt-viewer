#include "dltfileutils.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QDebug>

#if defined(_MSC_VER)
#include <process.h>
#else
#include <unistd.h>
#endif

#include "qdltsettingsmanager.h"



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
                          QString("Temporary file %1\nalready exists !")
                          .arg(file.fileName()));
        }
        return file.fileName();
    }
    if( false == file.open(QIODevice::ReadWrite) )
    {
	if ( silentmode == true )
		{
         qDebug() << "Can not create temporary file " << file.fileName() << file.errorString();
	    }
		else
		{
        QMessageBox::critical(0, QString("DLT Viewer"),
                          QString("Can not create temporary file !\n%1\n%2")
                          .arg(file.fileName()).arg(file.errorString()));
        }
        return file.fileName();
    }
    file.close();
    //qDebug() << "Create " << file.fileName() << "OK";
    return file.fileName();
}

QDir DltFileUtils::getTempPath(bool silentmode)
{
    QDltSettingsManager *settings = QDltSettingsManager::getInstance();

    QDir temp_path = QString(".");
    bool dir_ok = true;
  //  bool silentmode = true;

    if(settings->tempUseSystem)
    {
        temp_path = QDir(settings->tempSystemPath);
        if (false == temp_path.exists())
        {  // the path does not yet exist
            qDebug() << "System temporary path does not exist, try to create it !";
            if( false == temp_path.mkpath(settings->tempSystemPath) )
            {
                if ( silentmode == true )
                            {
                             qDebug() << QString("Check write privilges for system temporary path %1 !!").arg(temp_path.absolutePath());
                             qDebug() << "System temporary path - not accessible - using path . instead !";
                            }
                           else
                            {
                              QMessageBox::critical(0, QString("DLT Viewer"),
                                                QString("System temporary path\n %1\n not accessible !\nUsing \".\" instead !")
                                                .arg(temp_path.absolutePath()));
                            }

                dir_ok = false;
            }
        }
    }
    else if(settings->tempUseOwn)
    {
        temp_path = QDir(settings->tempOwnPath);
        if(!temp_path.exists())
        {
            if(!temp_path.mkdir("."))
            {
              if ( silentmode == true )
               {
                qDebug() << QString("Could not create temporary path !\n%1\nUsing \".\" instead !").arg(temp_path.absolutePath());
               }
              else
               {
                QMessageBox::critical(0, QString("DLT Viewer"),
                                  QString("Could not create temporary path !\n%1\nUsing \".\" instead !")
                                  .arg(temp_path.absolutePath()));
               }
                dir_ok = false;
            }
        }

    }
    if( true == dir_ok )
    {
        return temp_path;
    }
    //
    // Fallback, use current directory.
    return QDir (".");
}

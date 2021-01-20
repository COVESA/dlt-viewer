#ifndef DLTRAWLOGGING_H
#define DLTRAWLOGGING_H

#include <qstring.h>
#include <qbytearray.h>
#include <qfile.h>
#include <qdatetime.h>

/* raw logging of data received from different ECUs */

class DltRawLogging
{
public:
    DltRawLogging();
    ~DltRawLogging();

    /* write data into raw file */
    /* open, close and rename file if needed */
    bool writeData(QString &ecuId, QByteArray &data);

    /* check if timeout reached, if no data is received and close and rename file */
    void checkEndTime();

    /* check if max file size is reached, close and rename file if reached */
    void checkFileSize();

    /* set path where raw files are stored */
    void setPath(QString &path) { this->path = path; }

    /* set project name */
    void setProjectName(QString &projectName) { this->projectName = projectName; }

    /* set timeout in seconds, when new file is generated */
    void setTimeout(unsigned int timeout) { this->timeout = timeout; }

    /* set maxFileSize in MBs, when new file is generated */
    void setMaxFileSize(unsigned int maxFileSize) { this->maxFileSize = maxFileSize * 1024ul * 1024ul; }

private:

    /* settings */
    QString path; // the path where the raw logging files are stored
    QString projectName; // project name added to the beginning of the filename
    unsigned int timeout; // timeout in seconds, when new file is generated; default 5s
    unsigned long maxFileSize; // maximum files size of file, if reached new file is created; default 100MB

    QFile outputFile; // current used output file

    QDateTime beginTime; // time when first data is written into the file
    QDateTime endTime; // time when last data is written into the file

};

#endif // DLTRAWLOGGING_H

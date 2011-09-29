#ifndef FILE_H
#define FILE_H


#include <QTreeWidgetItem>
#include <QFile>
#include <QDir>
#include "globals.h"

class File : public QTreeWidgetItem
{

public:
     File();
     File(QTreeWidgetItem *parent=0);
     ~File();

     QString getFilename();
     QString getFilenameOnTarget();
     QString getFileSerialNumber();
     QString getFileCreationDate();

     unsigned int getPackages();
     unsigned int getReceivedPackages();
     unsigned int getSizeInBytes();
     unsigned int getBufferSize();
     int getWrittenBytes();

     void setFilename(QString f);
     void setFileCreationDate(QString f);
     void setFileSerialNumber(QString s);
     void setPackages(QString p);
     void increaseReceivedPackages();
     void setSizeInBytes(QString s);
     void setBuffersize(QString b);
     void setComplete();

     void errorHappens(QString filename, QString errorCode1, QString errorCode2, QString time);

     bool isComplete();

     void appendData(QString packageNumber,int size, const QByteArray& ba);

     bool saveFile(QString newFile);

     QByteArray getData();

     QString saveAsTmpFile();
     bool removeTmpFile(QString tmp);
private:
    QString filename;
    QString fileCreationDate;
    unsigned int fileSerialNumber;
    unsigned int packages;
    unsigned int receivedPackages;
    unsigned int sizeInBytes;
    unsigned int buffer;


    QByteArray *data;
    QByteArray *fullData;
    int *dataSize;
    int writtenBytes;
};

#endif // FILE_H

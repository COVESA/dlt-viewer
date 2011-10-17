#ifndef FILE_H
#define FILE_H


#include <QTreeWidgetItem>
#include <QFile>
#include <QDir>
#include <QList>
#include "globals.h"
#include "qdlt.h"

class File : public QTreeWidgetItem
{

public:
     File();
     File(QDltFile *qfile,QTreeWidgetItem *parent=0);
     ~File();

     QString getFilename();
     QString getFilenameOnTarget();
     QString getFileSerialNumber();
     QString getFileCreationDate();

     unsigned int getPackages();
     unsigned int getReceivedPackages();
     unsigned int getSizeInBytes();
     unsigned int getBufferSize();

     void setFilename(QString f);
     void setFileCreationDate(QString f);
     void setFileSerialNumber(QString s);
     void setPackages(QString p);
     void increaseReceivedPackages();
     void setSizeInBytes(QString s);
     void setBuffersize(QString b);
     void setComplete();

     void freeFile();

     void errorHappens(QString filename, QString errorCode1, QString errorCode2, QString time);

     bool isComplete();
     void setQFileIndexForPackage(QString packageNumber, int index);

     bool saveFile(QString newFile);

     QByteArray* getFileData();

private:
    QString filenameWithPath;
    QString fileCreationDate;
    unsigned int fileSerialNumber;
    unsigned int packages;
    unsigned int receivedPackages;
    unsigned int sizeInBytes;
    unsigned int buffer;

    QList<int> *dltFileIndex;
    QDltFile *dltFile;
    QByteArray *fileData;
};

#endif // FILE_H

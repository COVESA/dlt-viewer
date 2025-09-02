/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 * Contributions are licensed to the COVESA Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file file.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

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

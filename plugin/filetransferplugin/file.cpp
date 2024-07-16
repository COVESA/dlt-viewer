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
 * \file file.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "file.h"
#include <iostream>
#include <fstream>
#include <QDebug>
using namespace std;


File::File():QTreeWidgetItem()
{
    fileSerialNumber = 0;
    packages = 0;
    receivedPackages = 0;
    sizeInBytes = 0;
    buffer = 0;
    dltFileIndex = NULL;
    dltFile = NULL;
    fileData = NULL;
}

File::File(QDltFile *qfile,QTreeWidgetItem *parent):QTreeWidgetItem(parent)
{
    receivedPackages = 0;
    fileSerialNumber = 0;
    packages = 0;
    sizeInBytes = 0;
    buffer = 0;
    dltFileIndex = NULL;
    dltFile = qfile;
    fileData = NULL;

    this->setText(COLUMN_STATUS, "Incomplete");
    this->setForeground(COLUMN_STATUS,Qt::white);
    this->setBackground(COLUMN_STATUS,Qt::red);
    this->setText(COLUMN_RECPACKAGES, "0");
}

File::~File()
{

}

QString File::getFilename(){
    QStringList pathList = filenameWithPath.split("/");
    return pathList.last();
}
QString File::getFileCreationDate(){
    return fileCreationDate;
}

QString File::getFilenameOnTarget(){
    return filenameWithPath;
}
QString File::getFileSerialNumber(){
    QString str;
    str.append(QString("%1").arg(fileSerialNumber));

    return str;
}
unsigned int File::getPackages(){
    return packages;
}
unsigned int File::getReceivedPackages(){
    return receivedPackages;
}
unsigned int File::getSizeInBytes(){
    return sizeInBytes;
}
unsigned int File::getBufferSize(){
    return buffer;
}


void File::setFilename(QString f){
    filenameWithPath = f;
    this->setText(COLUMN_FILENAME, filenameWithPath);
}

void File::setFileCreationDate(QString f){
    fileCreationDate = f.simplified().remove(QChar::Null);
    this->setText(COLUMN_FILEDATE, fileCreationDate);
}

void File::setFileSerialNumber(QString s)
{
    fileSerialNumber = s.toUInt();
    this->setText(COLUMN_FILEID, s);
}

void File::setPackages(QString p){
    packages = p.toUInt();
    dltFileIndex = new QList<int>[packages];
    this->setText(COLUMN_PACKAGES, p);
}
void File::increaseReceivedPackages(){
    receivedPackages++;

    QString str;
    str.append(QString("%1").arg(receivedPackages));
    this->setText(COLUMN_RECPACKAGES, str);
}

void File::setSizeInBytes(QString s){
    sizeInBytes = s.toUInt();
    this->setText(COLUMN_SIZE, s);
}

void File::setBuffersize(QString b){
    buffer = b.toUInt();
    this->setText(COLUMN_BUFFERSIZE, b);
}

void File::setComplete(){
    this->setText(COLUMN_STATUS, "Complete");
    this->setForeground(COLUMN_STATUS,Qt::black);
    this->setBackground(COLUMN_STATUS,Qt::green);
}

void File::errorHappens(QString filename, QString errorCode1, QString errorCode2, QString time){
    setFilename(filename);

    fileSerialNumber = 1;
    packages = 2;
    receivedPackages = 3;
    sizeInBytes = 4;
    buffer = 5;

    QString str = errorCode1+", "+errorCode2;
    this->setText(COLUMN_FILEID,str);

    this->setText(COLUMN_FILEDATE,time);
    this->setText(COLUMN_STATUS, "ERROR");
    this->setForeground(COLUMN_STATUS,Qt::white);
    this->setBackground(COLUMN_STATUS,Qt::red);

}

bool File::isComplete(){
    return receivedPackages == packages;
}

void File::setQFileIndexForPackage(QString packageNumber, int index){
    int i = packageNumber.toInt();
    if((i-1) <= dltFileIndex->length())
    {
        dltFileIndex->insert(i-1, index);
    }
    else
    {
        qDebug() << "ERROR in setQFileIndexForPackage: i" << i << "is greater than dltFileIndex length" << dltFileIndex->length() << "FileSerialNumber" << fileSerialNumber;
    }
    increaseReceivedPackages();
}


bool File::saveFile(QString newFile)
{

    if(QFile::exists(newFile)){
        if(!QFile::remove(newFile))
        {
            qDebug() << "File " << newFile << "already exists";
            // everything fine, file exists so that is what we want anyway
            return true;
        }
    }

    QByteArray *completeFileData = getFileData();

    QFile file(newFile);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "File " << newFile << "could not be opened" << __LINE__;
        freeFile();
        return false;
    }

    QDataStream stream( &file );
    int writtenBytes = stream.writeRawData(*completeFileData,completeFileData->size());

    file.close();

    freeFile();

    if((unsigned int)writtenBytes != sizeInBytes){
        return false;
    }

    return true;
}

void File::freeFile(){
    delete fileData;
}

QByteArray* File::getFileData(){
   QDltMsg msg;
   QByteArray msgBuffer;
   QDltArgument data;

   fileData = new QByteArray();

    for(unsigned int i=0; i<packages;i++){
       int qfileIdx = dltFileIndex->value(i);
       msgBuffer =  dltFile->getMsg(qfileIdx);
       msg.setMsg(msgBuffer);
       msg.setIndex(qfileIdx);
       msg.getArgument(PROTOCOL_FLDA_DATA,data);
       fileData->append(data.getData());
    }

    return fileData;
}

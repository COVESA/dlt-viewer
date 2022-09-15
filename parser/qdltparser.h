/**
 * @licence app begin@
 * Copyright (C) 2014  BMW AG
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
 * \author
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \file qdltparser.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef QDLTPARSER_H
#define QDLTPARSER_H

#include <QString>

#include <stdarg.h>
#include <stdint.h>

#include <QMap>
#include <QFile>

typedef int pid_t;
typedef unsigned int speed_t;

typedef float  float32_t;
typedef double float64_t;

class QDltCon
{
public:
       QDltCon() {  }

       QString conid;
       QString appid;
       QString description;
       QString context;
};

class QDltFibexPdu
{
public:
       QDltFibexPdu() { byteLength = 0; }

       QString id;
       QString description;
       int32_t byteLength;
       QString typeInfo;
};

class QDltFibexPduRef
{
public:
       QDltFibexPduRef() { ref = 0; }

       QString id;
       QDltFibexPdu *ref;
};

/**
* The structure of a Fibex Frame information.
*/
class QDltFibexFrame
{
public:
   QDltFibexFrame() { byteLength=0;messageType=0;pduRefCounter=0;id=0;lineNumber=0; }

       QString context;
       QString idString;
       uint32_t id;
       int32_t byteLength;
       uint8_t messageType;
       QString messageInfo;
       QString appid;
       QString ctid;
       QString filename;
       int lineNumber;

       QList<QDltFibexPdu*> pdureflist;
       QList<QDltFibexPduRef*> pdurefreflist;
       uint32_t pduRefCounter;
};

class QDltParser
{
public:
    QDltParser();

    void clear();

    bool parseFile(QString fileName);
    bool parseConfiguration(QString fileName);
    bool parseCheck();
    bool converteFile(QString fileName);

    QString getLastError() { return errorString; }

    QList<QDltFibexFrame*> getMessages() { return messages; }
    QMap<QString,QDltCon*> getContexts() { return contexts; }
    QMap<QString,QString> getApplications() { return applications; }
    QMap<QString,uint32_t> getMessageIds() { return messageIds; }

    bool checkDoubleIds(QString &text, bool perApplication);
    bool checkId(uint32_t startId, uint32_t endId, uint32_t &id);
    bool checkId(uint32_t startId,uint32_t endId,uint32_t &id, bool perApplication,QString appId);
    bool generateId(uint32_t startId, uint32_t endId, bool perApplication);

    bool writeFibex(QString &fileName);
    bool writeIdHeader(QString &directory, bool perApplication);
    bool writeCsv(QString &fileName);
    bool readFibex(QString &fileName);

private:

    bool parseContextsRegisterApp(QString text);
    bool parseContextsRegisterContext(QString text);
    bool parseMessageId(QString text);
    bool parseMessage(QFile &file, QFile &fileWrite, int &linecounter, QString text);
    QStringList parseMessageLine(QFile &file, QString &line, int &linecounter);
    void parseMessageStringList(QDltFibexFrame *frame,QFile &fileWrite,QStringList &list,bool withid);
    void parseMessageParameter(QString argtext,QDltFibexFrame *frame);

    QString errorString;
    QMap<QString,QString> applications;
    QMap<QString,QDltCon*> contexts;
    QMap<QString,uint32_t> messageIds;
    QList<QDltFibexFrame*> messages;

};

#endif // QDLTPARSER_H

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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> 2011-2012
 *
 * \file qdlt.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef QDLT_MSG_H
#define QDLT_MSG_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
//#include <QColor>
#include <QMutex>
#include <time.h>

#include "export_rules.h"

//! Access to a DLT message.
/*!
  This class provide access to a single DLT message from a DLT log file.
  This class is currently not thread safe.
*/
class QDLT_EXPORT QDltMsg : public QDlt
{
public:
    //! Constructor.
    /*!
      This call clears all variables of the argument.
    */
    QDltMsg();

    //! Destructor.
    /*!
    */
    ~QDltMsg();

    //! The type of the DLT message.
    typedef enum { DltTypeUnknown = -2, DltTypeLog = 0,DltTypeAppTrace,DltTypeNwTrace,DltTypeControl } DltTypeDef;

    //! The subtype of the DLT message, if type is DltTypeLog.
    typedef enum { DltLogUnknown = -2, DltLogDefault = -1, DltLogOff = 0, DltLogFatal, DltLogError, DltLogWarn, DltLogInfo, DltLogDebug, DltLogVerbose } DltLogDef;

    //! The subtype of the DLT message, if type is DltTypeAppTrace.
    typedef enum { DltTraceUnknown = -2, DltTraceVariable = 1, DltTraceFunctionIn, DltTraceFunctionOut, DltTraceState, DltTraceVfb } DltTraceDef;

    //! The subtype of the DLT message, if type is DltTypeNwTrace.
    typedef enum { DltNetworkTraceUnknown = -2, DltNetworkTraceIpc = 1, DltNetworkTraceCan, DltNetworkTraceFlexray, DltNetworkTraceMost } DltNetworkTraceDef;

    //! The subtype of the DLT message, if type is DltTypeControl.
    typedef enum { DltControlUnknwon = -2, DltControlRequest = 1, DltControlResponse, DltControlTime } DltControlDef;

    //! The verbose mode of the message.
    typedef enum { DltModeUnknown = -2, DltModeNonVerbose = 0, DltModeVerbose = 1 } DltModeDef;

    //! Get the time of the DLT message, when the DLT message is logged.
    /*!
      \return The time when the DLT message is logged.
    */
    time_t getTime() const { return time; }

    //! Set the time of the DLT message, when the DLT message is logged.
    /*!
      \param _time The time when the DLT message is logged.
    */
    void setTime(unsigned int _time) { time = _time; }

    //! Get the time of the message as a formatted string.
    /*!
      \return QString representing the time of the message.
    */
    QString getTimeString() const;

    //! Get the time of the message based on the gmtime with offset as string.
    /*!
      \param utcOffsetInSeconds Offset in seconds added to gmtime.
      \param dst Daylight saving time - if true, adding automatically 3600 seconds on top.
      \return QString representing the the time of the message for specific time zone.
    */
    QString getGmTimeWithOffsetString(qlonglong utcOffsetInSeconds, bool dst);

    //! Get the time, microseconds part, of the DLT message, when the DLT message is logged.
    /*!
      \return The microseconds when the DLT message is logged.
    */
    unsigned int getMicroseconds() const { return microseconds; }

    //! Set the time, microseconds part, of the DLT message, when the DLT message is logged.
    /*!
      \param _microseconds The microseconds when the DLT message is logged.
    */
    void setMicroseconds(unsigned int _microseconds) { microseconds = _microseconds; }

    //! Get the uptime of the DLT message, when the DLT message is generated.
    /*!
      \return The uptime when the DLT message is generated.
    */
    unsigned int getTimestamp() const { return timestamp; }

    //! Get the uptime of the DLT message, when the DLT message is generated.
    /*!
      \param _timestamp The uptime when the DLT message is generated.
    */
    void setTimestamp(unsigned int _timestamp) { timestamp = _timestamp; }

    //! Get the session id of the DLT message.
    /*!
      \return The session id of the DLT message.
    */
    unsigned int getSessionid() const { return sessionid; }

    //! Set the session id of the DLT message.
    /*!
      \param _sessionid The session id of the DLT message.
    */
    void setSessionid(unsigned int _sessionid) { sessionid = _sessionid; }

    //! Get the session name of the DLT message.
    /*!
      \return The session name of the DLT message.
    */
    QString getSessionName() const { return sessionName; }

    //! Set the session name of the DLT message.
    /*!
      \param sessionName The session name of the DLT message.
    */
    void setSessionName(QString sessionName) { this->sessionName = sessionName; }

    //! Get the message counter of the DLT message.
    /*!
      The message counter is increased by one for each message of a context.
      \return The message counter.
    */
    unsigned char getMessageCounter() const { return messageCounter; }

    //! Set the message counter of the DLT message.
    /*!
      The message counter is increased by one for each message of a context.
      \param _messageCounter The message counter.
    */
    void setMessageCounter(unsigned char _messageCounter) { messageCounter = _messageCounter; }

    //! Get the ecu id of the DLT message.
    /*!
      \return The ecu id of the DLT message.
    */
    QString getEcuid() const { return ecuid; }

    //! Set the ecu id of the DLT message.
    /*!
      \param _ecuid The ecu id of the DLT message.
    */
    void setEcuid(QString _ecuid) { ecuid = _ecuid; }

    //! Get the application id of the DLT message.
    /*!
      \return The application id.
    */
    QString getApid() const { return apid; }

    //! Set the application id of the DLT message.
    /*!
      \param id The application id.
    */
    void setApid(QString id) { apid = id; }

    //! Get the context id of the DLT message.
    /*!
      \return The contex id.
    */
    QString getCtid() const { return ctid; }

    //! Set the context id of the DLT message.
    /*!
      \param id The context id.
    */
    void setCtid(QString id) { ctid = id; }

    //! Get the type of the DLT message.
    /*!
      Depending on the type the subtype has different values.
      \sa DltTypeDef
      \return The type of the DLT message.
    */
    DltTypeDef getType() const { return type; }

    //! Set the type of the DLT message.
    /*!
      Depending on the type the subtype has different values.
      \sa DltTypeDef
      \param _type The type of the DLT message.
    */
    void setType(DltTypeDef _type) { type = _type; }

    //! Get the text of the type of the DLT message.
    /*!
      \return The text of the type.
    */
    QString getTypeString() const;

    //! Get the endianness of the DLT message.
    /*!
      \sa DltEndiannessDef
      \return The endianness of the DLT message.
    */
    DltEndiannessDef getEndianness() const { return endianness; }

    //! Set the endianness of the DLT message.
    /*!
      \sa DltEndiannessDef
      \param _endianness The endianness of the DLT message.
    */
    void setEndianness(DltEndiannessDef _endianness) { endianness = _endianness; }

    //! Get the text of the endianness of the DLT message.
    /*!
      \return The text of endianness of the DLT message.
    */
    QString getEndiannessString() const;

    //! Get the subtype of the DLT message.
    /*!
      The subtype depends on the type.
      \sa DltLogDef
      \sa DltTraceDef
      \sa DltNetworkTraceDef
      \sa DltControlDef
      \return The subtype of the DLT message.
    */
    int getSubtype() const { return subtype; }

    //! Set the subtype of the DLT message.
    /*!
      The subtype depends on the type.
      \param _subtype The subtype of the DLT message.
    */
    void setSubtype(unsigned char _subtype) { subtype = _subtype; }

    //! Get the text of the subtype.
    /*!
      \return The text of the subtype.
    */
    QString getSubtypeString() const;

    //! Get the mode (verbose or non-verbose) of the DLT message.
    /*!
      Non-verbose messages contains not all valid variables.
      DLT Ctrl messages are also in non-verbose mode.
      \return The mode of the DLT message.
    */
    DltModeDef getMode() const { return mode; }

    //! Set the mode (verbose or non-verbose) of the DLT message.
    /*!
      Non-verbose messages contains not all valid variables.
      DLT Ctrl messages are also in non-verbose mode.
      \param _mode The mode of the DLT message.
    */
    void setMode(DltModeDef _mode) { mode = _mode; }

    //! Get the text of the mode (verbose or non-verbose).
    /*!
      \return The text of the mode.
    */
    QString getModeString() const;

    //! Get the number of arguments of the DLT message.
    /*!
      This is the number of arguments from the DLT message header.
      This is not the number of arguments in the argument list.
      In verbose mode both values should be the same, if the extraction of the arguments from the payload was succesfull.
      In non-verbose mode this value can be invalid.
      \return The number of arguments.
    */
    unsigned char getNumberOfArguments() const { return numberOfArguments; }

    //! Set the number of arguments in the payload.
    /*!
      The number of arguments and the size of the argument list must not be the same.
      E.g. if a non-verbose message is decoded these two parameters are different.
      \param noargs The number of arguments in the payload.
    */
    void setNumberOfArguments(unsigned char noargs) { numberOfArguments = noargs; }

    //! Get the binary header of the DLT message.
    /*!
      \return Byte Array containing the complete header of the DLT message.
    */
    QByteArray getHeader() const { return header; }

    //! Set the binary header of the DLT message.
    /*!
      Be careful with this function, binary data and interpreted data will not be in sync anymore.
      \param data The new header of the DLT message
    */
    void setHeader(QByteArray &data) { header = data; }

    //! Get the size of the header.
    /*!
      This value is even set, if the data was to small, but the header was read.
      \return Byte Array containing the complete payload of the DLT message.
    */
    int getHeaderSize() const { return headerSize; }

    //! Get the binary payload of the DLT message.
    /*!
      \return Byte Array containing the complete payload of the DLT message.
    */
    QByteArray getPayload() const { return payload; }

    //! Set the binary payload of the DLT message.
    /*!
      Be careful with this function, binary data and interpreted data will not be in sync anymore.
      \param data The new payload of the DLT message
    */
    void setPayload(QByteArray &data) { payload = data; }

    //! Generate binary header and payload.
    /*!
      This function will generate first the binary payload from the argument list of the DLt message.
      In a second step it will generate the binary header from all information in the DLT message.
    */
    void genMsg();

    //! Get the size of the payload.
    /*!
      This value is even set, if the data was to small, but the header was read.
      \return Byte Array containing the complete payload of the DLT message.
    */
    int getPayloadSize() const { return payloadSize; }

    //! Get the the message id of non-verbose DLT message.
    /*!
      This value is only useful, if the DLT message is a non-verbose message and non DLT control message.
      \return The message id of non-verbose message.
    */
    unsigned int getMessageId() const { return messageId; }

    //! Get the the service id of ctrl DLT message.
    /*!
      This value is only useful, if the DLT message is a ctrl message.
      \return The service id of a ctrl message.
    */
    unsigned int getCtrlServiceId() const;

    //! Get the the service id string of ctrl DLT message.
    /*!
      This value is only useful, if the DLT message is a ctrl message.
      \return The service id string of a ctrl message.
    */
    QString getCtrlServiceIdString() const;

    //! Get the the ctrl return type of ctrl DLT response message.
    /*!
      This value is only useful, if the DLT message is a ctrl response message.
      \return The return type of a ctrl message.
    */
    unsigned char getCtrlReturnType() const;

    //! Get the the return type string of ctrl DLT response message.
    /*!
      This value is only useful, if the DLT message is a ctrl response message.
      \return The return type string of a ctrl message.
    */
    QString getCtrlReturnTypeString() const;

    //! Get argument size.
    /*!
      Get the number of arguments in the argument list.
      This could be a different size of number of arguments, especially in the non verbose mode.
      \return number of arguments in the argument list.
    */
    int sizeArguments() const;

    //! Clear the list of arguments.
    /*!
      Clear the list of arguments.
      The number of arguments value is not changed.
    */
    void clearArguments();

    //! Get one of the arguments from the DLT message.
    /*!
      This is only possible if DLT message is in verbose mode or the DLT message is converted into verbose mode.
      \param Index the number of the argument starting by zero.
      \return True if the operation was successful, false if there was an error.
    */
    bool getArgument(int index,QDltArgument &argument) const;

    //! Add an argument to the argument list.
    /*!
      \param argument the argument to be added.
      \param index position where argument is added, -1 if added to end of list.
    */
    void addArgument(QDltArgument argument, int index = -1);

    //! Remove an argument.
    /*!
      \param index position where argument is removed.
    */
    void removeArgument(int index);

    //! Set the message provided by a byte array containing the DLT message.
    /*!
      The message must start at the beginning of the byte array, but the byte array can be
      bigger than the message itself. On success the header and the payload is copied to the
      corresponding buffers. If it fails, but at least the header can be read, the payload
      size can be retrieved, which is perhaps wrong.
      This function returns false, if an error in the decoded message was found.
      \param buf the buffer containing the DLT messages.
      \param withSH message to be parsed contains storage header, default true.
      \return True if the operation was successful, false if there was an error.
    */
    bool setMsg(const QByteArray& buf,bool withStorageHeader = true);

    //! Get the message written into a byte array containing the DLT message.
    /*!
      This function returns false, if an error in the data was found.
      \param buf the buffer containing the DLT messages.
      \param withStorageHeader message contains storage header, default true.
      \return True if the operation was successful, false if there was an error.
    */
    bool getMsg(QByteArray &buf,bool withStorageHeader = true);

    //! Clears all variables of the message.
    void clear();

    //! Print Header into a string.
    /*!
      \return The header string.
    */
    QString toStringHeader() const;

    //! Print Payload content into a string.
    /*!
      \return The payload string.
    */
    QString toStringPayload() const;
protected:

private:

    //! The header parameter ECU Id.
    QString ecuid;

    //! The header parameter application Id.
    QString apid;

    //! The header parameter context Id.
    QString ctid;

    //! The header parameter type of the message.
    DltTypeDef type;

    //! The header parameter subtype of the message, depends on type.
    int subtype; /* depends on type */

    //! The verbose or non-verbose mode of the message.
    DltModeDef mode;

    //! The endianness of the payload of the message.
    DltEndiannessDef endianness;

    //! The time, seconds part, of the message generated by the logger.
    time_t time;

    //! The time, microseconds part, of the message generated by the logger.
    unsigned int microseconds;

    //! The timestamp generated by the ECU.
    unsigned int timestamp;

    //! The session id of the DLT message.
    unsigned int sessionid;

    //! The session name of the DLT message.
    QString sessionName;

    //! The message counter of a context.
    unsigned char messageCounter;

    //! The number of arguments of the DLT message.
    unsigned char numberOfArguments;

    //! The complete header of the DLT message.
    QByteArray header;
    int headerSize;

    //! The complete payload of the DLT message.
    QByteArray payload;
    int payloadSize;

    //! The message id if this is a non-verbose message and no control message.
    unsigned int messageId;

    //! The service id if the message is a ctrl message.
    unsigned int ctrlServiceId;

    //! The return type if the message is a ctrl response message.
    unsigned char ctrlReturnType;

    //! List of arguments of the DLT message.
    QList<QDltArgument> arguments;
};

#endif // QDLT_MSG_H

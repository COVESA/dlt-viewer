/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
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
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef QDLT_H
#define QDLT_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
#include <QColor>
#include <QMutex>
#include <time.h>

struct sDltFile;
struct sDltMessage;

class QextSerialPort;
class QTcpSocket;

//! Base class for all DLT classes.
/*!
  This class contains helper functions needed for all DLT operations and classes.
*/
class QDlt
{

public:    
    //! Constructor.
    /*!
    */
    QDlt();

    //! Destructor.
    /*!
    */
    ~QDlt();

    //! Byte swap some bytes.
    /*!
      \param bytes The data to be swapped
      \param size The number of bytes to be swapped, -1 if all bytes of teh byte array
      \param offset Offset in the byte array where to begin to byte swap
    */
     bool swap(QByteArray &bytes,int size = -1, int offset = 0);

    //! Convert byte array to text or HTML output.
    /*!
      \param bytes The data to be converted
      \param withLineNumber Line numbers are added to each line
      \param withBinary data is output in hex format
      \param withAscii data is output in ascii if possible
      \param blocksize Binary output in one line is grouped in blocks of bytes
      \param linesize Number of bytes written in one line
      \param toHtml true output is don in HTML, false output in text only
      \return The string with ascii or html output.
    */
    QString toAsciiTable(QByteArray &bytes, bool withLineNumber, bool withBinary, bool withAscii, int blocksize = 8, int linesize = 16, bool toHtml = true);

    //! Convert byte array to text output.
    /*!
      \param ascii true output in ascii, false output in hex
      \return The string with ascii output.
    */
    QString toAscii(QByteArray &bytes,bool ascii = false);

    //! The endianness of the message.
    typedef enum { DltEndiannessUnknown = -2, DltEndiannessLittleEndian = 0, DltEndiannessBigEndian = 1 } DltEndiannessDef;

protected:


private:


};

//! One argument of a DLT message.
/*!
  This class contains one argument of a DLT message.
  A QDltMessage contains several Arguments.
*/
class QDltArgument : public QDlt
{
public:

    //! Constructor.
    /*!
      This call clears all variables of the argument.
    */
    QDltArgument();

    //! Destructor.
    /*!
    */
    ~QDltArgument();

    //! The type definition of a DLT message argument.
    typedef enum { DltTypeInfoUnknown = -2, DltTypeInfoStrg = 0,DltTypeInfoBool,DltTypeInfoSInt,DltTypeInfoUInt, DltTypeInfoFloa, DltTypeInfoRawd, DltTypeInfoTrai } DltTypeInfoDef;

    //! Get the byte offset of the parameter starting in the payload.
    /*!
      \return The offset in bytes.
    */
    int getOffsetPayload();

    //! Set the byte offset of the parameter starting in the payload.
    /*!
      \param _offsetPayload The offset in bytes.
    */
    void setOffsetPayload(int _offsetPayload) { offsetPayload = _offsetPayload; }

    //! The size of the parameter data in bytes.
    /*!
      \return The size of the parameter in bytes.
    */
    int getDataSize();

    //! Get the type of the argument.
    /*!
      \sa DltTypeInfoDef
      \return The type of the argument.
    */
    DltTypeInfoDef getTypeInfo();

    //! Set the type of the argument.
    /*!
      \sa DltTypeInfoDef
      \param _typeInfo The type of the argument.
    */
    void setTypeInfo(DltTypeInfoDef _typeInfo) { typeInfo = _typeInfo; }

    //! The type of the argument as text output.
    /*!
      \return The text of the type.
    */
    QString getTypeInfoString();

    //! Get the endianness of the Argument.
    /*!
      \sa DltEndiannessDef
      \return The endianness of the DLT message.
    */
    DltEndiannessDef getEndianness() { return endianness; }

    //! Set the endianess of the Argument.
    /*!
      \sa DltEndiannessDef
      \param _endianess The endianess of the argument.
    */
    void setEndianness(DltEndiannessDef _endianess) { endianness = (QDlt::DltEndiannessDef)_endianess; }

    //! Get the byte data of the parameter.    
    /*!
      The endianess of the parameter is as set in teh endianess parameter.
      \return The complete data of the parameter as byte array.
    */
    QByteArray getData();

    //! Set the data of the parameter.
    /*!
      \param _data The new data of the parameter.
    */
    void setData(QByteArray _data) { data = _data; }

    //! Get the name of the DLT parameter.
    /*!
      The name of the DLT parameter is optional.
      This value is empty if it is not used.
      \return The name of the parameter.
    */
    QString getName();

    //! Set the name of the DLT parameter.
    /*!
      The name of the DLT parameter is optional.
      This value is empty if it is not used.
      \param _name The name of the parameter.
    */
    void setName(QString _name) { name = _name; }

    //! Get the name of the unit of the DLT parameter.
    /*!
      The unit name of the parameter is optional.
      The unit name is only used in integer and float arguments.
      \return The name of the unit of the variable.
    */
    QString getUnit();

    //! Set the name of the unit of the DLT parameter.
    /*!
      The unit name of the parameter is optional.
      The unit name is only used in integer and float arguments.
      \return The name of the unit of the variable.
    */
    void setUnit(QString _unit) { unit = _unit; }

    //! Parse the payload of DLT message and extract argument.
    /*!
      Parses an argument of the DLT message payload and stores all parsed values in the corresponding variables.
      All variables are cleared before starting parsing.
      \sa clear()
      \param payload This can be a filename or a directory name.
      \param offset Offset where to start parsing in the payload.
      \param _endianess The new endianess of the argument
      \return The name of the unit of the variable.
    */
    bool setArgument(QByteArray &payload,unsigned int &offset,DltEndiannessDef _endianess);

    //! Get argument as byte array and appends it to data.
    /*!
      \param data byte array to be appended
      \param verboseMode data should be returned in verboseMode
      \return true if operation was succesful, false if there was an error.
    */
    bool getArgument(QByteArray &data, bool verboseMode = true);

    //! Print argument content into a string.
    /*!
      \param binary if true write parameter as  Hex, if false translate into text
      \return The payload string.
    */
    QString toString(bool binary = false);

    //! Clears all variables of the class.
    void clear();

    QVariant getValue();

    bool setValue(QVariant value, bool verboseMode = true);

protected:

private:

    //! The endianness of the argument.
    DltEndiannessDef endianness;

    //! This type of the argument.
    DltTypeInfoDef typeInfo;

    //! The offset of the argument in the payload.
    int offsetPayload;

    //! This data of the argument.
    QByteArray data;

    //! This name of the argument.
    /*!
      This is an optional parameter.
    */
    QString name;

    //! The unit name of the argument.
    /*!
      This is an optional parameter.
    */
    QString unit;

};

//! Access to a DLT message.
/*!
  This class provide access to a single DLT message from a DLT log file.
  This class is currently not multithread save.
*/
class QDltMsg : public QDlt
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
    unsigned int getTime() { return time; }

    //! Set the time of the DLT message, when the DLT message is logged.
    /*!
      \param _time The time when the DLT message is logged.
    */
    void setTime(unsigned int _time) { time = _time; }

    //! Get the time of the message as a formatted string.
    /*!
      \return QString representing the time of the message.
    */
    QString getTimeString();

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
    unsigned int getMicroseconds() { return microseconds; }

    //! Set the time, microseconds part, of the DLT message, when the DLT message is logged.
    /*!
      \param _microseconds The microseconds when the DLT message is logged.
    */
    void setMicroseconds(unsigned int _microseconds) { microseconds = _microseconds; }

    //! Get the uptime of the DLT message, when the DLT message is generated.
    /*!
      \return The uptime when the DLT message is generated.
    */
    unsigned int getTimestamp() { return timestamp; }

    //! Get the uptime of the DLT message, when the DLT message is generated.
    /*!
      \param _timestamp The uptime when the DLT message is generated.
    */
    void setTimestamp(unsigned int _timestamp) { timestamp = _timestamp; }

    //! Get the session id of the DLT message.
    /*!
      \return The session id of the DLT message.
    */
    unsigned int getSessionid() { return sessionid; }

    //! Set the session id of the DLT message.
    /*!
      \param _sessionid The session id of the DLT message.
    */
    void setSessionid(unsigned int _sessionid) { sessionid = _sessionid; }

    //! Get the message counter of the DLT message.
    /*!
      The message counter is increased by one for each message of a context.
      \return The message counter.
    */
    unsigned char getMessageCounter() { return messageCounter; }

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
    QString getEcuid() { return ecuid; }

    //! Set the ecu id of the DLT message.
    /*!
      \param _ecuid The ecu id of the DLT message.
    */
    void setEcuid(QString _ecuid) { ecuid = _ecuid; }

    //! Get the application id of the DLT message.
    /*!
      \return The application id.
    */
    QString getApid() { return apid; }

    //! Set the application id of the DLT message.
    /*!
      \param id The application id.
    */
    void setApid(QString id) { apid = id; }

    //! Get the context id of the DLT message.
    /*!
      \return The contex id.
    */
    QString getCtid() { return ctid; }

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
    DltTypeDef getType() { return type; }

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
    QString getTypeString();

    //! Get the endianness of the DLT message.
    /*!
      \sa DltEndiannessDef
      \return The endianness of the DLT message.
    */
    DltEndiannessDef getEndianness() { return endianness; }

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
    QString getEndiannessString();

    //! Get the subtype of the DLT message.
    /*!
      The subtype depends on the type.
      \sa DltLogDef
      \sa DltTraceDef
      \sa DltNetworkTraceDef
      \sa DltControlDef
      \return The subtype of the DLT message.
    */
    int getSubtype() { return subtype; }

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
    QString getSubtypeString();

    //! Get the mode (verbose or non-verbose) of the DLT message.
    /*!
      Non-verbose messages contains not all valid variables.
      DLT Ctrl messages are also in non-verbose mode.
      \return The mode of the DLT message.
    */
    DltModeDef getMode() { return mode; }

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
    QString getModeString();

    //! Get the number of arguments of the DLT message.
    /*!
      This is the number of arguments from the DLT message header.
      This is not the number of arguments in the argument list.
      In verbose mode both values should be the same, if the extraction of the arguments from the payload was succesfull.
      In non-verbose mode this value can be invalid.
      \return The number of arguments.
    */
    unsigned char getNumberOfArguments() { return numberOfArguments; }

    //! Set the number of arguments in the payload.
    /*!
      The number of arguments and the size of the argumnet list must not be the same.
      E.g. if a non-verbose message is decoded these two parameters are different.
      \param noargs The number of arguments in the payload.
    */
    void setNumberOfArguments(unsigned char noargs) { numberOfArguments = noargs; }

    //! Get the complete payload of the DLT message.
    /*!
      \return Byte Array containig the complete payload of the DLT message.
    */
    QByteArray getPayload() { return payload; }

    //! Get the the message id of non-verbose DLT message.
    /*!
      This value is only useful, if the DLT message is a non-verbose message and non DLT control message.
      \return The message id of non-verbose message.
    */
    unsigned int getMessageId() { return messageId; }

    //! Get the the service id of ctrl DLT message.
    /*!
      This value is only useful, if the DLT message is a ctrl message.
      \return The service id of a ctrl message.
    */
    unsigned int getCtrlServiceId();

    //! Get the the service id string of ctrl DLT message.
    /*!
      This value is only useful, if the DLT message is a ctrl message.
      \return The service id string of a ctrl message.
    */
    QString getCtrlServiceIdString();

    //! Get the the ctrl return type of ctrl DLT response message.
    /*!
      This value is only useful, if the DLT message is a ctrl response message.
      \return The return type of a ctrl message.
    */
    unsigned char getCtrlReturnType();

    //! Get the the return type string of ctrl DLT response message.
    /*!
      This value is only useful, if the DLT message is a ctrl response message.
      \return The return type string of a ctrl message.
    */
    QString getCtrlReturnTypeString();

    //! Get argument size.
    /*!
      Get the number of arguments in the argument list.
      This could be a different size of number of arguments, especially in the non verbose mode.
      \return number of arguments in teh argument list.
    */
    int sizeArguments();

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
      \return True if the operation was succesfull, false if there was an error.
    */
    bool getArgument(int index,QDltArgument &argument);

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
      This function returns false, if an error in the decoded message was found.
      \param buf the buffer containing the DLT messages.
      \param withSH message to be parsed contains storage header, default true.
      \return True if the operation was succesfull, false if there was an error.
    */
    bool setMsg(QByteArray buf,bool withStorageHeader = true);

    //! Get the message written into a byte array containing the DLT message.
    /*!
      This function returns false, if an error in the data was found.
      \param buf the buffer containing the DLT messages.
      \param withStorageHeader message contains storage header, default true.
      \return True if the operation was succesfull, false if there was an error.
    */
    bool getMsg(QByteArray &buf,bool withStorageHeader = true);

    //! Clears all variables of the message.
    void clear();

    //! Print Header into a string.
    /*!
      \return The header string.
    */
    QString toStringHeader();

    //! Print Payload content into a string.
    /*!
      \return The payload string.
    */
    QString toStringPayload();


protected:

private:

    //! The header paraemter ECU Id.
    QString ecuid;

    //! The header paraemter application Id.
    QString apid;

    //! The header paraemter context Id.
    QString ctid;

    //! The header parameter type of the message.
    DltTypeDef type;

    //! The header parameter subtype of the messsage, depends on type.
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

    //! The message counter of a context.
    unsigned char messageCounter;

    //! The number of arguments of the DLT message.
    unsigned char numberOfArguments;

    //! The complete payload of the DLT message.
    QByteArray payload;

    //! The message id if this is a non-verbose message and no control message.
    unsigned int messageId;

    //! The service id if the message is a ctrl message.
    unsigned int ctrlServiceId;

    //! The return type if the message is a ctrl response message.
    unsigned char ctrlReturnType;

    //! List of arguments of the DLT message.
    QList<QDltArgument> arguments;
};

class QDltFilter
{
public:
    QString ecuid;
    QString apid;
    QString ctid;
    QString header;
    QString payload;
    bool enableFilter;
    bool enableEcuid;
    bool enableApid;
    bool enableCtid;
    bool enableHeader;
    bool enablePayload;
    bool enableCtrlMsgs;
    bool enableLogLevelMax;
    bool enableLogLevelMin;
    QColor filterColour;
    int logLevelMax;
    int logLevelMin;
protected:
private:
};

//! Access to a DLT log file.
/*!
  This class provide access to DLT log file.
  This class is currently not multithread save.
*/
class QDltFile : public QDlt
{
public:
    //! The constructor.
    /*!
    */
    QDltFile();

    //! The destructor.
    /*!
      \return The name of the plugin
    */
    ~QDltFile();

    //! Get the number of DLT message in the DLT log file.
    /*!
      \return the number of all DLT messages in the currently opened DLT file.
    */
    int size();

    //! Get the number of filtered DLT message in the DLT log file.
    /*!
      \return the number of filtered DLT messages in the currently opened DLT file.
    */
    int sizeFilter();

    //! Open a DLT log file.
    /*!
      The DLT log file is parsed and a index of all DLT log messages is created.
      \param filename The DLT filename.
      \return true if the file is successfully opened with no error, false if an error occured.
    */
    bool open(QString _filename);

    //! Close teh currently opened DLT log file.
    /*!
    */
    void close();

    //! Sets the internal index of all DLT messages.
    /*!
      \param New index list of all DLT messages
    */
    void setDltIndex(QList<unsigned long> &_indexAll);

    //! Clears the internal index of all DLT messages.
    /*!
    */
    void clearIndex();

    //! Create an internal index of all DLT messages of the currently opened DLT log file.
    /*!
      \return true if the operation was successful, false if an error occured.
    */
    bool createIndex();

    //! Update the index of the currently opened DLT log file by checking if new DLT messages were added to the file.
    /*!
      \return true if the operation was successful, false if an error occured.
    */
    bool updateIndex();

    //! Create an internal index of all filtered DLT messages of the currently opened DLT log file.
    /*!
      \return true if the operation was successful, false if an error occured.
    */
    bool createIndexFilter();

    //! Update the filtered index of the currently opened DLT log file by checking if new DLT messages were added to the file.
    /*!
      \return true if the operation was successful, false if an error occured.
    */
    bool updateIndexFilter();

    //! Get one message of the DLT log file.
    /*!
      This function retrieves on DLT message of the log file
      \param index The number of the DLT message in the DLT file starting from zero.
      \param msg The message which contains the DLT message after the function returns.
      \return true if the message is valid, false if an error occured.
    */
    bool getMsg(int index,QDltMsg &msg);

    //! Get one DLT message of the DLT log file selected by index
    /*!
      \param index position of the DLT message in the log file up to the number DLT messages in the file
      \return Byte array containing the complete DLT message.
    */
    QByteArray getMsg(int index);

    //! Get one DLT message of the filtered DLT log file selected by index
    /*!
      \param index position of the DLT message in the log file up to the number of DLT messages in the file
      \return Byte array containing the complete DLT message.
    */
    QByteArray getMsgFilter(int index);

    //! Get the poition in the log file of the filtered DLT log file selected by index
    /*!
      \param index position of the DLT message in the log file up to the number of DLT messages in the file
      \return real position in log file, -1 if invalid.
    */
    int getMsgFilterPos(int index);

    //! Delete all filters and markers.
    /*!
      This includes all positive and negative filters and markers.
    */
    void clearFilter();

    //! Add a positive filter to the positive filter list.
    /*!
      \param filter the filter configuration
    */
    void addPFilter(QDltFilter &filter);

    //! Add a negative filter to the negative filter list.
    /*!
      \param filter the filter configuration
    */
    void addNFilter(QDltFilter &filter);

    //! Add a marker to the marker list.
    /*!
      \param filter the filter configuration
    */
    void addMarker(QDltFilter &filter);

    //! Get the status of the filter.
    /*!
      \return true if filtering is enabled, fals if filtering is disabled
    */
    bool isFilter();

    //! Enable or disable filtering.
    /*!
      \return state true if filtering is enabled, false if filtering is disabled
    */
    void enableFilter(bool state);

    //! Check if message matches the filter.
    /*!
      \param msg The message to be checked
      \return true if message wil be displayed, false if message will be filtered out
    */
    bool checkFilter(QDltMsg &msg);

    //! Clear the filter index.
    /*!
    */
    void clearFilterIndex();

    //! Add filter to the filter index.
    /*!
      \param index The position of the message in the allIndex to be added
    */
    void addFilterIndex (int index);

    //! Check if message will be marked.
    /*!
      Colours used are:
      1 = green
      2 = red
      3 = yellow
      4 = blue
      5 = light grey
      6 = dark grey
      \param msg The messages to be marked
      \return 0 if message will not be marked, colour if message will be marked
    */
    QColor checkMarker(QDltMsg &msg);

protected:

private:

    //! Mutex to lock critical path for infile
    QMutex mutexQDlt;

    //! DLT log file.
    QFile infile;

    //! Index of all DLT messages.
    /*!
      Index contains positions of beginning of DLT messages in DLT log file.
    */
    QList<unsigned long> indexAll;

    //! Index of all DLT messages matching filter.
    /*!
      Index contains positions of DLT messages in indexAll.
    */
    QList<unsigned long> indexFilter;

    //! List of positive filters.
    QList<QDltFilter> pfilter;

    //! List of negative filters.
    QList<QDltFilter> nfilter;

    //! List of markers.
    QList<QDltFilter> marker;

    //! Enabling filter.
    /*!
      true filtering is enabled.
      false filtering is disabled.
    */
    bool filterFlag;
};

class QDltControl : public QObject
{
      Q_OBJECT
public:
    QDltControl(QObject *server);
    ~QDltControl();

    void sendInjection(int index,QString applicationId,QString contextId,int serviceId,QByteArray data);

signals:
    void sendInjectionSignal(int index,QString applicationId,QString contextId,int serviceId,QByteArray data);

protected:
private:
    QObject *server;

};

class QDltConnection
{

public:
    //! The possible DLT connection states of an ECU
    enum QDltConnectionState{QDltConnectionOffline=0, QDltConnectionConnecting, QDltConnectionOnline, QDltConnectionError};

    QDltConnection();
    ~QDltConnection();

    void setSendSerialHeader(bool _sendSerialHeader);
    bool getSendSerialHeader();

    void setSyncSerialHeader(bool _syncSerialHeader);
    bool getSyncSerialHeader();

protected:

    bool sendSerialHeader;
    bool syncSerialHeader;

};

class QDltTCPConnection : public QDltConnection
{
public:

    QDltTCPConnection();
    ~QDltTCPConnection();

    void setHostname(QString _hostname);
    QString getHostname();

    void setTcpPort(unsigned int _tcpport);
    void setDefaultTcpPort();
    unsigned int getTcpPort();

private:

    QString hostname;
    unsigned int tcpport;

    QTcpSocket *socket;

};

class QDltSerialConnection : public QDltConnection
{
public:

    QDltSerialConnection();
    ~QDltSerialConnection();

    void setPort(QString _port);
    QString getPort();

    void setBaudrate(int _baudrate);
    unsigned int getBaudrate();

private:

    QString port;
    int baudrate;

    QextSerialPort *serialport;

};

#endif // QDLT_H

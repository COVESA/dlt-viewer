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

#ifndef QDLT_ARGUMENT_H
#define QDLT_ARGUMENT_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
//#include <QColor>
#include <QMutex>
#include <QVariant>
#include <time.h>

#include "export_rules.h"

//! One argument of a DLT message.
/*!
  This class contains one argument of a DLT message.
  A QDltMessage contains several Arguments.
*/
class QDLT_EXPORT QDltArgument : public QDlt
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
    typedef enum { DltTypeInfoUnknown = -2, DltTypeInfoStrg = 0,DltTypeInfoBool,DltTypeInfoSInt,DltTypeInfoUInt, DltTypeInfoFloa, DltTypeInfoRawd, DltTypeInfoTrai, DltTypeInfoUtf8  } DltTypeInfoDef;

    //! Get the byte offset of the parameter starting in the payload.
    /*!
      \return The offset in bytes.
    */
    int getOffsetPayload() const;

    //! Set the byte offset of the parameter starting in the payload.
    /*!
      \param _offsetPayload The offset in bytes.
    */
    void setOffsetPayload(int _offsetPayload) { offsetPayload = _offsetPayload; }

    //! The size of the parameter data in bytes.
    /*!
      \return The size of the parameter in bytes.
    */
    int getDataSize() const;

    //! Get the type of the argument.
    /*!
      \sa DltTypeInfoDef
      \return The type of the argument.
    */
    DltTypeInfoDef getTypeInfo() const;

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
    QString getTypeInfoString() const;

    //! Get the endianness of the Argument.
    /*!
      \sa DltEndiannessDef
      \return The endianness of the DLT message.
    */
    DltEndiannessDef getEndianness() const { return endianness; }

    //! Set the endianness of the Argument.
    /*!
      \sa DltEndiannessDef
      \param _endianess The endianness of the argument.
    */
    void setEndianness(DltEndiannessDef _endianess) { endianness = (QDlt::DltEndiannessDef)_endianess; }

    //! Get the byte data of the parameter.
    /*!
      The endianness of the parameter is as set in the endianness parameter.
      \return The complete data of the parameter as byte array.
    */
    QByteArray getData() const;

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
    QString getName() const;

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
    QString getUnit() const;

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
      \param _endianess The new endianness of the argument
      \return The name of the unit of the variable.
    */
    bool setArgument(QByteArray &payload,unsigned int &offset,DltEndiannessDef _endianess);

    //! Get argument as byte array and appends it to data.
    /*!
      \param data byte array to be appended
      \param verboseMode data should be returned in verboseMode
      \return true if operation was successful, false if there was an error.
    */
    bool getArgument(QByteArray &data, bool verboseMode = true) const;

    //! Print argument content into a string.
    /*!
      \param binary if true write parameter as  Hex, if false translate into text
      \return The payload string.
    */
    QString toString(bool binary = false) const;

    //! Clears all variables of the class.
    void clear();

    QVariant getValue() const;

    bool setValue(QVariant value, bool verboseMode = true);

protected:

private:

    //! The endianness of the argument.
    DltEndiannessDef endianness;

    //! This type of the argument linked in the DLT message header
    unsigned int dltType;

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


#endif // QDLT_ARGUMENT_H

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

#ifndef QDLT_FILTER_H
#define QDLT_FILTER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
//#include <QColor>
#include <QMutex>
#include <time.h>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QRegularExpression>
#include "export_rules.h"


class QDLT_EXPORT QDltFilter
{
public:

    typedef enum { positive = 0, negative, marker } FilterType;

    FilterType type;
    QString name;

    QString ecuid;
    QString apid;
    QString ctid;
    QString header;
    QString payload;
    QString regex_search;
    QString regex_replace;

    bool enableRegexp_Appid;
    bool enableRegexp_Context;
    bool enableRegexp_Header;
    bool enableRegexp_Payload;

    bool ignoreCase_Header;
    bool ignoreCase_Payload;

    bool enableFilter;
    bool enableEcuid;
    bool enableApid;
    bool enableCtid;
    bool enableHeader;
    bool enablePayload;
    bool enableCtrlMsgs;
    bool enableLogLevelMax;
    bool enableLogLevelMin;
    bool enableMarker;
    bool enableMessageId;
    bool enableRegexSearchReplace;

    QString filterColour;
    int logLevelMax;
    int logLevelMin;
    unsigned int messageIdMax;
    unsigned int messageIdMin;

    // generated from header and payload string
    QRegularExpression headerRegularExpression;
    QRegularExpression payloadRegularExpression;
    QRegularExpression contextRegularExpression;
    QRegularExpression appidRegularExpression;

    //! Constructor.
    /*!
    */
    QDltFilter();

    //! Destructor.
    /*!
    */
    ~QDltFilter();

    //! Reset all values to default values
    /*!
    */
    void clear();

    //! Check if this is a marker.
    /*!
    */
    bool isMarker() const;

    //! Check if this is a positive filter.
    /*!
    */
    bool isPositive() const;

    //! Check if this is a negative filter.
    /*!
    */
    bool isNegative() const;

    //! Copy operator.
    /*!
    */
    QDltFilter& operator= (QDltFilter const& _filter);

    //! Create regular expressions.
    /*!
    */
    bool compileRegexps();

    //! Check if filter matches.
    /*!
      \return true if filter matches the message, else false
    */
    bool match(QDltMsg &msg) const;

    //! Save filter parameters in XML file.
    /*!
    */
    void SaveFilterItem(QXmlStreamWriter &xml);

    //! Load filter parameters from XML file.
    /*!
    */
    void LoadFilterItem(QXmlStreamReader &xml);

protected:
private:
};

#endif // QDLT_FILTER_H

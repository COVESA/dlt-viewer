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
#include <QColor>
#include <QMutex>
#include <time.h>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

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

    bool enableRegexp;
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

    // generated from header and payload string
    QRegExp headerRegexp;
    QRegExp payloadRegexp;

    //! Constructor.
    /*!
    */
    QDltFilter();

    //! Destructor.
    /*!
    */
    ~QDltFilter();

    void clear();

    //! Copy operator.
    /*!
    */
    QDltFilter& operator= (QDltFilter const& _filter);

    //! Create regular expressions.
    /*!
    */
    bool compileRegexps();

    //! Ccheck if filter matches.
    /*!
      \return true if filter matches the msg, else false
    */
    bool match(QDltMsg &msg);

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

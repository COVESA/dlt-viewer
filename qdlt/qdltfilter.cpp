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
 * \file qdlt.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtDebug>

#include "qdlt.h"

extern "C"
{
#include "dlt_common.h"
}

QDltFilter::QDltFilter()
{
    clear();
}

QDltFilter::~QDltFilter()
{

}

QDltFilter& QDltFilter::operator= (QDltFilter const& _filter)
{
    type = _filter.type;
    name = _filter.name;

    ecuid = _filter.ecuid;
    apid = _filter.apid;
    ctid = _filter.ctid;
    header = _filter.header;
    payload = _filter.payload;
    regex_search = _filter.regex_search;
    regex_replace = _filter.regex_replace;

    enableRegexp_Appid       = _filter.enableRegexp_Appid;
    enableRegexp_Context     = _filter.enableRegexp_Context;
    enableRegexp_Header      = _filter.enableRegexp_Header;
    enableRegexp_Payload     = _filter.enableRegexp_Payload;
    ignoreCase_Header        = _filter.ignoreCase_Header;
    ignoreCase_Payload       = _filter.ignoreCase_Payload;
    enableRegexSearchReplace = _filter.enableRegexSearchReplace;

    enableFilter = _filter.enableFilter;
    enableEcuid = _filter.enableEcuid;
    enableApid = _filter.enableApid;
    enableCtid = _filter.enableCtid;
    enableHeader = _filter.enableHeader;
    enablePayload = _filter.enablePayload;
    enableCtrlMsgs = _filter.enableCtrlMsgs;
    enableLogLevelMax = _filter.enableLogLevelMax;
    enableLogLevelMin = _filter.enableLogLevelMin;
    enableMarker = _filter.enableMarker;
    enableMessageId=_filter.enableMessageId;

    messageIdMax=_filter.messageIdMax;
    messageIdMin=_filter.messageIdMin;
    filterColour = _filter.filterColour;
    logLevelMax = _filter.logLevelMax;
    logLevelMin = _filter.logLevelMin;

    // generated from header and payload string
    headerRegularExpression  = _filter.headerRegularExpression;
    payloadRegularExpression = _filter.payloadRegularExpression;
    contextRegularExpression = _filter.contextRegularExpression;
    appidRegularExpression   = _filter.appidRegularExpression;

    return *this;
}

void QDltFilter::clear()
{
    type = QDltFilter::positive;
    name = "New Filter";

    ecuid.clear();
    apid.clear();
    ctid.clear();
    header.clear();
    payload.clear();
    regex_search.clear();
    regex_replace.clear();

    enableRegexp_Appid = false;
    enableRegexp_Context = false;
    enableRegexp_Header = false;
    enableRegexp_Payload = false;
    ignoreCase_Header  = false;
    ignoreCase_Payload = false;
    enableFilter = false;
    enableEcuid = false;
    enableApid = false;
    enableCtid = false;
    enableHeader = false;
    enablePayload = false;
    enableCtrlMsgs = false;
    enableLogLevelMax = false;
    enableLogLevelMin = false;
    enableMarker = false;
    enableRegexSearchReplace = false;

    filterColour = "#000000"; // QColor() default contructor initializes to an invalid color RGB 0,0,0
    logLevelMax = 6;
    logLevelMin = 0;
    messageIdMax=0;
    messageIdMin=0;
}

bool QDltFilter::isMarker() const
{
    return ( type == QDltFilter::marker || enableMarker );
}

bool QDltFilter::isPositive() const
{
    return ( type == QDltFilter::positive );
}

bool QDltFilter::isNegative() const
{
    return ( type == QDltFilter::negative );
}

bool QDltFilter::compileRegexps()
{

    headerRegularExpression.setPattern(header);
    payloadRegularExpression.setPattern(payload);
    contextRegularExpression.setPattern(ctid);
    appidRegularExpression.setPattern(apid);

    headerRegularExpression.setPatternOptions(
        ignoreCase_Header ? QRegularExpression::CaseInsensitiveOption
                          : QRegularExpression::NoPatternOption);
    payloadRegularExpression.setPatternOptions(
        ignoreCase_Payload ? QRegularExpression::CaseInsensitiveOption
                           : QRegularExpression::NoPatternOption);

    return (headerRegularExpression.isValid() &&
            payloadRegularExpression.isValid() &&
            contextRegularExpression.isValid() &&
            appidRegularExpression.isValid());
}

bool QDltFilter::match(QDltMsg &msg) const
{

    if( (true == enableEcuid) && (msg.getEcuid() != ecuid))
    {
        return false;
    }

    if( true == enableRegexp_Appid )
    {
        if( (true == enableApid) && ( false == appidRegularExpression.match(msg.getApid()).hasMatch() ) )
        {
            return false;
        }

    }
    else
    {
        if( (true == enableApid) && (msg.getApid() != apid))
        {
            return false;
        }
    }

    if(true == enableRegexp_Context)
    {
        if( (true == enableCtid) && ( false == contextRegularExpression.match(msg.getCtid()).hasMatch() ) )
        {
            return false;
        }
    }
    else
    {
        if( (true ==enableCtid) && ( false == msg.getCtid().contains(ctid) ) )
        {
            return false;
        }
    }

    if(true == enableRegexp_Header)
    {
        if( (true == enableHeader) && ( false == headerRegularExpression.match(msg.toStringHeader()).hasMatch() ) )
        {
            return false;
        }
    }
    else
    {
        if( ( true == enableHeader ) && ( false == msg.toStringHeader().contains(header,ignoreCase_Header?Qt::CaseInsensitive:Qt::CaseSensitive)) )
        {
            return false;
        }
    }

    if( true == enableRegexp_Payload)
    {
        if( (true == enablePayload) && ( false == payloadRegularExpression.match(msg.toStringPayload()).hasMatch() ) )
        {
            return false;
        }
    }
    else
    {
        if( (true == enablePayload) && ( false == msg.toStringPayload().contains(payload,ignoreCase_Payload?Qt::CaseInsensitive:Qt::CaseSensitive)) )
        {
            return false;
        }
    }

    if (true == enableMessageId)
    {
        if (messageIdMax==0)
        {
            if(  false == ((msg.getMessageId()==messageIdMin)) )
                {
                    return false;
                }
        }
        else 
        {
            if( false == ((msg.getMessageId()>=messageIdMin)&&(msg.getMessageId()<messageIdMax)) )
                {
                    return false;
                }
        }
    }

    if(enableCtrlMsgs && !((msg.getType() == QDltMsg::DltTypeControl)))
    {
        return false;
    }
    if(enableLogLevelMax && !((msg.getType() == QDltMsg::DltTypeLog) && (msg.getSubtype() <= logLevelMax)))
    {
        return false;
    }
    if(enableLogLevelMin && !((msg.getType() == QDltMsg::DltTypeLog) && (msg.getSubtype() >= logLevelMin)))
    {
        return false;
    }

    return true;
}

void QDltFilter::LoadFilterItem(QXmlStreamReader &xml)
{
    if(xml.name() == QString("type"))
    {
          type = (QDltFilter::FilterType)(xml.readElementText().toInt());

    }
    if(xml.name() == QString("name"))
    {
          name = xml.readElementText();

    }
    if(xml.name() == QString("ecuid"))
    {
          ecuid = xml.readElementText();

    }
    if(xml.name() == QString("applicationid"))
    {
          apid = xml.readElementText();

    }
    if(xml.name() == QString("contextid"))
    {
          ctid = xml.readElementText();

    }
    if(xml.name() == QString("headertext"))
    {
          header = xml.readElementText();
    }
    if(xml.name() == QString("payloadtext"))
    {
          payload = xml.readElementText();
    }
    if(xml.name() == QString("regex_search"))
    {
          regex_search = xml.readElementText();
    }
    if(xml.name() == QString("regex_replace"))
    {
          regex_replace = xml.readElementText();
    }
    if(xml.name() == QString("enableregexp"))    //legacy
    {
        enableRegexp_Appid   = xml.readElementText().toInt();
        enableRegexp_Context = xml.readElementText().toInt();
        enableRegexp_Header  = xml.readElementText().toInt();
    }
    if(xml.name() == QString("enableregexp_Appid"))
    {
          enableRegexp_Appid = xml.readElementText().toInt();
    }
    if(xml.name() == QString("enableregexp_Context"))
    {
          enableRegexp_Context = xml.readElementText().toInt();
    }
    if(xml.name() == QString("enableregexp_Header"))
    {
          enableRegexp_Header = xml.readElementText().toInt();
    }
    if(xml.name() == QString("enableregexp_Payload"))
    {
          enableRegexp_Payload = xml.readElementText().toInt();
    }
    if(xml.name() == QString("ignoreCase_Header"))
    {
          ignoreCase_Header = xml.readElementText().toInt();
    }
    if(xml.name() == QString("ignoreCase_Payload"))
    {
          ignoreCase_Payload = xml.readElementText().toInt();
    }
    if(xml.name() == QString("enablefilter"))
    {
          enableFilter = xml.readElementText().toInt();
    }
    if(xml.name() == QString("enableecuid"))
    {
          enableEcuid = xml.readElementText().toInt();
    }
    if(xml.name() == QString("enableapplicationid"))
    {
          enableApid = xml.readElementText().toInt();;
    }
    if(xml.name() == QString("enablecontextid"))
    {
          enableCtid = xml.readElementText().toInt();;
    }
    if(xml.name() == QString("enableheadertext"))
    {
          enableHeader = xml.readElementText().toInt();;
    }
    if(xml.name() == QString("enablepayloadtext"))
    {
          enablePayload = xml.readElementText().toInt();;
    }
    if(xml.name() == QString("enablectrlmsgs"))
    {
          enableCtrlMsgs = xml.readElementText().toInt();;
    }
    if(xml.name() == QString("enableLogLevelMax"))
    {
          enableLogLevelMax = xml.readElementText().toInt();;
    }
    if(xml.name() == QString("enableLogLevelMin"))
    {
          enableLogLevelMin = xml.readElementText().toInt();;
    }
    if(xml.name() == QString("enableMarker"))
    {
          enableMarker = xml.readElementText().toInt();;
    }
    if(xml.name() == QString("enableMessageId"))
    {
          enableMessageId = xml.readElementText().toInt();;
    }
    if(xml.name() == QString("enableRegexSearchReplace"))
    {
          enableRegexSearchReplace = xml.readElementText().toInt();;
    }
    if(xml.name() == QString("filterColour"))
    {
          filterColour = xml.readElementText();
    }
    if(xml.name() == QString("logLevelMax"))
    {
          logLevelMax = xml.readElementText().toInt();;
    }
    if(xml.name() == QString("logLevelMin"))
    {
          logLevelMin = xml.readElementText().toInt();;
    }
    if(xml.name() == QString("messageIdMax"))
    {
          messageIdMax = xml.readElementText().toUInt();
    }
    if(xml.name() == QString("messageIdMin"))
    {
          messageIdMin = xml.readElementText().toUInt();
    }

}

void QDltFilter::SaveFilterItem(QXmlStreamWriter &xml)
{
    xml.writeTextElement("type",QString("%1").arg((int)(type)));

    xml.writeTextElement("name",name);
    xml.writeTextElement("ecuid",ecuid);
    xml.writeTextElement("applicationid",apid);
    xml.writeTextElement("contextid",ctid);
    xml.writeTextElement("headertext",header);
    xml.writeTextElement("payloadtext",payload);
    xml.writeTextElement("regex_search",regex_search);
    xml.writeTextElement("regex_replace",regex_replace);
    xml.writeTextElement("messageIdMin",QString("%1").arg(messageIdMin));
    xml.writeTextElement("messageIdMax",QString("%1").arg(messageIdMax));

    xml.writeTextElement("enableregexp_Appid",QString("%1").arg(enableRegexp_Appid));
    xml.writeTextElement("enableregexp_Context",QString("%1").arg(enableRegexp_Context));
    xml.writeTextElement("enableregexp_Header",QString("%1").arg(enableRegexp_Header));
    xml.writeTextElement("enableregexp_Payload",QString("%1").arg(enableRegexp_Payload));
    xml.writeTextElement("ignoreCase_Header",QString("%1").arg(ignoreCase_Header));
    xml.writeTextElement("ignoreCase_Payload",QString("%1").arg(ignoreCase_Payload));
    xml.writeTextElement("enablefilter",QString("%1").arg(enableFilter));
    xml.writeTextElement("enableecuid",QString("%1").arg(enableEcuid));
    xml.writeTextElement("enableapplicationid",QString("%1").arg(enableApid));
    xml.writeTextElement("enablecontextid",QString("%1").arg(enableCtid));
    xml.writeTextElement("enableheadertext",QString("%1").arg(enableHeader));
    xml.writeTextElement("enablepayloadtext",QString("%1").arg(enablePayload));
    xml.writeTextElement("enablectrlmsgs",QString("%1").arg(enableCtrlMsgs));
    xml.writeTextElement("enableLogLevelMin",QString("%1").arg(enableLogLevelMin));
    xml.writeTextElement("enableLogLevelMax",QString("%1").arg(enableLogLevelMax));
    xml.writeTextElement("enableMarker",QString("%1").arg(enableMarker));
    xml.writeTextElement("enableMessageId",QString("%1").arg(enableMessageId));
    xml.writeTextElement("enableRegexSearchReplace",QString("%1").arg(enableRegexSearchReplace));

    xml.writeTextElement("filterColour",filterColour);

    xml.writeTextElement("logLevelMax",QString("%1").arg(logLevelMax));
    xml.writeTextElement("logLevelMin",QString("%1").arg(logLevelMin));
}

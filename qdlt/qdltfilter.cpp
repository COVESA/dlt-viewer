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

    enableRegexp = _filter.enableRegexp;
    enableFilter = _filter.enableFilter;
    enableEcuid = _filter.enableEcuid;
    enableApid = _filter.enableApid;
    enableCtid = _filter.enableCtid;
    enableHeader = _filter.enableHeader;
    enablePayload = _filter.enablePayload;
    enableCtrlMsgs = _filter.enableCtrlMsgs;
    enableLogLevelMax = _filter.enableLogLevelMax;
    enableLogLevelMin = _filter.enableLogLevelMin;

    filterColour = _filter.filterColour;
    logLevelMax = _filter.logLevelMax;
    logLevelMin = _filter.logLevelMin;

    // generated from header and payload string
    headerRegexp = _filter.headerRegexp;
    payloadRegexp = _filter.payloadRegexp;

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

    enableRegexp = false;
    enableFilter = false;
    enableEcuid = false;
    enableApid = false;
    enableCtid = false;
    enableHeader = false;
    enablePayload = false;
    enableCtrlMsgs = false;
    enableLogLevelMax = false;
    enableLogLevelMin = false;

    filterColour = QColor();
    logLevelMax = 6;
    logLevelMin = 0;
}

bool QDltFilter::compileRegexps()
{
    headerRegexp.setPattern(header);
    payloadRegexp.setPattern(payload);
    return (headerRegexp.isValid() && payloadRegexp.isValid());
}

bool QDltFilter::match(QDltMsg &msg)
{
    if(enableEcuid && (msg.getEcuid() != ecuid)) {
        return false;
    }
    if(enableApid && (msg.getApid() != apid)) {
        return false;
    }
    if(enableCtid && (msg.getCtid() != ctid)) {
        return false;
    }
    if(enableRegexp)
    {
        if(enableHeader && headerRegexp.indexIn(msg.toStringHeader()) < 0) {
            return false;
        }
        if(enablePayload && payloadRegexp.indexIn(msg.toStringPayload()) < 0) {
            return false;
        }
    }
    else
    {
        if(enableHeader && !(msg.toStringHeader().contains(header))) {
            return false;
        }
        if(enablePayload && !(msg.toStringPayload().contains(payload))) {
            return false;
        }
    }
    if(enableCtrlMsgs && !((msg.getType() == QDltMsg::DltTypeControl))) {
        return false;
    }
    if(enableLogLevelMax && !((msg.getType() == QDltMsg::DltTypeLog) && (msg.getSubtype() <= logLevelMax))) {
        return false;
    }
    if(enableLogLevelMin && !((msg.getType() == QDltMsg::DltTypeLog) && (msg.getSubtype() >= logLevelMin))) {
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
    if(xml.name() == QString("enableregexp"))
    {
          enableRegexp = xml.readElementText().toInt();
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
    if(xml.name() == QString("filterColour"))
    {
          filterColour = QColor(xml.readElementText());
    }
    if(xml.name() == QString("logLevelMax"))
    {
          logLevelMax = xml.readElementText().toInt();;
    }
    if(xml.name() == QString("logLevelMin"))
    {
          logLevelMin = xml.readElementText().toInt();;
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

    xml.writeTextElement("enableregexp",QString("%1").arg(enableRegexp));
    xml.writeTextElement("enablefilter",QString("%1").arg(enableFilter));
    xml.writeTextElement("enableecuid",QString("%1").arg(enableEcuid));
    xml.writeTextElement("enableapplicationid",QString("%1").arg(enableApid));
    xml.writeTextElement("enablecontextid",QString("%1").arg(enableCtid));
    xml.writeTextElement("enableheadertext",QString("%1").arg(enableHeader));
    xml.writeTextElement("enablepayloadtext",QString("%1").arg(enablePayload));
    xml.writeTextElement("enablectrlmsgs",QString("%1").arg(enableCtrlMsgs));
    xml.writeTextElement("enableLogLevelMin",QString("%1").arg(enableLogLevelMin));
    xml.writeTextElement("enableLogLevelMax",QString("%1").arg(enableLogLevelMax));

    xml.writeTextElement("filterColour",filterColour.name());

    xml.writeTextElement("logLevelMax",QString("%1").arg(logLevelMax));
    xml.writeTextElement("logLevelMin",QString("%1").arg(logLevelMin));
}

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

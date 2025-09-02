/**
 * @licence app begin@
 * Copyright (C) 2015-2016  Harman Becker Automotive Systems GmbH
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
 * \author Olaf Dreyer <olaf.dreyer@harman.com>
 *
 * \file qdltmessagedecoder.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef QDLTMESSAGEDECODER_HPP
#define QDLTMESSAGEDECODER_HPP

#include "export_rules.h"
#include "qdltmsg.h"

class QDltMsg;

class QDLT_EXPORT QDltMessageDecoder
{
public:
    QDltMessageDecoder();
    virtual ~QDltMessageDecoder();

    //! Decode message by decoding through all loaded an activated decoder plugins.
    /*!
      \param msg The message to be decoded.
      \param triggeredByUser Whether decode operation was triggered by the user or not
    */
    virtual void decodeMsg(QDltMsg& /*msg*/, int /*triggeredByUser*/){}

private:
    QDltMessageDecoder(const QDltMessageDecoder&) = delete;
    QDltMessageDecoder& operator= (const QDltMessageDecoder&) = delete;
};
#endif // QDLTMESSAGEDECODER_HPP

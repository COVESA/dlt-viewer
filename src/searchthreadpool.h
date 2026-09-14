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
 * \file searchthreadpool.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef SEARCHTHREADPOOL_H
#define SEARCHTHREADPOOL_H

#include <QThreadPool>

class SearchThreadPool
{
public:
    enum class Priority
    {
        Urgent,
        Background
    };

    static SearchThreadPool &instance();

    QThreadPool *pool(Priority priority);

private:
    SearchThreadPool();

    QThreadPool m_urgentPool;
    QThreadPool m_backgroundPool;
};

#endif // SEARCHTHREADPOOL_H

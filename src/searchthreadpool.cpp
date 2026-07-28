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
 * \file searchthreadpool.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "searchthreadpool.h"

#include <QThread>

namespace {

int interactiveThreadCount()
{
    const int ideal = QThread::idealThreadCount();
    const int capped = (ideal > 0) ? qMin(4, ideal) : 4;
    return qMax(1, capped);
}

int backgroundThreadCount()
{
    const int urgent = interactiveThreadCount();
    return qMax(1, urgent / 2);
}

}

SearchThreadPool &SearchThreadPool::instance()
{
    static SearchThreadPool pool;
    return pool;
}

SearchThreadPool::SearchThreadPool()
{
    m_urgentPool.setMaxThreadCount(interactiveThreadCount());
    m_urgentPool.setThreadPriority(QThread::NormalPriority);

    m_backgroundPool.setMaxThreadCount(backgroundThreadCount());
    m_backgroundPool.setThreadPriority(QThread::LowPriority);
}

QThreadPool *SearchThreadPool::pool(Priority priority)
{
    return priority == Priority::Urgent ? &m_urgentPool : &m_backgroundPool;
}
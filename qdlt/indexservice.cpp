/**
 * @licence app begin@
 * Copyright (C) 2026
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 * Contributions are licensed to the COVESA Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file indexservice.cpp
 * @licence end@
 */

#include "indexservice.h"

std::size_t CIndexService::fullRowCount(const std::vector<int> &fullProjection) const
{
    return fullProjection.size();
}

std::size_t CIndexService::filteredRowCount(const std::vector<int> &filteredProjection) const
{
    return filteredProjection.size();
}

int CIndexService::globalIndexForFullRow(int row, const std::vector<int> &fullProjection) const
{
    if (row < 0 || row >= static_cast<int>(fullProjection.size()))
        return -1;

    const int globalIndex = fullProjection.at(static_cast<std::size_t>(row));
    return globalIndex >= 0 ? globalIndex : -1;
}

int CIndexService::globalIndexForFilteredRow(int row,
                                             const std::vector<int> &filteredProjection) const
{
    if (row < 0 || row >= static_cast<int>(filteredProjection.size()))
        return -1;

    const int globalIndex = filteredProjection.at(static_cast<std::size_t>(row));
    return globalIndex >= 0 ? globalIndex : -1;
}

std::vector<int> CIndexService::snapshotProjection(const std::vector<int> &projection) const
{
    std::vector<int> snapshot;
    snapshot.reserve(projection.size());
    for (const int index : projection)
    {
        if (index >= 0)
            snapshot.push_back(index);
    }

    return snapshot;
}
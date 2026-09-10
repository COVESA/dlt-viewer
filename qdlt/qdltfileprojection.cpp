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
 * \file qdltfileprojection.cpp
 * @licence end@
 */

#include "qdltfileprojection.h"

#include "qdltfile.h"

#include <limits>

std::vector<int> buildActiveFilteredProjection(const QDltFile *file)
{
    std::vector<int> filteredProjection;
    if (!file)
        return filteredProjection;

    if (!file->isFilter())
    {
        const int total = file->size();
        filteredProjection.reserve(total);
        for (int i = 0; i < total; ++i)
            filteredProjection.push_back(i);
        return filteredProjection;
    }

    const auto &filterRef = file->getIndexFilterRef();
    filteredProjection.reserve(filterRef.size());
    for (const auto index : filterRef)
    {
        if (index >= 0 && index <= std::numeric_limits<int>::max())
            filteredProjection.push_back(static_cast<int>(index));
    }

    return filteredProjection;
}

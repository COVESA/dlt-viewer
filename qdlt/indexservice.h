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
 * \file indexservice.h
 * @licence end@
 */

#ifndef INDEXSERVICE_H
#define INDEXSERVICE_H

#include "export_rules.h"

#include <cstddef>
#include <vector>

class QDLT_EXPORT CIndexService
{
public:
    //! Return the number of rows in the provided full projection.
    std::size_t fullRowCount(const std::vector<int> &fullProjection) const;
    //! Return the number of rows in the provided filtered projection.
    std::size_t filteredRowCount(const std::vector<int> &filteredProjection) const;

    //! Map a full-view row to its global message index.
    int globalIndexForFullRow(int row, const std::vector<int> &fullProjection) const;
    //! Map a filtered-view row to its global message index.
    int globalIndexForFilteredRow(int row, const std::vector<int> &filteredProjection) const;

    //! Snapshot the provided projection as global indexes.
    std::vector<int> snapshotProjection(const std::vector<int> &projection) const;
};

#endif // INDEXSERVICE_H
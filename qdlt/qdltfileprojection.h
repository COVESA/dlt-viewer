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
 * \file qdltfileprojection.h
 * @licence end@
 */

#ifndef QDLTFILEPROJECTION_H
#define QDLTFILEPROJECTION_H

#include "export_rules.h"

#include <vector>

class QDltFile;

/**
 * @brief Build the active global-index projection for a QDltFile.
 *
 * The result contains either all message indexes or the currently visible
 * filtered indexes, including any active manual markers in their display order.
 * The returned vector is independent of @p file.
 * @param file File whose filter and sorting state should be projected.
 * @return Global message indexes in the active view order, or an empty vector
 *         when @p file is null or contains no indexed messages.
 */
QDLT_EXPORT std::vector<int> buildActiveFilteredProjection(const QDltFile *file);

#endif // QDLTFILEPROJECTION_H

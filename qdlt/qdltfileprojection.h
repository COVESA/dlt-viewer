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

//! Generates projection from file/filter state.
QDLT_EXPORT std::vector<int> buildActiveFilteredProjection(const QDltFile *file);

#endif // QDLTFILEPROJECTION_H

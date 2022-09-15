/**
 * @licence app begin@
 * Copyright (C) 2014  BMW AG
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
 * \author
 * Alexander Wenzel <alexander.aw.wenzel@bmw.de>
 *
 * \file example5.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef EXAMPLE5_H
#define EXAMPLE5_H

extern "C"
{
    #include"dlt_common_api.h"
}

extern void example5_init();
extern void example5_test1();
extern void example5_test2();
extern void example5_test3(int repeat);

#endif // EXAMPLE5_H

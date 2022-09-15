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
 * \file mapp.c
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "dlt_common_api.h"

#include "dlt_id.h"

DLT_DECLARE_CONTEXT(myContext1)
DLT_DECLARE_CONTEXT(myContext2)
DLT_DECLARE_CONTEXT(myContext3)

void main()
{
    DLT_REGISTER_APP("MAPP","My Application");

    DLT_REGISTER_CONTEXT_APP(myContext1,"MCT1","MAPP","My Context 1");
    DLT_REGISTER_CONTEXT_APP(myContext2,"MCT2","MAPP","My Context 2");
    DLT_REGISTER_CONTEXT_APP(myContext3,"MCT3","MAPP","My Context 3");

    DLT_LOG_ID0(myContext1,DLT_LOG_INFO,DLT_MSG_ID_MAPP_MCT1_1);
    DLT_LOG_ID1(myContext2,DLT_LOG_ERROR,DLT_MSG_ID_MAPP_MCT2_1, DLT_STRING("Hello DLT message"));
    DLT_LOG_ID2(myContext3,DLT_LOG_VERBOSE,DLT_MSG_ID_MAPP_MCT3_1, DLT_CSTRING("Hello value"), DLT_INT(-100));
    DLT_LOG_ID3(myContext1,DLT_LOG_FATAL,DLT_MSG_ID_MAPP_MCT1_2, DLT_CSTRING("Hello value2"), DLT_INT(-100), DLT_UINT(200));

    DLT_UNREGISTER_CONTEXT(myContext1);
    DLT_UNREGISTER_CONTEXT(myContext2);
    DLT_UNREGISTER_CONTEXT(myContext3);

    DLT_UNREGISTER_APP();
}

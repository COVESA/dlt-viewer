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
 * \file capp.c
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include "dlt_common_api.h"

#include "dlt_id_CAPP.h"

DLT_DECLARE_CONTEXT(mycContext1)
DLT_DECLARE_CONTEXT(mycContext2)
DLT_DECLARE_CONTEXT(mycContext3)

void main()
{
    DLT_REGISTER_APP("CAPP","My Convert Application");

    DLT_REGISTER_CONTEXT_APP(mycContext1,"CCT1","CAPP","My Context 1");
    DLT_REGISTER_CONTEXT_APP(mycContext2,"CCT2","CAPP","My Context 2");
    DLT_REGISTER_CONTEXT_APP(mycContext3,"CCT3","CAPP","My Context 3");

    DLT_LOG_ID0(mycContext1,DLT_LOG_INFO,DLT_MSG_ID_CAPP_CCT1_944ef165);
    DLT_LOG_ID1(mycContext2,DLT_LOG_ERROR,DLT_MSG_ID_CAPP_CCT2_4451f8f2,DLT_STRING("Hello DLT message"));
    DLT_LOG_ID2(mycContext3,DLT_LOG_VERBOSE,DLT_MSG_ID_CAPP_CCT3_545fc245,DLT_CSTRING("Hello value"),DLT_INT(-100));
    DLT_LOG_ID3(mycContext1,DLT_LOG_FATAL,DLT_MSG_ID_CAPP_CCT1_387ba9d6,DLT_CSTRING("Hello value2"),DLT_INT(-100),DLT_UINT(200));

    DLT_UNREGISTER_CONTEXT(mycContext1);
    DLT_UNREGISTER_CONTEXT(mycContext2);
    DLT_UNREGISTER_CONTEXT(mycContext3);

    DLT_UNREGISTER_APP();
}

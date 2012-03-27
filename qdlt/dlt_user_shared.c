/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> 2011-2012
 * 
 * \file dlt_user_shared.c
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

/*******************************************************************************
**                                                                            **
**  SRC-MODULE: dlt_user_shared.c                                             **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**              Markus Klein                                                  **
**                                                                            **
**  PURPOSE   :                                                               **
**                                                                            **
**  REMARKS   :                                                               **
**                                                                            **
**  PLATFORM DEPENDANT [yes/no]: yes                                          **
**                                                                            **
**  TO BE CHANGED BY USER [yes/no]: no                                        **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision: 1522 $
 * $LastChangedDate: 2010-12-14 09:03:50 +0100 (Di, 14. Dez 2010) $
 * $LastChangedBy$
 Initials    Date         Comment
 aw          13.01.2010   initial
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/uio.h> /* writev() */

#include "dlt_user_shared.h"
#include "dlt_user_shared_cfg.h"

int dlt_user_set_userheader(DltUserHeader *userheader, uint32_t mtype)
{
    if (userheader==0)
    {
        return -1;
    }

    if (mtype<=0)
    {
        return -1;
    }

    userheader->pattern[0] = 'D';
    userheader->pattern[1] = 'U';
    userheader->pattern[2] = 'H';
    userheader->pattern[3] = 1;
    userheader->message = mtype;

    return 0;
}

int dlt_user_check_userheader(DltUserHeader *userheader)
{
    if (userheader==0)
	{
        return -1;
	}

    return  ((userheader->pattern[0] == 'D') &&
             (userheader->pattern[1] == 'U') &&
             (userheader->pattern[2] == 'H') &&
             (userheader->pattern[3] == 1));
}

DltReturnValue dlt_user_log_out2(int handle, void *ptr1, size_t len1, void* ptr2, size_t len2)
{
    struct iovec iov[2];
    int bytes_written;

    if (handle<=0)
    {
        /* Invalid handle */
        return DLT_RETURN_ERROR;
    }

    iov[0].iov_base = ptr1;
    iov[0].iov_len = len1;
    iov[1].iov_base = ptr2;
    iov[1].iov_len = len2;

    bytes_written = writev(handle, iov, 2);

    if (bytes_written!=(len1+len2))
    {
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

DltReturnValue dlt_user_log_out3(int handle, void *ptr1, size_t len1, void* ptr2, size_t len2, void *ptr3, size_t len3)
{
    struct iovec iov[3];
    int bytes_written;

    if (handle<=0)
    {
        /* Invalid handle */
        return DLT_RETURN_ERROR;
    }

    iov[0].iov_base = ptr1;
    iov[0].iov_len = len1;
    iov[1].iov_base = ptr2;
    iov[1].iov_len = len2;
    iov[2].iov_base = ptr3;
    iov[2].iov_len = len3;

    bytes_written = writev(handle, iov, 3);

    if (bytes_written!=(len1+len2+len3))
    {
        switch(errno)
        {
            case EBADF:
            {
                return DLT_RETURN_PIPE_ERROR; /* EBADF - handle not open */
                break;
            }
            case EPIPE:
            {
                return DLT_RETURN_PIPE_ERROR; /* EPIPE - pipe error */
                break;
            }
            case EAGAIN:
            {
                return DLT_RETURN_PIPE_FULL; /* EAGAIN - data could not be written */
                break;
            }
            default:
            {
                break;
            }
        }
        return DLT_RETURN_ERROR;
    }

    return DLT_RETURN_OK;
}

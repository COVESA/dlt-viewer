/*
 * Dlt- Diagnostic Log and Trace user library
 * @licence app begin@
 *
 * Copyright (C) 2011, BMW AG - Alexander Wenzel <alexander.wenzel@bmw.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the 
 * GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General 
 * Public License, version 2.1, for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along 
 * with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * 
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may 
 * also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * 
 * Linking DLT statically or dynamically with other modules is making a combined work based on DLT. You may 
 * license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to 
 * license your linked modules under the GNU Lesser General Public License, version 2.1, you 
 * may use the program under the following exception.
 * 
 * As a special exception, the copyright holders of DLT give you permission to combine DLT 
 * with software programs or libraries that are released under any license unless such a combination is not
 * permitted by the license of such a software program or library. You may copy and distribute such a 
 * system following the terms of the GNU Lesser General Public License, version 2.1, including this
 * special exception, for DLT and the licenses of the other code concerned.
 * 
 * Note that people who make modified versions of DLT are not obligated to grant this special exception 
 * for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, 
 * version 2.1, gives permission to release a modified version without this exception; this exception 
 * also makes it possible to release a modified version which carries forward this exception.
 *
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

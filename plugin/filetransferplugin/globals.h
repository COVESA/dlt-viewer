/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
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
 * \file globals.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#define COLUMN_FILEID 0
#define COLUMN_FILENAME 1
#define COLUMN_FILEDATE 2
#define COLUMN_STATUS 3
#define COLUMN_CHECK 4
#define COLUMN_SIZE 5
#define COLUMN_PACKAGES 6
#define COLUMN_RECPACKAGES 7
#define COLUMN_BUFFERSIZE 8

#define PROTOCOL_FLST_STARTFLAG 0
#define PROTOCOL_FLST_FILEID 1
#define PROTOCOL_FLST_FILENAME 2
#define PROTOCOL_FLST_FILEDATE 4
#define PROTOCOL_FLST_SIZE 3
#define PROTOCOL_FLST_PACKAGES 5
#define PROTOCOL_FLST_BUFFERSIZE 6
#define PROTOCOL_FLST_ENDFLAG 7

#define PROTOCOL_FLDA_STARTFLAG 0
#define PROTOCOL_FLDA_FILEID 1
#define PROTOCOL_FLDA_PACKAGENR 2
#define PROTOCOL_FLDA_DATA 3
#define PROTOCOL_FLDA_ENDFLAG 4

#define PROTOCOL_FLFI_STARTFLAG 0
#define PROTOCOL_FLFI_FILEID 1
#define PROTOCOL_FLFI_ENDFLAG 2

#define PROTOCOL_FLER_STARTFLAG 0
#define PROTOCOL_FLER_ERRCODE1 1
#define PROTOCOL_FLER_ERRCODE2 2
#define PROTOCOL_FLER_FILENAME 3
#define PROTOCOL_FLER_ENDFLAG 4

#define PROTOCOL_FLIF_STARTFLAG 0
#define PROTOCOL_FLIF_ENDFLAG 11

#define PROTOCOL_ALL_STARTFLAG 0

#endif // GLOBALS_H

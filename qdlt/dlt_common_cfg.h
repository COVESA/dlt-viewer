/*
 * Dlt- Diagnostic Log and Trace daemon
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
**  SRC-MODULE: dlt_common_cfg.h                                              **
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
**                      Author Identity                                       **
********************************************************************************
**                                                                            **
** Initials     Name                       Company                            **
** --------     -------------------------  ---------------------------------- **
**  aw          Alexander Wenzel           BMW                                **
**  mk          Markus Klein               Fraunhofer ESK                     **
*******************************************************************************/

#ifndef DLT_COMMON_CFG_H
#define DLT_COMMON_CFG_H

/*************/
/* Changable */
/*************/

/* Buffer length for temporary buffer */
#define DLT_COMMON_BUFFER_LENGTH 255

/* Number of ASCII chars to be printed in one line as HEX and as ASCII */
/* e.g. XX XX XX XX ABCD is DLT_COMMON_HEX_CHARS = 4 */
#define DLT_COMMON_HEX_CHARS  16

/* Length of line number */
#define DLT_COMMON_HEX_LINELEN 8

/* Length of one char */
#define DLT_COMMON_CHARLEN     1

/* Number of indices to be allocated at one, if no more indeces are left */
#define DLT_COMMON_INDEX_ALLOC	   1000

/* If limited output is called, 
   this is the maximum number of characters to be printed out */
#define DLT_COMMON_ASCII_LIMIT_MAX_CHARS 20

/* This defines the dummy ECU ID set in storage header during import
   of a message from a DLT file in RAW format (without storage header) */
#define DLT_COMMON_DUMMY_ECUID "ECU"


/************************/
/* Don't change please! */
/************************/

/* ASCII value for space */
#define DLT_COMMON_ASCII_CHAR_SPACE  32

/* ASCII value for tilde */
#define DLT_COMMON_ASCII_CHAR_TILDE 126

/* ASCII value for lesser than */
#define DLT_COMMON_ASCII_CHAR_LT     60

#endif /* DLT_COMMON_CFG_H */


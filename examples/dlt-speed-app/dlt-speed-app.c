/*
 * Dlt Speed App - Example Spped Application
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
**  SRC-MODULE: dlt-speed-app.c                                               **
**                                                                            **
**  TARGET    : linux                                                         **
**                                                                            **
**  PROJECT   : DLT                                                           **
**                                                                            **
**  AUTHOR    : Alexander Wenzel Alexander.AW.Wenzel@bmw.de                   **
**              Christian Muck christian.muck@bmw.de                          **
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
**  cm          Christian Muck             BMW 
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/

/*
 * $LastChangedRevision: xxxx $
 * $LastChangedDate: 2010-12-13 10:07:42 +0100 (Mo, 13. Dez 2010) $
 * $LastChangedBy$
 Initials    Date         Comment
 aw          13.01.2010   initial
 cm          27.04.2011   fixing
 */
 
 
#include <netdb.h>		/* Definitions for network database operations */
#include <ctype.h>		/* ANSI C Standard Library for the C programming language contains declarations for character classification functions*/
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */


/**
 * Include the dlt header file for using dlt functions
 */
#include <dlt/dlt.h>

#define DEFAULT_DELAY 500
#define DEFAULT_MESSAGES 1000

/**
 * 
 */ 
int maxMessageCount;
int delay; 
int hflag = 0;
int gflag = 0;
int aflag = 0;
char *dvalue = 0;
char *fvalue = 0;
char *nvalue = 0;

//For injection callback - delete if interface does not provide injection
/*
char *message = 0;
char *text = 0;
int dlt_user_injection_callback(uint32_t service_id, void *data, uint32_t length);
*/

/**
 * Create logging context 
 */
DLT_DECLARE_CONTEXT(mycontext);


/**
 * Print usage information of tool.
 */
void printUsage()
{
    char version[255];

    dlt_get_version(version);

    printf("Usage: dlt-speed-app [options] mode\n");
    printf("Generate DLT messages with current speed information.\n");
    printf("%s \n", version);
    printf("Mode:\n");
    printf("  1             Ramp up and down from 0 - 100 km/h\n");
    printf("Options:\n");
    printf("  -d delay      Milliseconds to wait between sending messages (Default: 500)\n");
    printf("  -f filename   Use local log file instead of sending to daemon (absolute path) - don't forget .dlt\n");
    printf("  -n count      Number of messages to be generated (Default: 1000)\n");
    printf("  -g            Switch to non-verbose mode (Default: verbose mode)\n");
    printf("  -a            Enable local printing of DLT messages (Default: disabled)\n");
    printf("  -h            Print usage\n");
}


/**
 * Parse arguments from program start, see printUsage for possible options 
 */ 
int parseArguments(int argc, char* argv[]){
    int index;
	int c;
	
    opterr = 0;

    while ((c = getopt (argc, argv, "hgad:f:n:")) != -1)
    {
        switch (c)
        {
		case 'h':
        {
            hflag = 1;
            break;
        }
        case 'g':
        {
            gflag = 1;
            break;
        }
        case 'a':
        {
            aflag = 1;
            break;
        }
        case 'd':
        {
            dvalue = optarg;
            break;
        }
        case 'f':
        {
            fvalue = optarg;
            break;
        }
        case 'n':
        {
            nvalue = optarg;
            break;
        }
        case '?':
        {
            if (optopt == 'd' || optopt == 'f' || optopt == 'n')
            {
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            }
            else if (isprint (optopt))
            {
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            }
            else
            {
                fprintf (stderr, "Unknown option character `\\x%x'.\n",optopt);
            }

            /* Unknown or wrong option used, show usage information and terminate */
            printUsage();
            return EXIT_FAILURE;
        }
        default:
        {
			/* Generate an abnormal process abort*/
            abort();
        }
        }
    }

	//For injection callback - delete if interface does not provide injection
	/*
    for (index = optind; index < argc; index++)
    {
        message = argv[index];
    }

    if (message == 0)
    {
        //no message, show usage and terminate
        fprintf(stderr,"ERROR: No message selected\n");
        usage();
        return -1;
    }*/

	if (hflag)
	{
		printUsage();
		return EXIT_SUCCESS;
	}

    /* DLT is intialised automatically, except another output target will be used (i.e. -f option is used to log into file - don't forget .dlt) */
    if (fvalue)
    {
        /* Log to file */
        if (dlt_init_file(fvalue)<0) 
        {
            return EXIT_FAILURE;
        }
		printf("- output: %s\n", fvalue);
    }
    else
    {
		printf("- output: daemon\n");
	}	
    
    
    if (gflag)
    {
        DLT_NONVERBOSE_MODE();
    }
	printf("- nonverbose mode: %i \n",gflag);



    if (aflag)
    {
        DLT_ENABLE_LOCAL_PRINT();
    }
	printf("- local printing: %i \n",aflag);


    if (nvalue && (atoi(nvalue)>0) )
    {
        maxMessageCount = atoi(nvalue);
    }
    else
    {
		maxMessageCount = DEFAULT_MESSAGES;
    }
	printf("- messages: %i \n",maxMessageCount);


    if (dvalue && (atoi(dvalue)>0) )
    {
        delay = atoi(dvalue) * 1000;
    }
    else
    {
        delay = DEFAULT_DELAY * 1000;
    }
    printf("- delay: %i ms\n",delay/1000);
    
    
    
    return EXIT_SUCCESS;
}

/**
 * Main function
 */
int main(int argc, char* argv[])
{

	// Current speed info
	int speed = 0;
	
	// Increase or decrease speed
	int direction = 1;
	
	// Generated messages
	int generatedMessageCount;
	
	
	if(parseArguments(argc,argv) == EXIT_FAILURE)
	{
			return EXIT_FAILURE;
	}
	else if(hflag)
	{
		return EXIT_SUCCESS;
	}
	
	
	
	//Register the application to dlt
    DLT_REGISTER_APP("SPEED","Speed Application");
    
    //Register the context to dlt
    DLT_REGISTER_CONTEXT(mycontext,"SIG","Speed signal");
	
	//Register injection callback to dlt
    //DLT_REGISTER_INJECTION_CALLBACK(mycontext, 0xFFF, dlt_user_injection_callback);


	//For injection callback - delete if interface does not provide injection
    //text = message;

   
	//Loop for generate speed messages and log to dlt
    for (generatedMessageCount=0; generatedMessageCount < maxMessageCount; generatedMessageCount++)
    {
		// Output to console
        printf("Message %d: Speed %d\n",generatedMessageCount,speed);
        
        
        // Log to dlt
        DLT_LOG(mycontext,DLT_LOG_INFO,DLT_STRING("Speed"),DLT_INT(speed));
        
        // Increase or decrease speed depending on direction value
	    speed += direction;
	    
	    // Limitation: 0 < speed < 100
	    if(speed>=100) {
			direction = -1;
		}
		else if(speed<=0) {
			direction = 1;
		}

        if (gflag)
        {
            /* Non-verbose mode */
        }
        else
        {
            /* Verbose mode */
        }

		// Wait between sending messages 
        if (delay>0)
        {
            usleep(delay);
        }
    }

    // Unregister context from dlt
    DLT_UNREGISTER_CONTEXT(mycontext);

	// Unregister application from dlt
    DLT_UNREGISTER_APP();


    return EXIT_SUCCESS;
}

/*int dlt_user_injection_callback(uint32_t service_id, void *data, uint32_t length)
{
    char text[1024];

    printf("Injection %d, Length=%d \n",service_id,length);
    if (length>0)
    {
        dlt_print_mixed_string(text,1024,data,length,0);
        printf("%s \n", text);
    }

    return 0;
}*/


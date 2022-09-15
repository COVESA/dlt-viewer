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
 *
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> BMW 2011,2012
 *
 * \file dlt-speed-app.c
 * For further information see http://www.covesa.global/.
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
 gw          20.09.2018   add injection example
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
#include <dlt.h>

#define DEFAULT_DELAY 1000
#define DEFAULT_MESSAGES 10000000

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

// Current speed info
int speed = 0;
// Increase or decrease speed
int direction = 1;

//For injection callback - delete if interface does not provide injection

char *message = 0;
char *text = 0;

int dlt_user_injection_callback(uint32_t service_id, void *data, uint32_t length);


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

    dlt_get_version(version, 255);

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

int dlt_user_injection_callback(uint32_t service_id, void *data, uint32_t length)
{
    char text[1024];
    printf("Injection received: %d, Length=%d \n",service_id,length);

    if (length>0)
    {
	int par = atoi ( (char *) data);
	 switch (service_id)
       {
	    case 4096:
		     direction = direction * -1;
		     printf("ServiceID: %i - Change direction to %i,  par is %i\n",service_id, direction, par);
             //dlt_print_mixed_string(text,1024,data,length,0);
             //printf("%s\n", text);
             break;

	    case 4097:
	         speed = par;
	         printf("ServiceID: %i - Set speed to %i \n",service_id, par);
	         break;

	   case 4098:
	        delay = par*10000;
	        printf("ServiceID: %i - Delay set to %i \n",service_id, delay);
	        break;

	   default:
	       break;
     }

    }

    return 0;
}
/**
 * Main function
 */
int main(int argc, char* argv[])
{
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
    DLT_REGISTER_INJECTION_CALLBACK(mycontext, 0x1000, dlt_user_injection_callback);
    DLT_REGISTER_INJECTION_CALLBACK(mycontext, 0x1001, dlt_user_injection_callback);
    DLT_REGISTER_INJECTION_CALLBACK(mycontext, 0x1002, dlt_user_injection_callback);

	//For injection callback - delete if interface does not provide injection
    text = message;

   
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
	    if(speed>=100)
	    {
			direction = -1;
			//printf("up\n");
		}
		else if(speed<=0) 
		{
			direction = 1;
			//printf("down\n");
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




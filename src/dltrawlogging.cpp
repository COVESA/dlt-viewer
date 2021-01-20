#include "dltrawlogging.h"

#include <dlt_common.h>

#if defined(_MSC_VER)
#include <io.h>
#include <time.h>
#include <WinSock.h>
#else
#include <unistd.h>     /* for read(), close() */
#include <sys/time.h>	/* for gettimeofday() */
#endif

/**
 * The structure of the DLT raw message header. This header is used before each stored raw data.
 */
typedef struct
{
    char pattern[DLT_ID_SIZE];		/**< This pattern should be LTR0x01 */
    uint32_t seconds;			    /**< seconds since 1.1.1970 */
    int32_t microseconds;			/**< Microseconds */
    char ecu[DLT_ID_SIZE];			/**< The ECU id is added to identify the data */
    uint16_t size;			        /**< size of payload following the header */
} PACKED DltRawHeader;


DltRawLogging::DltRawLogging()
{
    timeout = 5;
    maxFileSize = 100ul * 1000ul * 1000ul;
}

DltRawLogging::~DltRawLogging()
{

    /* if output file is still open during shutdown close it and rename it with end time */
    if(outputFile.isOpen())
    {
        outputFile.close();
        outputFile.rename(path+"/"+projectName+beginTime.toString("_yyyy-MM-dd[hh.mm.ss]")+endTime.toString("_yyyy-MM-dd[hh.mm.ss]")+".rlt");
    }
}

bool DltRawLogging::writeData(QString &ecuId, QByteArray &data)
{

    /* close file, if the time since last received data is elapsed */
    checkEndTime();

    /* close file, if the maximum file size is reached */
    checkFileSize();

    /* if file is not open yet, create a new one */
    if(!outputFile.isOpen())
    {
        beginTime = QDateTime::currentDateTime();

        outputFile.setFileName(path+"/"+projectName+beginTime.toString("_yyyy-MM-dd[hh.mm.ss]")+".rlt");

        outputFile.open(QIODevice::WriteOnly);
    }

    /* prepare raw datat header */
    DltRawHeader raw;
    raw.pattern[0]='L';
    raw.pattern[1]='T';
    raw.pattern[2]='R';
    raw.pattern[3]=0x01;
    raw.ecu[0]=0;
    raw.ecu[1]=0;
    raw.ecu[2]=0;
    raw.ecu[3]=0;

    /* write time into header */
    #if defined(_MSC_VER)
       SYSTEMTIME systemtime;
       GetSystemTime(&systemtime);
       time_t timestamp_sec;
       time(&timestamp_sec);
       raw.seconds = (time_t)timestamp_sec;
       raw.microseconds = (int32_t)systemtime.wMilliseconds * 1000; // for some reasons we do not have microseconds in Windows !
    #else
        struct timeval tv;
        gettimeofday(&tv, NULL);
        raw.seconds = (time_t)tv.tv_sec; /* value is long */
        raw.microseconds = (int32_t)tv.tv_usec; /* value is long */
    #endif

    /* write ECU id into header */
    dlt_set_id(raw.ecu,ecuId.toLatin1());

    /* write size of payload into header */
    raw.size = data.size();

    /* write header and payload into file */
    outputFile.write((char*)&raw,sizeof(DltRawHeader));
    outputFile.write(data);
    outputFile.flush();

    /* save time of last received data */
    endTime = QDateTime::currentDateTime();

    return true; // success
}

void DltRawLogging::checkEndTime()
{
    /* check if timeout is set */
    if(timeout > 0)
    {
        /* check if timeout of last received data reached */
        QDateTime now = QDateTime::currentDateTime();
        if(!endTime.isNull() && endTime.secsTo(now)>timeout)
        {
            /* close and rename current output file */
            if(outputFile.isOpen())
            {
                outputFile.close();
                outputFile.rename(path+"/"+projectName+beginTime.toString("_yyyy-MM-dd[hh.mm.ss]")+endTime.toString("_yyyy-MM-dd[hh.mm.ss]")+".rlt");
            }
        }
    }
}

void DltRawLogging::checkFileSize()
{
    /* close and rename current ouput file */
    if((maxFileSize > 0) && outputFile.isOpen() && (outputFile.size() >= maxFileSize) )
    {
        outputFile.close();
        outputFile.rename(path+"/"+projectName+beginTime.toString("_yyyy-MM-dd[hh.mm.ss]")+endTime.toString("_yyyy-MM-dd[hh.mm.ss]")+".rlt");
    }
}

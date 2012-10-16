#include "threaddltindex.h"
#include <QDebug>

ThreadDltIndex::ThreadDltIndex(QObject *parent) :  QThread(parent)
{
}

void ThreadDltIndex::run(){

    QByteArray buf;
    unsigned long pos = 0;

    /* clear old index */
    indexAll.clear();

    /* set new filename */
    infile.setFileName(filename);

    /* open the log file read only */
    if(infile.open(QIODevice::ReadOnly)==false) {
        /* open file failed */
        qWarning() << currentThreadId() << " thread: open of file" << filename << "failed";
        return;
    }

    qDebug() << "Started thread " << currentThreadId() << " and opened file: " << filename;

    infile.seek(0);

    /* Align kbytes, 1MB read at a time */
    static const int READ_BUF_SZ = 1024 * 1024;

    /* walk through the whole file and find all DLT0x01 markers */
    /* store the found positions in the indexAll */
    char lastFound = 0;

    while(true) {

        /* read buffer from file */
        buf = infile.read(READ_BUF_SZ);
        if(buf.isEmpty())
            break; // EOF

        /* Use primitive buffer for faster access */
        int cbuf_sz = buf.size();
        const char *cbuf = buf.constData();

        /* find marker in buffer */
        for(int num=0;num<cbuf_sz;num++) {
            if(cbuf[num] == 'D')
            {
                lastFound = 'D';
            }
            else if(lastFound == 'D' && cbuf[num] == 'L')
            {
                lastFound = 'L';
            }
            else if(lastFound == 'L' && cbuf[num] == 'T')
            {
                lastFound = 'T';
            }
            else if(lastFound == 'T' && cbuf[num] == 0x01)
            {
                indexAll.append(pos+num-3);
                lastFound = 0;
                if(indexAll.size()%10000 ==0)
                {
                    emit updateProgressText(QString("Parsing DLT file...found messages %1").arg(indexAll.size()));
                }
            }
            else
            {
                lastFound = 0;
            }
        }
        pos += cbuf_sz;
    }

    qDebug() << "Finished thread " << currentThreadId() << " and found messages: " << indexAll.size();
}
void ThreadDltIndex::setFilename(QString &_filename){
    filename = _filename;
}

QList<unsigned long> ThreadDltIndex::getIndexAll(){
    return indexAll;
}

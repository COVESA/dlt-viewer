#include <QFile>
#include <QDebug>
#include <QtEndian>
#include <QString>

extern "C" {
    #include "dlt_common.h"
}

#include "qdltmsg.h"
#include "qdltimporter.h"

#include <time.h>

QDltImporter::QDltImporter(QFile *outputfile, QStringList fileNames, QObject *parent) :
    QThread(parent)
{
    this->outputfile = outputfile;
    this->fileNames = fileNames;
    setPcapPorts(QString("3490 3489 49362"));
}

QDltImporter::QDltImporter(QFile *outputfile, QString fileName,QObject *parent) :
                                                                                         QThread(parent)
{
    this->outputfile = outputfile;
    fileNames.append(fileName);
    setPcapPorts(QString("3490 3489 49362"));
}

QDltImporter::~QDltImporter()
{

}

void QDltImporter::run()
{
    QString result;
    for ( const auto& i : fileNames )
    {
        if(i.endsWith(".mf4",Qt::CaseInsensitive))
            dltIpcFromMF4(i);
        else if (i.endsWith(".pcap",Qt::CaseInsensitive))
            dltIpcFromPCAP(i);
    }
    emit resultReady(result);
}

void QDltImporter::dltIpcFromPCAP(QString fileName)
{
    counterRecords = 0;
    counterRecordsDLT = 0;
    counterRecordsIPC = 0;
    counterDLTMessages = 0;
    counterIPCMessages = 0;

    QFile inputfile(fileName);

    if(!inputfile.open(QFile::ReadOnly))
       return;

    /* open output file */
    if(!outputfile->open(QIODevice::WriteOnly|QIODevice::Append))
    {
        qDebug() << "Failed opening WriteOnly" << outputfile->fileName();
    }

    int progressCounter = 1;
    emit progress("PCAP",1,0);

    pcap_hdr_t globalHeader;
    pcaprec_hdr_t recordHeader;

    qDebug() << "Import DLT/IPC from PCAP file:" << fileName;

    if(inputfile.read((char*)&globalHeader,sizeof(pcap_hdr_t))!=sizeof(pcap_hdr_t))
    {
        inputfile.close();
        outputfile->close();
        qDebug() << "fromPCAP:" << "Cannot open file" << fileName;
        return;
    }
    quint64 fileSize = inputfile.size();
    while(inputfile.read((char*)&recordHeader,sizeof(pcaprec_hdr_t))==sizeof(pcaprec_hdr_t))
    {
        int percent = inputfile.pos()*100/fileSize;
        if(percent>=progressCounter)
        {
            progressCounter += 1;
            emit progress("PCAP:",2,percent); // every 1%
            if((percent>0) && ((percent%10)==0))
                qDebug() << "Import PCAP:" << percent << "%"; // every 10%
        }

        // TODO: Handle cancel request

        QByteArray record = inputfile.read(recordHeader.incl_len);
         if(record.length() != recordHeader.incl_len)
         {
             inputfile.close();
             outputfile->close();
             qDebug() << "fromPCAP: PCAP file not complete!";
             qDebug() << "fromPCAP:" << "Size Error: Cannot read Record";
             return;
         }
         counterRecords ++;
         quint64 pos = 12;
         //Read EtherType
         if(record.size()<(qsizetype)(pos+2))
         {
             inputfile.close();
             outputfile->close();
             qDebug() << "dltFromPCAP:" << "Size Error: Cannot read Record";
             return;
         }
         quint16 etherType = (((quint16)record.at(pos))<<8)|((quint16)(record.at(pos+1)&0xff));
         pos+=2;
         if(!dltFromEthernetFrame(record,pos,etherType,recordHeader.ts_sec,recordHeader.ts_usec))
         {
             inputfile.close();
             outputfile->close();
             qDebug() << "fromPCAP:" << "Size Error: Cannot read Ethernet Frame";
             return;
         }
         if(!ipcFromEthernetFrame(record,pos,etherType,recordHeader.ts_sec,recordHeader.ts_usec))
         {
             inputfile.close();
             outputfile->close();
             qDebug() << "fromPCAP:" << "Size Error: Cannot read Ethernet Frame";
             return;
         }
    }
    inputfile.close();
    outputfile->close();

    emit progress("",3,100);

    qDebug() << "fromPCAP: Counter Records:" << counterRecords;
    qDebug() << "fromPCAP: Counter Records DLT:" << counterRecordsDLT;
    qDebug() << "fromPCAP: Counter DLT Mesages:" << counterDLTMessages;
    qDebug() << "fromPCAP: Counter Records IPC:" << counterRecordsIPC;
    qDebug() << "fromPCAP: Counter IPC Mesages:" << counterIPCMessages;

    qDebug() << "fromPCAP: Import finished";
}

void QDltImporter::dltIpcFromMF4(QString fileName)
{
    counterRecords = 0;
    counterRecordsDLT = 0;
    counterRecordsIPC = 0;
    counterDLTMessages = 0;
    counterIPCMessages = 0;
    channelGroupLength.clear();
    channelGroupName.clear();

    QFile inputfile(fileName);

    if(!inputfile.open(QFile::ReadOnly))
    {
       qDebug() << "fromMF4:" << "Cannot open file" << fileName;
       return;
    }

    /* open output file */
    if(!outputfile->open(QIODevice::WriteOnly|QIODevice::Append))
    {
        qDebug() << "Failed opening WriteOnly" << outputfile->fileName();
    }

    int progressCounter = 1;
    emit progress("MF4",1,0);

    qDebug() << "Import DLT/IPC from MF4 file:" << fileName;

    if(inputfile.read((char*)&mdfIdblock,sizeof(mdf_idblock_t))!=sizeof(mdf_idblock_t))
    {
        inputfile.close();
        outputfile->close();
        qDebug() << "fromMF4:" << "Size Error: Cannot reard Id Block";
        return;
    }

    mdf_hdr_t mdfHeader,mdfDgHeader,mdfCgHeader,mdfCnHeader,mdfTxHeader;
    mdf_dgblocklinks_t mdfDgBlockLinks = {};
    memset((char*)&mdfHeader,0,sizeof(mdf_hdr_t));

    if(inputfile.read((char*)&mdfHeader,sizeof(mdf_hdr_t))!=sizeof(mdf_hdr_t))
    {
        inputfile.close();
        outputfile->close();
        qDebug() << "fromMF4:" << "Size Error: Cannot read mdf header";
        return;
    }
    if(mdfHeader.id[0]=='#' && mdfHeader.id[1]=='#' && mdfHeader.id[2]=='H' && mdfHeader.id[3]=='D')
    {
        if(inputfile.read((char*)&hdBlockLinks,sizeof(mdf_hdblocklinks_t))!=sizeof(mdf_hdblocklinks_t))
        {
            inputfile.close();
            outputfile->close();
            qDebug() << "fromMF4:" << "Size Error: Cannot read HD Block";
            return;
        }
        // Iterate through all data groups
        quint64 ptrDg = hdBlockLinks.hd_dg_first;
        while(ptrDg)
        {
            inputfile.seek(ptrDg);
            if(inputfile.read((char*)&mdfDgHeader,sizeof(mdf_hdr_t))!=sizeof(mdf_hdr_t))
            {
                inputfile.close();
                outputfile->close();
                qDebug() << "fromMF4:" << "Size Error: Cannot reard DG Block";
                return;
            }
            if(mdfDgHeader.id[0]=='#' && mdfDgHeader.id[1]=='#' && mdfDgHeader.id[2]=='D' && mdfDgHeader.id[3]=='G')
            {
                //qDebug() << "\tDG:";
                if(inputfile.read((char*)&mdfDgBlockLinks,sizeof(mdf_dgblocklinks_t))!=sizeof(mdf_dgblocklinks_t))
                {
                    inputfile.close();
                    outputfile->close();
                    qDebug() << "fromMF4:" << "Size Error: Cannot reard DG Block";
                    return;
                }
                ptrDg=mdfDgBlockLinks.dg_dg_next;
                // Iterate through all channel groups
                quint64 ptrCg = mdfDgBlockLinks.dg_cg_first;
                while(ptrCg)
                {
                    inputfile.seek(ptrCg);
                    if(inputfile.read((char*)&mdfCgHeader,sizeof(mdf_hdr_t))!=sizeof(mdf_hdr_t))
                    {
                        inputfile.close();
                        outputfile->close();
                        qDebug() << "fromMF4:" << "Size Error: Cannot reard CG Block";
                        return;
                    }
                    if(mdfCgHeader.id[0]=='#' && mdfCgHeader.id[1]=='#' && mdfCgHeader.id[2]=='C' && mdfCgHeader.id[3]=='G')
                    {
                        //qDebug() << "\t\tCG:";
                        mdf_cgblocklinks_t mdfCgBlockLinks;
                        if(inputfile.read((char*)&mdfCgBlockLinks,sizeof(mdf_cgblocklinks_t))!=sizeof(mdf_cgblocklinks_t))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot reard CG Block";
                            return;
                        }
                        //qDebug() << "\t\cg_record_id =" << mdfCgBlockLinks.cg_record_id;
                        //qDebug() << "\t\cg_data_bytes =" << mdfCgBlockLinks.cg_data_bytes;
                        ptrCg=mdfCgBlockLinks.cg_cg_next;
                        if(mdfCgBlockLinks.cg_flags&1) // VLSD
                            channelGroupLength[mdfCgBlockLinks.cg_record_id]=-1;
                        else
                            channelGroupLength[mdfCgBlockLinks.cg_record_id]=mdfCgBlockLinks.cg_data_bytes;
                        // Iterate through all channels
                        quint64 ptrCh = mdfCgBlockLinks.cg_cn_first;
                        while(ptrCh)
                        {
                            inputfile.seek(ptrCh);
                            if(inputfile.read((char*)&mdfCnHeader,sizeof(mdf_hdr_t))!=sizeof(mdf_hdr_t))
                            {
                                qDebug() << "fromMF4:" << "Size Error: Cannot reard CN Block";
                                inputfile.close();
                                outputfile->close();
                                return;
                            }
                            if(mdfCnHeader.id[0]=='#' && mdfCnHeader.id[1]=='#' && mdfCnHeader.id[2]=='C' && mdfCnHeader.id[3]=='N')
                            {
                                //qDebug() << "\t\t\tCN:";
                                mdf_cnblocklinks_t mdfChBlockLinks;
                                if(inputfile.read((char*)&mdfChBlockLinks,sizeof(mdf_cnblocklinks_t))!=sizeof(mdf_cnblocklinks_t))
                                {
                                    inputfile.close();
                                    outputfile->close();
                                    qDebug() << "fromMF4:" << "Size Error: Cannot reard CN Block";
                                    return;
                                }
                                ptrCh=mdfChBlockLinks.cn_cn_next;
                                // Read channel name
                                inputfile.seek(mdfChBlockLinks.cn_tx_name);
                                if(inputfile.read((char*)&mdfTxHeader,sizeof(mdf_hdr_t))!=sizeof(mdf_hdr_t))
                                {
                                    inputfile.close();
                                    outputfile->close();
                                    qDebug() << "fromMF4:" << "Size Error: Cannot reard Tx Block";
                                    return;
                                }
                                char cnName[256];
                                memset(cnName,0,256);

                                const auto cnNameLength = mdfTxHeader.length-sizeof(mdf_hdr_t);
                                if(cnNameLength < 256) {
                                    const quint64 cnNameReadLength = inputfile.read((char*)cnName, cnNameLength);
                                    if(cnNameReadLength != cnNameLength )
                                    {
                                        inputfile.close();
                                        outputfile->close();
                                        qDebug() << "fromMF4:" << "Size Error: Cannot read cn name";
                                        return;
                                    }
                                }
                                // FIXME: this is probably a bug if this line is reached because cnNameLength >= 256, since cnName is 0-initialized 256-bytes array
                                channelGroupName[mdfCgBlockLinks.cg_record_id] = QString(cnName);
                                //qDebug() << "fromMF4: cnName=" << cnName;

                            }
                            else
                                ptrCh=0;
                        }
                    }
                    else
                        ptrCg=0;
                }
            }
            else
                ptrDg=0;
        }
    }
    // seek to and read data list header
    inputfile.seek(mdfDgBlockLinks.dg_data);
    if(inputfile.read((char*)&mdfHeader,sizeof(mdf_hdr_t))!=sizeof(mdf_hdr_t))
    {
        inputfile.close();
        outputfile->close();
        qDebug() << "fromMF4: Cannot read datalist header";
        return;
    }
    int numberOfLinks = 0;
    bool isDataBlock = false;
    if(mdfHeader.id[0]=='#' && mdfHeader.id[1]=='#' && mdfHeader.id[2]=='D' && mdfHeader.id[3]=='L')
    {
        // Datalist detected, get number of datablocks
        numberOfLinks = mdfHeader.link_count;
        qDebug() << "fromMF4: Datalist detected";
    }
    else if(mdfHeader.id[0]=='#' && mdfHeader.id[1]=='#' && mdfHeader.id[2]=='D' && mdfHeader.id[3]=='T')
    {
        // Only one Datalblock detected
        numberOfLinks = 2;
        isDataBlock = true;
        qDebug() << "fromMF4: Datablock detected";
    }
    else
    {
        inputfile.close();
        outputfile->close();
        qDebug() << "fromMF4: Cannot find Datalist or Datablock";
        return;
    }
    for(int num=1;num<numberOfLinks;num++)
    {
        if(!isDataBlock)
        {
            quint64 addressOfDataBlock;
            inputfile.seek(mdfDgBlockLinks.dg_data+sizeof(mdf_hdr_t)+num*sizeof(quint64));
            if(inputfile.read((char*)&addressOfDataBlock,sizeof(quint64))!=sizeof(quint64))
            {
                inputfile.close();
                outputfile->close();
                qDebug() << "fromMF4: Cannot read datablock address";
                return;
            }
            inputfile.seek(addressOfDataBlock);
            if(inputfile.read((char*)&mdfHeader,sizeof(mdf_hdr_t))!=sizeof(mdf_hdr_t))
            {
                inputfile.close();
                outputfile->close();
                qDebug() << "fromMF4: Cannot read datablock header";
                return;
            }
        }
        if(mdfHeader.id[0]=='#' && mdfHeader.id[1]=='#' && mdfHeader.id[2]=='D' && mdfHeader.id[3]=='T')
        {
            const auto pos = inputfile.pos() - sizeof(mdf_hdr_t);
            quint64 posDt=0;
            quint16 recordId;
            quint32 lengthVLSD;
            mdf_ethFrame ethFrame;
            mdf_plpRaw_t plpRaw;
            mdf_dltFrame_t dltFrameBlock;
            QByteArray recordData;
            quint64 fileSize = inputfile.size();
            while(posDt<(mdfHeader.length-sizeof(mdf_hdr_t)))
            {
                int percent = inputfile.pos()*100/fileSize;
                if(percent>=progressCounter)
                {
                    progressCounter += 1;
                    emit progress("MF4:",2,percent); // every 1%
                    if((percent>0) && ((percent%10)==0))
                    {
                        qDebug() << "Import MF4:" << percent << "%"; // every 10%
                    }
                }
                //qDebug() << "Record:" << counterRecords << pos << posDt;

                // TODO: Handle cancel operation

                //qDebug() << "posDt =" << posDt;
                inputfile.seek(pos+sizeof(mdf_hdr_t)+posDt);
                counterRecords++;
                if(inputfile.read((char*)&recordId,sizeof(quint16))!=sizeof(quint16))
                {
                    inputfile.close();
                    outputfile->close();
                    qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                    return;
                }
                posDt += sizeof(quint16);
                if(channelGroupLength.contains(recordId))
                {
                    QString name;
                    if(channelGroupName.contains(recordId))
                        name = channelGroupName[recordId];
                    if(channelGroupLength[recordId]==-1)
                    {
                        if(inputfile.read((char*)&lengthVLSD,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        posDt += sizeof(quint32);
                        posDt += lengthVLSD;
                        recordData = inputfile.read(lengthVLSD);
                        //qDebug() << "recordId =" << recordId << "length =" << lengthVLSD;
                    }
                    else if(channelGroupLength[recordId]==43)
                    {
                        // Ethernet Group
                        if(inputfile.read((char*)&ethFrame.timeStamp,sizeof(quint64))!=sizeof(quint64))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.asynchronous,sizeof(quint8))!=sizeof(quint8))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.source,6)!=6)
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.destination,6)!=6)
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.etherType,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.crc,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.receivedDataByteCount,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.dataLength,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.dataBytes,sizeof(quint64))!=sizeof(quint64))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(!recordData.isEmpty())
                        {
                            quint64 time = hdBlockLinks.start_time_ns+ethFrame.timeStamp;
                            if(!dltFromEthernetFrame(recordData,0,ethFrame.etherType,time/1000000000ul,time%1000000000ul/1000ul))
                            {
                                inputfile.close();
                                outputfile->close();
                                qDebug() << "fromMF4: ERROR:" << "Size Error: Cannot read Ethernet Frame";
                                return;
                            }
                            if(!ipcFromEthernetFrame(recordData,0,ethFrame.etherType,time/1000000000ul,time%1000000000ul/1000ul))
                            {
                                inputfile.close();
                                outputfile->close();
                                qDebug() << "fromMF4: ERROR:" << "Size Error: Cannot read Ethernet Frame";
                                return;
                            }
                            recordData.clear();
                        }
                        posDt += channelGroupLength[recordId];
                    }
                    else if(channelGroupLength[recordId]==51)
                    {
                        // Ethernet Group
                        if(inputfile.read((char*)&ethFrame.timeStamp,sizeof(quint64))!=sizeof(quint64))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.asynchronous,sizeof(quint8))!=sizeof(quint8))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.source,6)!=6)
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.destination,6)!=6)
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.etherType,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.crc,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.receivedDataByteCount,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.beaconTimeStamp,sizeof(quint64))!=sizeof(quint64)) // TODO: Beacon Time Stamp
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.dataLength,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.dataBytes,sizeof(quint64))!=sizeof(quint64))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(!recordData.isEmpty())
                        {
                            int pos = 0;
                            quint64 time = hdBlockLinks.start_time_ns+ethFrame.timeStamp;
                            if(!dltFromEthernetFrame(recordData,pos,ethFrame.etherType,time/1000000000ul,time%1000000000ul/1000ul))
                            {
                                inputfile.close();
                                outputfile->close();
                                qDebug() << "fromMF4: ERROR:" << "Size Error: Cannot read Ethernet Frame";
                                return;
                            }
                            pos = 0;
                            if(!ipcFromEthernetFrame(recordData,pos,ethFrame.etherType,time/1000000000ul,time%1000000000ul/1000ul))
                            {
                                inputfile.close();
                                outputfile->close();
                                qDebug() << "fromMF4: ERROR:" << "Size Error: Cannot read Ethernet Frame";
                                return;
                            }
                            recordData.clear();
                        }
                        posDt += channelGroupLength[recordId];
                    }
                    else if(channelGroupLength[recordId]==29 && name=="DLT_Frame")
                    {
                        // DLT Frame
                        if(inputfile.read((char*)&dltFrameBlock.timeStamp,sizeof(quint64))!=sizeof(quint64))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&dltFrameBlock.asynchronous,sizeof(quint8))!=sizeof(quint8))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&dltFrameBlock.currentFragmentNumber,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&dltFrameBlock.lastFragmentNumber,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&dltFrameBlock.ecuId,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&dltFrameBlock.dataLength,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&dltFrameBlock.dataBytes,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(!recordData.isEmpty())
                        {
                            int pos = 0;
                            quint64 time = hdBlockLinks.start_time_ns+dltFrameBlock.timeStamp;
                            if(!dltFrame(recordData,pos,time/1000000000ul,time%1000000000ul/1000ul))
                            {
                                inputfile.close();
                                outputfile->close();
                                qDebug() << "fromMF4: ERROR:" << "Size Error: Cannot read DLTFrame";
                                return;
                            }
                            recordData.clear();
                        }
                        posDt += channelGroupLength[recordId];
                    }
                    else if(channelGroupLength[recordId]==29)
                    {
                        // PLP Raw
                        if(inputfile.read((char*)&plpRaw.timeStamp,sizeof(quint64))!=sizeof(quint64))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.asynchronous,sizeof(quint8))!=sizeof(quint8))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.probeId,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.msgType,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.probeFlags,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.dataFlags,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.dataCounter,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.dataLength,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.dataBytes,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            outputfile->close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(!recordData.isEmpty())
                        {
                            quint64 time = hdBlockLinks.start_time_ns+plpRaw.timeStamp;
                            if(!ipcFromPlpRaw(&plpRaw,recordData,time/1000000000ul,time%1000000000ul/1000ul))
                            {
                                inputfile.close();
                                outputfile->close();
                                qDebug() << "fromMF4: ERROR:" << "Size Error: Cannot read Ethernet Frame";
                                return;
                            }
                            recordData.clear();
                        }
                        posDt += channelGroupLength[recordId];
                    }
                    else
                    {
                        qDebug() << "fromMF4: ERROR: Unknown recordId =" << recordId << "Length =" << channelGroupLength[recordId];
                        qDebug() << "fromMF4: But try to continue read file.";
                        posDt += channelGroupLength[recordId];
                    }
                }
                else
                {
                    qDebug() << "fromMF4: ERROR: Unknown recordId =" << recordId;
                    break;
                }
            }
        }
    }

    inputfile.close();
    outputfile->close();

    emit progress("",3,100);

    qDebug() << "fromMF4: counterRecords:" << counterRecords;
    qDebug() << "fromMF4: counterRecordsDLT:" << counterRecordsDLT;
    qDebug() << "fromMF4: counterDLTMessages:" << counterDLTMessages;
    qDebug() << "fromMF4: counterRecordsIPC:" << counterRecordsIPC;
    qDebug() << "fromMF4: counterIPCMessages:" << counterIPCMessages;

    qDebug() << "fromMF4: Import finished";
}

DltStorageHeader QDltImporter::makeDltStorageHeader(std::optional<DltStorageHeaderTimestamp> ts)
{
    DltStorageHeader result;

    result.pattern[0] = 'D';
    result.pattern[1] = 'L';
    result.pattern[2] = 'T';
    result.pattern[3] = 0x01;

    result.ecu[0] = 0;
    result.ecu[1] = 0;
    result.ecu[2] = 0;
    result.ecu[3] = 0;

    if (ts) {
        result.seconds = static_cast<time_t>(ts->sec);
        result.microseconds = static_cast<int32_t>(ts->usec);
    } else {
        if (struct timespec ts; timespec_get(&ts, TIME_UTC)) {
            result.seconds = static_cast<uint32_t>(ts.tv_sec);
            result.microseconds = static_cast<int32_t>(ts.tv_nsec / 1000);
        } else {
            result.seconds = 0;
            result.microseconds = 0;
        }
    }

    return result;
}

bool QDltImporter::ipcFromEthernetFrame(QByteArray &record,int pos,quint16 etherType,quint32 sec,quint32 usec)
{
    if(etherType==0x9100 || etherType==0x88a8)
    {
        // VLAN tagging used
        pos+=2;
        if(record.size()<(pos+2))
        {
            qDebug() << "Size issue!";
            return false;
        }
        etherType = (((quint16)record.at(pos))<<8)|((quint16)(record.at(pos+1)&0xff));
        pos+=2;
    }
    if(etherType==0x8100)
    {
        // VLAN tagging used
        pos+=2;
        if(record.size()<(pos+2))
        {
            qDebug() << "ipcFromEthernetFrame: Size issue!";
            return false;
        }
        etherType = (((quint16)record.at(pos))<<8)|((quint16)(record.at(pos+1)&0xff));
        pos += 2;
    }
    if(etherType==0x2090) // PLP packet found
    {
       if(record.size()<(qsizetype)(pos+sizeof(plp_header_t)))
       {
           qDebug() << "ipcFromEthernetFrame: Size issue!";
           return false;
       }
       plp_header_t *plpHeader = (plp_header_t *) (record.data()+pos);

       pos += sizeof(plp_header_t);

       bool startOfSegment = qFromBigEndian(plpHeader->probeFlags) & 0x2;
       if(startOfSegment)
       {
           inSegment = true;
           segmentBuffer.clear();
       }
       bool endOfSegment = qFromBigEndian(plpHeader->probeFlags) & 0x1;
       if(endOfSegment)
       {
           inSegment = false;
       }
       //bool multiFrame = qFromBigEndian(plpHeader->probeFlags) & 0x8;
       if(qFromBigEndian(plpHeader->probeId) == 0xd0 && qFromBigEndian(plpHeader->msgType) == 0x500)
       {
           counterRecordsIPC++;
           while(record.size()>=(qsizetype)(pos+sizeof(plp_header_data_t)))
           {
               plp_header_data_t *plpHeaderData = (plp_header_data_t *) (record.data()+pos);

               pos += sizeof(plp_header_data_t);

               if(record.size()<(pos+qFromBigEndian(plpHeaderData->length)))
               {
                   qDebug() << "ipcFromEthernetFrame: Size issue!";
                   break;
               }
               counterIPCMessages++;

               if(inSegment || endOfSegment)
               {
                   segmentBuffer += record.mid(pos,qFromBigEndian(plpHeaderData->length));
               }
               if(!inSegment || endOfSegment)
               {
                   /* now write DLT message here */
                   QByteArray empty,payload;
                   QDltMsg msg;

                   // set parameters of DLT message to be generated
                   msg.clear();
                   msg.setEcuid("IPNP");
                   msg.setApid("IPC");
                   msg.setCtid("IPC");
                   msg.setMode(QDltMsg::DltModeVerbose);
                   msg.setType(QDltMsg::DltTypeLog);
                   msg.setSubtype(QDltMsg::DltLogInfo);
                   msg.setMessageCounter(0);
                   msg.setNumberOfArguments(3);

                   // add PLP Header Data
                   QDltArgument arg1;
                   arg1.setTypeInfo(QDltArgument::DltTypeInfoRawd);
                   arg1.setEndianness(QDlt::DltEndiannessLittleEndian);
                   arg1.setOffsetPayload(0);
                   arg1.setData(record.mid(pos-sizeof(plp_header_data_t),sizeof(plp_header_data_t)));
                   msg.addArgument(arg1);

                   // add IPC Header
                   QDltArgument arg2;
                   arg2.setTypeInfo(QDltArgument::DltTypeInfoRawd);
                   arg2.setEndianness(QDlt::DltEndiannessLittleEndian);
                   arg2.setOffsetPayload(0);
                   if(endOfSegment)
                   {
                       arg2.setData(segmentBuffer.mid(0,35));
                   }
                   else
                   {
                       arg2.setData(record.mid(pos,35));
                   }
                   msg.addArgument(arg2);

                   // add IPC Data
                   QDltArgument arg3;
                   arg3.setTypeInfo(QDltArgument::DltTypeInfoRawd);
                   arg3.setEndianness(QDlt::DltEndiannessLittleEndian);
                   arg3.setOffsetPayload(0);
                   if(endOfSegment)
                   {
                       arg3.setData(segmentBuffer.mid(35));
                       segmentBuffer.clear();
                   }
                   else
                   {
                       arg3.setData(record.mid(pos+35,qFromBigEndian(plpHeaderData->length)-35));
                   }
                   msg.addArgument(arg3);

                   // write DLT message
                   msg.getMsg(payload,false);
                   writeDLTMessageToFile(payload,0,0,0,sec,usec);
               }

               pos += qFromBigEndian(plpHeaderData->length);
           }
       }
    }

    return true;
}

bool QDltImporter::ipcFromPlpRaw(mdf_plpRaw_t *plpRaw,QByteArray &record,quint32 sec,quint32 usec)
{
       bool startOfSegment = plpRaw->probeFlags & 0x2;
       if(startOfSegment)
       {
           inSegment = true;
           segmentBuffer.clear();
       }
       bool endOfSegment = plpRaw->probeFlags & 0x1;
       if(endOfSegment)
       {
           inSegment = false;
       }
       //bool multiFrame = plpRaw->probeFlags & 0x8;
       if(plpRaw->probeId == 0xd0 && plpRaw->msgType == 0x500)
       {
           counterRecordsIPC++;
               counterIPCMessages++;

               if(inSegment || endOfSegment)
               {
                   segmentBuffer += record;
               }
               if(!inSegment || endOfSegment)
               {
                   /* now write DLT message here */
                   QByteArray empty,payload;
                   QDltMsg msg;

                   // set parameters of DLT message to be generated
                   msg.clear();
                   msg.setEcuid("IPNP");
                   msg.setApid("IPC");
                   msg.setCtid("IPC");
                   msg.setMode(QDltMsg::DltModeVerbose);
                   msg.setType(QDltMsg::DltTypeLog);
                   msg.setSubtype(QDltMsg::DltLogInfo);
                   msg.setMessageCounter(0);
                   msg.setNumberOfArguments(3);

                   // add PLP Header Data
                   QDltArgument arg1;
                   arg1.setTypeInfo(QDltArgument::DltTypeInfoRawd);
                   arg1.setEndianness(QDlt::DltEndiannessLittleEndian);
                   arg1.setOffsetPayload(0);
                   plp_header_data_t  plpHeaderData;
                   plpHeaderData.busSpecId=0;
                   plpHeaderData.dataFlags=plpRaw->dataFlags;
                   plpHeaderData.length=plpRaw->dataLength;
                   plpHeaderData.timeStampHigh=0;
                   plpHeaderData.timeStampLow=0;
                   arg1.setData(QByteArray((char*)&plpHeaderData,sizeof(plp_header_data_t)));
                   msg.addArgument(arg1);

                   // add IPC Header
                   QDltArgument arg2;
                   arg2.setTypeInfo(QDltArgument::DltTypeInfoRawd);
                   arg2.setEndianness(QDlt::DltEndiannessLittleEndian);
                   arg2.setOffsetPayload(0);
                   if(endOfSegment)
                   {
                       arg2.setData(segmentBuffer.mid(0,35));
                   }
                   else
                   {
                       arg2.setData(record.mid(0,35));
                   }
                   msg.addArgument(arg2);

                   // add IPC Data
                   QDltArgument arg3;
                   arg3.setTypeInfo(QDltArgument::DltTypeInfoRawd);
                   arg3.setEndianness(QDlt::DltEndiannessLittleEndian);
                   arg3.setOffsetPayload(0);
                   if(endOfSegment)
                   {
                       arg3.setData(segmentBuffer.mid(35));
                       segmentBuffer.clear();
                   }
                   else
                   {
                       arg3.setData(record.mid(35));
                   }
                   msg.addArgument(arg3);

                   // write DLT message
                   msg.getMsg(payload,false);
                   writeDLTMessageToFile(payload,0,0,0,sec,usec);
               }

       }

    return true;
}

bool QDltImporter::dltFrame(QByteArray &record,int pos,quint32 sec,quint32 usec)
{
    counterRecordsDLT++;
    // Now read the DLT Messages
    quint64 dataSize;
    dataSize = record.size()-pos;
    char* dataPtr = record.data()+pos;
    // Find one ore more DLT messages in the UDP message
    while(dataSize>0)
    {
        QDltMsg qmsg;
        quint64 sizeMsg = qmsg.checkMsgSize(dataPtr,dataSize);
        if(sizeMsg>0)
        {
            // DLT message found, write it with storage header
            QByteArray empty;
            writeDLTMessageToFile(empty,dataPtr,sizeMsg,0,sec,usec);
            counterDLTMessages++;

            //totalBytesRcvd+=sizeMsg;
            if(sizeMsg<=dataSize)
            {
                dataSize -= sizeMsg;
                dataPtr += sizeMsg;
            }
            else
            {
                dataSize = 0;
            }
        }
        else
        {
            dataSize = 0;
        }
    }

    return true;
}


bool QDltImporter::dltFromEthernetFrame(QByteArray &record,int pos,quint16 etherType,quint32 sec,quint32 usec)
{
    if(etherType==0x9100 || etherType==0x88a8)
    {
        // VLAN tagging used
        pos+=2;
        if(record.size()<(pos+2))
        {
            qDebug() << "Size issue!";
            return false;
        }
        etherType = (((quint16)record.at(pos))<<8)|((quint16)(record.at(pos+1)&0xff));
        pos+=2;
    }
    if(etherType==0x8100)
    {
        // VLAN tagging used
        pos+=2;
        if(record.size()<(pos+2))
        {
            qDebug() << "Size issue!";
            return false;
        }
        etherType = (((quint16)record.at(pos))<<8)|((quint16)(record.at(pos+1)&0xff));
        pos+=2;
    }
    if(etherType==0x2090) // PLP packet found
    {
        if(record.size()<(qsizetype)(pos+sizeof(plp_header_t)))
        {
            qDebug() << "dltFromEthernetFrame: Size issue!";
            return false;
        }
        plp_header_t *plpHeader = (plp_header_t *) (record.data()+pos);

        pos += sizeof(plp_header_t);

        if(/*qFromBigEndian(plpHeader->probeId) == 0x62 &&*/ qFromBigEndian(plpHeader->msgType) == 0x80)
        {
            plp_header_data_t *plpHeaderData = (plp_header_data_t *) (record.data()+pos);

            pos += sizeof(plp_header_data_t);

            if(record.size()<(pos+qFromBigEndian(plpHeaderData->length)))
            {
                qDebug() << "dltFromEthernetFrame: Size issue!";
                return false;
            }
            pos+=12;
            if(record.size()<(pos+2))
            {
                qDebug() << "dltFromEthernetFrame:" << "Size Error: Cannot read Ethernet frame in PLP";
                return false;
            }
            quint16 etherType2 = (((quint16)record.at(pos))<<8)|((quint16)(record.at(pos+1)&0xff));
            pos+=2;
            if(!dltFromEthernetFrame(record,pos,etherType2,sec,usec)) // TODO: Use time from PLP instead
            {
                qDebug() << "dltFromEthernetFrame:" << "Size Error: Cannot read Ethernet Frame";
                return false;
            }
        }
    }
    if(etherType==0x0800) // IP packet found
    {
       pos+=4;
       if(record.size()<(pos+2))
       {
           qDebug() << "Size issue!";
           return false;
       }
       //quint16 identification = (((quint16)record.at(pos))<<8)|((quint16)(record.at(pos+1)&0xff));
       pos+=2;
       if(record.size()<(pos+2))
       {
           qDebug() << "Size issue!";
           return false;
       }
       quint16 flagsOffset = (((quint16)record.at(pos))<<8)|((quint16)(record.at(pos+1)&0xff));
       quint16 flags =  flagsOffset >> 13; // TODO: Flags are in the wrong bit order
       quint16 offset =  flagsOffset & 0x1fff;
       pos+=3;
       if(record.size()<(pos+2))
       {
           qDebug() << "Size issue!";
           return false;
       }
       quint8 protocol = record.at(pos);
       if(protocol==0x11) // UDP packet found
       {
           pos+=11;
           if((flags==0 || flags==2) && offset==0)
           {
               // no fragmentation
               pos+=2;
               if(record.size()<(pos+2))
               {
                   qDebug() << "Size issue!";
                   return false;
               }
               quint16 destPort = (((quint16)record.at(pos))<<8)|((quint16)(record.at(pos+1)&0xff));
               if(pcapPorts.contains(destPort))
               {
                   pos+=6;
                   dltFrame(record,pos,sec,usec);
               }
           }
           else
           {
               //fragmentation
               if(flags==0)
               {
                   // last fragment
                   segmentBufferUDP += QByteArray(record.data()+pos,record.size()-pos);

                   pos=2;
                   if(segmentBufferUDP.size()<(pos+2))
                   {
                       qDebug() << "Size issue!";
                       return false;
                   }
                   quint16 destPort = (((quint16)segmentBufferUDP.at(pos))<<8)|((quint16)(segmentBufferUDP.at(pos+1)&0xff));
                   if(pcapPorts.contains(destPort))
                   {
                       pos+=6;
                       dltFrame(segmentBufferUDP,pos,sec,usec);
                   }
                   segmentBufferUDP.clear();
               }
               else
               {
                   // first or further fragment
                   segmentBufferUDP += QByteArray(record.data()+pos,record.size()-pos);
               }
           }
       }
    }
    return true;
}

void QDltImporter::writeDLTMessageToFile(QByteArray &bufferHeader,char* bufferPayload,quint32 bufferPayloadSize,QString ecuId,quint32 sec,quint32 usec)
{
    const auto timestamp =
        (sec || usec) ? std::optional<DltStorageHeaderTimestamp>({sec, usec}) : std::nullopt;
    DltStorageHeader str = makeDltStorageHeader(timestamp);

    dlt_set_id(str.ecu, ecuId.toLatin1());

    outputfile->write((char*)&str,sizeof(DltStorageHeader));
    outputfile->write(bufferHeader);
    outputfile->write(bufferPayload,bufferPayloadSize);
}

void QDltImporter::setPcapPorts(const QString &importPcapPorts)
{
    pcapPorts.clear();
    QStringList portList = importPcapPorts.split(' ');

    for (const QString& item : portList) {
        if(item.toUShort()!=0)
            pcapPorts.append(item.toUShort());
    }
}


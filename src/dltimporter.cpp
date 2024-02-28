#include <QFile>
#include <QProgressDialog>
#include <QLabel>
#include <QDebug>
#include <QtEndian>

/**
 * From QDlt.
 * Must be a "C" include to interpret the imports correctly
 * for MSVC compilers.
 **/
#include "dlt_common.h"
extern "C" {

    #include "dlt_user.h"
}

#if defined(_MSC_VER)
#include <io.h>
#include <time.h>
#include <WinSock.h>
#else
#include <unistd.h>     /* for read(), close() */
#include <sys/time.h>	/* for gettimeofday() */
#endif

#include "dltimporter.h"

DltImporter::DltImporter()
{

}

DltImporter::~DltImporter()
{

}

void DltImporter::dltIpcFromPCAP(QFile &outputfile,QString fileName,QWidget *parent,bool silent)
{
    counterRecords = 0;
    counterRecordsDLT = 0;
    counterRecordsIPC = 0;
    counterDLTMessages = 0;
    counterIPCMessages = 0;

    QFile inputfile(fileName);

    if(!inputfile.open(QFile::ReadOnly))
       return;

    QProgressDialog progress("Import DLT/IPC from PCAP...", "Abort Import", 0, inputfile.size()/1000, parent);
    QLabel label(&progress);
    if(!silent)
    {
        progress.setWindowTitle("Import DLT/IPC from PCAP");
        progress.setWindowModality(Qt::WindowModal);
        progress.setLabel(&label);
    }
    quint64 progressCounter = 0;

    pcap_hdr_t globalHeader;
    pcaprec_hdr_t recordHeader;

    QDltMsg qmsg;

    qDebug() << "Import DLT/IPC from PCAP file:" << fileName;

    if(inputfile.read((char*)&globalHeader,sizeof(pcap_hdr_t))!=sizeof(pcap_hdr_t))
    {
        inputfile.close();
        qDebug() << "fromPCAP:" << "Cannot open file" << fileName;
        return;
    }
    while(inputfile.read((char*)&recordHeader,sizeof(pcaprec_hdr_t))==sizeof(pcaprec_hdr_t))
    {
        progressCounter++;
        if(!silent && progressCounter%1000==0)
        {
            progress.setValue(inputfile.pos()/1000);
            label.setText(QString("Imported DLT %1 IPC %2").arg(counterDLTMessages).arg(counterIPCMessages));
        }

        if (progress.wasCanceled())
        {
            qDebug() << "fromPCAP:" << "Import Stopped";
            break;
        }

        QByteArray record = inputfile.read(recordHeader.incl_len);
         if(record.length() != recordHeader.incl_len)
         {
             inputfile.close();
             qDebug() << "fromPCAP: PCAP file not complete!";
             qDebug() << "fromPCAP:" << "Size Error: Cannot read Record";
             return;
         }
         counterRecords ++;
         quint64 pos = 12;
         //Read EtherType
         if(record.size()<(pos+2))
         {
             inputfile.close();
             qDebug() << "dltFromPCAP:" << "Size Error: Cannot read Record";
             return;
         }
         quint16 etherType = (((quint16)record.at(pos))<<8)|((quint16)(record.at(pos+1)&0xff));
         pos+=2;
         if(!dltFromEthernetFrame(outputfile,record,pos,etherType,recordHeader.ts_sec,recordHeader.ts_usec))
         {
             inputfile.close();
             qDebug() << "fromPCAP:" << "Size Error: Cannot read Ethernet Frame";
             return;
         }
         if(!ipcFromEthernetFrame(outputfile,record,pos,etherType,recordHeader.ts_sec,recordHeader.ts_usec))
         {
             inputfile.close();
             qDebug() << "fromPCAP:" << "Size Error: Cannot read Ethernet Frame";
             return;
         }
    }
    inputfile.close();

    qDebug() << "fromPCAP: Counter Records:" << counterRecords;
    qDebug() << "fromPCAP: Counter Records DLT:" << counterRecordsDLT;
    qDebug() << "fromPCAP: Counter DLT Mesages:" << counterDLTMessages;
    qDebug() << "fromPCAP: Counter Records IPC:" << counterRecordsIPC;
    qDebug() << "fromPCAP: Counter IPC Mesages:" << counterIPCMessages;

    qDebug() << "fromPCAP: Import finished";

}

void DltImporter::dltIpcFromMF4(QFile &outputfile,QString fileName,QWidget *parent,bool silent)
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

    QProgressDialog progress("Import DLT/IPC from MF4...", "Abort Import", 0, inputfile.size()/1000, parent);
    QLabel label(&progress);
    if(!silent)
    {
        progress.setWindowTitle("Import DLT/IPC from MF4");
        progress.setWindowModality(Qt::WindowModal);
        progress.setLabel(&label);
    }
    quint64 progressCounter = 0;

    qDebug() << "Import DLT/IPC from MF4 file:" << fileName;

    if(inputfile.read((char*)&mdfIdblock,sizeof(mdf_idblock_t))!=sizeof(mdf_idblock_t))
    {
        inputfile.close();
        qDebug() << "fromMF4:" << "Size Error: Cannot reard Id Block";
        return;
    }

    mdf_hdr_t mdfHeader,mdfDgHeader,mdfCgHeader,mdfCnHeader,mdfTxHeader;
    memset((char*)&mdfHeader,0,sizeof(mdf_hdr_t));
    quint64 pos,posDg;

    while(inputfile.read((char*)&mdfHeader,sizeof(mdf_hdr_t))==sizeof(mdf_hdr_t))
    {
        pos = inputfile.pos() - sizeof(mdf_hdr_t);
        if(mdfHeader.id[0]=='#' && mdfHeader.id[1]=='#' && mdfHeader.id[2]=='H' && mdfHeader.id[3]=='D')
        {
            //qDebug() << "HD:";
            if(inputfile.read((char*)&hdBlockLinks,sizeof(mdf_hdblocklinks_t))!=sizeof(mdf_hdblocklinks_t))
            {
                inputfile.close();
                qDebug() << "fromMF4:" << "Size Error: Cannot reard HD Block";
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
                    qDebug() << "fromMF4:" << "Size Error: Cannot reard DG Block";
                    return;
                }
                if(mdfDgHeader.id[0]=='#' && mdfDgHeader.id[1]=='#' && mdfDgHeader.id[2]=='D' && mdfDgHeader.id[3]=='G')
                {
                    //qDebug() << "\tDG:";
                    mdf_dgblocklinks_t mdfDgBlockLinks;
                    if(inputfile.read((char*)&mdfDgBlockLinks,sizeof(mdf_dgblocklinks_t))!=sizeof(mdf_dgblocklinks_t))
                    {
                        inputfile.close();
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
                                    return;
                                }
                                if(mdfCnHeader.id[0]=='#' && mdfCnHeader.id[1]=='#' && mdfCnHeader.id[2]=='C' && mdfCnHeader.id[3]=='N')
                                {
                                    //qDebug() << "\t\t\tCN:";
                                    mdf_cnblocklinks_t mdfChBlockLinks;
                                    if(inputfile.read((char*)&mdfChBlockLinks,sizeof(mdf_cnblocklinks_t))!=sizeof(mdf_cnblocklinks_t))
                                    {
                                        inputfile.close();
                                        qDebug() << "fromMF4:" << "Size Error: Cannot reard CN Block";
                                        return;
                                    }
                                    ptrCh=mdfChBlockLinks.cn_cn_next;
                                    // Read channel name
                                    inputfile.seek(mdfChBlockLinks.cn_tx_name);
                                    if(inputfile.read((char*)&mdfTxHeader,sizeof(mdf_hdr_t))!=sizeof(mdf_hdr_t))
                                    {
                                        inputfile.close();
                                        qDebug() << "fromMF4:" << "Size Error: Cannot reard Tx Block";
                                        return;
                                    }
                                    char cnName[256];
                                    memset(cnName,0,256);
                                    if((mdfTxHeader.length-sizeof(mdf_hdr_t))<256)
                                    if(inputfile.read((char*)cnName,mdfTxHeader.length-sizeof(mdf_hdr_t)) != (mdfTxHeader.length-sizeof(mdf_hdr_t)) )
                                    {
                                        inputfile.close();
                                        qDebug() << "fromMF4:" << "Size Error: Cannot reard cn name";
                                        return;
                                    }
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
        if(mdfHeader.id[0]=='#' && mdfHeader.id[1]=='#' && mdfHeader.id[2]=='D' && mdfHeader.id[3]=='T')
        {
            //qDebug() << "DT:";
            int posDt=0;
            quint16 recordId;
            quint32 lengthVLSD;
            mdf_ethFrame ethFrame;
            mdf_plpRaw_t plpRaw;
            mdf_dltFrame_t dltFrameBlock;
            QByteArray recordData;
            while(posDt<(mdfHeader.length-sizeof(mdf_hdr_t)))
            {
                progressCounter++;
                if(!silent && progressCounter%1000==0)
                {
                    progress.setValue(inputfile.pos()/1000);
                    label.setText(QString("Imported DLT %1 IPC %2").arg(counterDLTMessages).arg(counterIPCMessages));
                }

                if (progress.wasCanceled())
                {
                    qDebug() << "fromMF4:" << "Import Stopped";
                    break;
                }

                //qDebug() << "posDt =" << posDt;
                inputfile.seek(pos+sizeof(mdf_hdr_t)+posDt);
                counterRecords++;
                if(inputfile.read((char*)&recordId,sizeof(quint16))!=sizeof(quint16))
                {
                    inputfile.close();
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
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.asynchronous,sizeof(quint8))!=sizeof(quint8))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.source,6)!=6)
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.destination,6)!=6)
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.etherType,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.crc,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.receivedDataByteCount,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.dataLength,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.dataBytes,sizeof(quint64))!=sizeof(quint64))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(!recordData.isEmpty())
                        {
                            int pos = 0;
                            quint64 time = hdBlockLinks.start_time_ns+ethFrame.timeStamp+(hdBlockLinks.hd_tz_offset_min+hdBlockLinks.hd_dst_offset_min)*60*1000000000;
                            if(!dltFromEthernetFrame(outputfile,recordData,pos,ethFrame.etherType,time/1000000000,time%1000000000/1000))
                            {
                                inputfile.close();
                                qDebug() << "fromMF4: ERROR:" << "Size Error: Cannot read Ethernet Frame";
                                return;
                            }
                            pos = 0;
                            if(!ipcFromEthernetFrame(outputfile,recordData,pos,ethFrame.etherType,time/1000000000,time%1000000000/1000))
                            {
                                inputfile.close();
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
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.asynchronous,sizeof(quint8))!=sizeof(quint8))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.source,6)!=6)
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.destination,6)!=6)
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.etherType,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.crc,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.receivedDataByteCount,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.beaconTimeStamp,sizeof(quint64))!=sizeof(quint64)) // TODO: Beacon Time Stamp
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.dataLength,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&ethFrame.dataBytes,sizeof(quint64))!=sizeof(quint64))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(!recordData.isEmpty())
                        {
                            int pos = 0;
                            quint64 time = hdBlockLinks.start_time_ns+ethFrame.timeStamp+(hdBlockLinks.hd_tz_offset_min+hdBlockLinks.hd_dst_offset_min)*60*1000000000;
                            if(!dltFromEthernetFrame(outputfile,recordData,pos,ethFrame.etherType,time/1000000000,time%1000000000/1000))
                            {
                                inputfile.close();
                                qDebug() << "fromMF4: ERROR:" << "Size Error: Cannot read Ethernet Frame";
                                return;
                            }
                            pos = 0;
                            if(!ipcFromEthernetFrame(outputfile,recordData,pos,ethFrame.etherType,time/1000000000,time%1000000000/1000))
                            {
                                inputfile.close();
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
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&dltFrameBlock.asynchronous,sizeof(quint8))!=sizeof(quint8))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&dltFrameBlock.currentFragmentNumber,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&dltFrameBlock.lastFragmentNumber,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&dltFrameBlock.ecuId,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&dltFrameBlock.dataLength,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&dltFrameBlock.dataBytes,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(!recordData.isEmpty())
                        {
                            int pos = 0;
                            quint64 time = hdBlockLinks.start_time_ns+dltFrameBlock.timeStamp+(hdBlockLinks.hd_tz_offset_min+hdBlockLinks.hd_dst_offset_min)*60*1000000000;
                            if(!dltFrame(outputfile,recordData,pos,time/1000000000,time%1000000000/1000))
                            {
                                inputfile.close();
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
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.asynchronous,sizeof(quint8))!=sizeof(quint8))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.probeId,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.msgType,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.probeFlags,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.dataFlags,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.dataCounter,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.dataLength,sizeof(quint16))!=sizeof(quint16))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(inputfile.read((char*)&plpRaw.dataBytes,sizeof(quint32))!=sizeof(quint32))
                        {
                            inputfile.close();
                            qDebug() << "fromMF4:" << "Size Error: Cannot read Record";
                            return;
                        }
                        if(!recordData.isEmpty())
                        {
                            int pos = 0;
                            quint64 time = hdBlockLinks.start_time_ns+plpRaw.timeStamp+(hdBlockLinks.hd_tz_offset_min+hdBlockLinks.hd_dst_offset_min)*60*1000000000;
                            if(!ipcFromPlpRaw(&plpRaw,outputfile,recordData,pos,ethFrame.etherType,time/1000000000,time%1000000000/1000))
                            {
                                inputfile.close();
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
        if(mdfHeader.id[0]==0 && mdfHeader.id[1]==0 && mdfHeader.id[2]==0 && mdfHeader.id[3]==0)
        {
            // end reached
            break;

        }
        inputfile.seek(pos+mdfHeader.length);
    }

    inputfile.close();

    qDebug() << "fromMF4: counterRecords:" << counterRecords;
    qDebug() << "fromMF4: counterRecordsDLT:" << counterRecordsDLT;
    qDebug() << "fromMF4: counterDLTMessages:" << counterDLTMessages;
    qDebug() << "fromMF4: counterRecordsIPC:" << counterRecordsIPC;
    qDebug() << "fromMF4: counterIPCMessages:" << counterIPCMessages;

    qDebug() << "fromMF4: Import finished";
}

bool DltImporter::ipcFromEthernetFrame(QFile &outputfile,QByteArray &record,int pos,quint16 etherType,quint32 sec,quint32 usec)
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
       if(record.size()<(pos+sizeof(plp_header_t)))
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
       bool multiFrame = qFromBigEndian(plpHeader->probeFlags) & 0x8;
       if(qFromBigEndian(plpHeader->probeId) == 0xd0 && qFromBigEndian(plpHeader->msgType) == 0x500)
       {
           counterRecordsIPC++;
           while(record.size()>=(pos+sizeof(plp_header_data_t)))
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
                   arg1.setEndianness(QDltArgument::DltEndiannessLittleEndian);
                   arg1.setOffsetPayload(0);
                   arg1.setData(record.mid(pos-sizeof(plp_header_data_t),sizeof(plp_header_data_t)));
                   msg.addArgument(arg1);

                   // add IPC Header
                   QDltArgument arg2;
                   arg2.setTypeInfo(QDltArgument::DltTypeInfoRawd);
                   arg2.setEndianness(QDltArgument::DltEndiannessLittleEndian);
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
                   arg3.setEndianness(QDltArgument::DltEndiannessLittleEndian);
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
                   writeDLTMessageToFile(outputfile,payload,0,0,0,sec,usec);
               }

               pos += qFromBigEndian(plpHeaderData->length);
           }
       }
    }

    return true;
}

bool DltImporter::ipcFromPlpRaw(mdf_plpRaw_t *plpRaw, QFile &outputfile,QByteArray &record,int pos,quint16 etherType,quint32 sec,quint32 usec)
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
       bool multiFrame = plpRaw->probeFlags & 0x8;
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
                   arg1.setEndianness(QDltArgument::DltEndiannessLittleEndian);
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
                   arg2.setEndianness(QDltArgument::DltEndiannessLittleEndian);
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
                   arg3.setEndianness(QDltArgument::DltEndiannessLittleEndian);
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
                   writeDLTMessageToFile(outputfile,payload,0,0,0,sec,usec);
               }

       }

    return true;
}

bool DltImporter::dltFrame(QFile &outputfile,QByteArray &record,int pos,quint32 sec,quint32 usec)
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
            writeDLTMessageToFile(outputfile,empty,dataPtr,sizeMsg,0,sec,usec);
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


bool DltImporter::dltFromEthernetFrame(QFile &outputfile,QByteArray &record,int pos,quint16 etherType,quint32 sec,quint32 usec)
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
        if(record.size()<(pos+sizeof(plp_header_t)))
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
            if(!dltFromEthernetFrame(outputfile,record,pos,etherType2,sec,usec)) // TODO: Use time from PLP instead
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
       quint16 identification = (((quint16)record.at(pos))<<8)|((quint16)(record.at(pos+1)&0xff));
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
               if(destPort==3490)
               {
                   pos+=6;
                   dltFrame(outputfile,record,pos,sec,usec);
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
                   if(destPort==3490)
                   {
                       pos+=6;
                       dltFrame(outputfile,segmentBufferUDP,pos,sec,usec);
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

void DltImporter::writeDLTMessageToFile(QFile &outputfile,QByteArray &bufferHeader,char* bufferPayload,quint32 bufferPayloadSize,EcuItem* ecuitem,quint32 sec,quint32 usec)
{
    DltStorageHeader str;

    str.pattern[0]='D';
    str.pattern[1]='L';
    str.pattern[2]='T';
    str.pattern[3]=0x01;
    str.ecu[0]=0;
    str.ecu[1]=0;
    str.ecu[2]=0;
    str.ecu[3]=0;

    if (sec || usec)
    { // todo should better use ptrs and not != 0
        str.seconds = (time_t)sec;
        str.microseconds = (int32_t)usec;
    }
    else
    {
#if defined(_MSC_VER)
        struct timespec ts;
        (void)timespec_get(&ts, TIME_UTC);
        str.seconds = (time_t)ts.tv_sec;
        str.microseconds = (int32_t)(ts.tv_nsec / 1000);
#else
        struct timeval tv;
        gettimeofday(&tv, NULL);
        str.seconds = (time_t)tv.tv_sec;        /* value is long */
        str.microseconds = (int32_t)tv.tv_usec; /* value is long */
#endif
    }
    if (ecuitem)
        dlt_set_id(str.ecu, ecuitem->id.toLatin1());

    /* check if message is matching the filter */
    if(!outputfile.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        qDebug() << "Failed opening WriteOnly" << outputfile.fileName();
    }

    // write data into file
    if(!ecuitem || !ecuitem->getWriteDLTv2StorageHeader())
    {
        // write version 1 storage header
        outputfile.write((char*)&str,sizeof(DltStorageHeader));
    }
    else
    {
        // write version 2 storage header
        outputfile.write((char*)"DLT",3);
        quint8 version = 2;
        outputfile.write((char*)&version,1);
        quint32 nanoseconds = str.microseconds * 1000ul; // not in big endian format
        outputfile.write((char*)&nanoseconds,4);
        quint64 seconds = (quint64) str.seconds; // not in big endian format
        outputfile.write(((char*)&seconds),5);
        quint8 length;
        length = ecuitem->id.length();
        outputfile.write((char*)&length,1);
        outputfile.write(ecuitem->id.toLatin1(),ecuitem->id.length());
    }
    outputfile.write(bufferHeader);
    outputfile.write(bufferPayload,bufferPayloadSize);
    outputfile.flush();
    outputfile.close();

}


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

void DltImporter::dltFromPCAP(QFile &outputfile,QString fileName,QWidget *parent)
{
    counterRecords = 0;
    counterRecordsDLT = 0;
    counterDLTMessages = 0;

    QFile inputfile(fileName);

    if(!inputfile.open(QFile::ReadOnly))
       return;

    QProgressDialog progress("Import DLT from PCAP...", "Abort Import", 0, inputfile.size()/1000, parent);
    progress.setWindowTitle("Import DLT from PCAP");
    QLabel label(&progress);
    progress.setWindowModality(Qt::WindowModal);
    progress.setLabel(&label);
    quint64 progressCounter = 0;

    pcap_hdr_t globalHeader;
    pcaprec_hdr_t recordHeader;

    QDltMsg qmsg;

    qDebug() << "Import DLT from PCAP file:" << fileName;

    if(inputfile.read((char*)&globalHeader,sizeof(pcap_hdr_t))!=sizeof(pcap_hdr_t))
    {
        inputfile.close();
        qDebug() << "dltFromPCAP:" << "Cannot open file" << fileName;
        return;
    }
    while(inputfile.read((char*)&recordHeader,sizeof(pcaprec_hdr_t))==sizeof(pcaprec_hdr_t))
    {
        progressCounter++;
        if(progressCounter%1000==0)
        {
            progress.setValue(inputfile.pos()/1000);
            label.setText(QString("Found %1").arg(counterDLTMessages));
        }

        if (progress.wasCanceled())
        {
            inputfile.close();

            qDebug() << "Counter Records:" << counterRecords;
            qDebug() << "Counter Records DLT:" << counterRecordsDLT;
            qDebug() << "Counter DLT Mesages:" << counterDLTMessages;
            qDebug() << "dltFromPCAP:" << "Import Stopped";
            return;
        }

        QByteArray record = inputfile.read(recordHeader.incl_len);
         if(record.length() != recordHeader.incl_len)
         {
             inputfile.close();
             qDebug() << "PCAP file not complete!";
             qDebug() << "Counter Records:" << counterRecords;
             qDebug() << "Counter Records DLT:" << counterRecordsDLT;
             qDebug() << "Counter DLT Mesages:" << counterDLTMessages;
             qDebug() << "dltFromPCAP:" << "Size Error: Cannot read Record";
             return;
         }
         counterRecords ++;
         // Check if Record is IP/UDP Packet with Dest Port 3490
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
             qDebug() << "dltFromPCAP: Counter Records:" << counterRecords;
             qDebug() << "dltFromPCAP: Counter Records DLT:" << counterRecordsDLT;
             qDebug() << "dltFromPCAP: Counter DLT Mesages:" << counterDLTMessages;
             qDebug() << "dltFromPCAP:" << "Size Error: Cannot read Ethernet Frame";
             return;
         }
    }
    inputfile.close();

    qDebug() << "dltFromPCAP: Counter Records:" << counterRecords;
    qDebug() << "dltFromPCAP: Counter Records DLT:" << counterRecordsDLT;
    qDebug() << "dltFromPCAP: Counter DLT Mesages:" << counterDLTMessages;

    qDebug() << "dltFromPCAP: Import finished";

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
    if(etherType==0x0800) // IP packet found
    {
       pos+=9;
       if(record.size()<(pos+1))
       {
           qDebug() << "Size issue!";
           return false;
       }
       quint8 protocol = record.at(pos);
       if(protocol==0x11) // UDP packet found
       {
           pos+=11;
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
               counterRecordsDLT++;
               // Now read the DLT Messages
               quint64 dataSize;
               //if(recordHeader.orig_len<record.size())
               //    dataSize = recordHeader.orig_len-pos;
               //else
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
           }
       }
    }
    return true;
}

void DltImporter::ipcFromPCAP(QFile &outputfile,QString fileName,QWidget *parent)
{
    counterRecords = 0;
    counterRecordsIPC = 0;
    counterIPCMessages = 0;

    QFile inputfile(fileName);

    if(!inputfile.open(QFile::ReadOnly))
       return;

    QProgressDialog progress("Import IPC from PCAP...", "Abort Import", 0, inputfile.size()/1000, parent);
    progress.setWindowTitle("Import IPC from PCAP");
    QLabel label(&progress);
    progress.setWindowModality(Qt::WindowModal);
    progress.setLabel(&label);
    quint64 progressCounter = 0;

    pcap_hdr_t globalHeader;
    pcaprec_hdr_t recordHeader;

    qDebug() << "Import IPC from PCAP file:" << fileName;

    if(inputfile.read((char*)&globalHeader,sizeof(pcap_hdr_t))!=sizeof(pcap_hdr_t))
    {
        inputfile.close();
        return;
    }

    inSegment = false;
    segmentBuffer.clear();
    while(inputfile.read((char*)&recordHeader,sizeof(pcaprec_hdr_t))==sizeof(pcaprec_hdr_t))
    {
        progressCounter++;
        if(progressCounter%1000==0)
        {
            progress.setValue(inputfile.pos()/1000);
            label.setText(QString("Found %1").arg(counterIPCMessages));
        }

        if (progress.wasCanceled())
        {
            inputfile.close();

            qDebug() << "Counter Records:" << counterRecords;
            qDebug() << "Counter Records IPC:" << counterRecordsIPC;
            qDebug() << "Counter IPC Mesages:" << counterIPCMessages;

            return;
        }

        QByteArray record = inputfile.read(recordHeader.incl_len);
         if(record.length() != recordHeader.incl_len)
         {
             inputfile.close();
             qDebug() << "PCAP file not complete!";
             qDebug() << "Counter Records:" << counterRecords;
             qDebug() << "Counter Records IPC:" << counterRecordsIPC;
             qDebug() << "Counter IPC Mesages:" << counterIPCMessages;
             return;
         }
         counterRecords ++;
         // Check if Record is PLP packet
         quint64 pos = 12;
         //Read EtherType
         if(record.size()<(pos+2))
         {
             qDebug() << "Size issue!";
             inputfile.close();
             qDebug() << "PCAP file not complete!";
             qDebug() << "Counter Records:" << counterRecords;
             qDebug() << "Counter Records IPC:" << counterRecordsIPC;
             qDebug() << "Counter IPC Mesages:" << counterIPCMessages;
             return;
         }
         quint16 etherType = (((quint16)record.at(pos))<<8)|((quint16)(record.at(pos+1)&0xff));
         pos+=2;
         if(!ipcFromEthernetFrame(outputfile,record,pos,etherType,recordHeader.ts_sec,recordHeader.ts_usec))
         {
             inputfile.close();
             qDebug() << "ipcFromPCAP: Counter Records:" << counterRecords;
             qDebug() << "ipcFromPCAP: Counter Records IPC:" << counterRecordsIPC;
             qDebug() << "ipcFromPCAP: Counter IPC Mesages:" << counterIPCMessages;
             qDebug() << "ipcFromPCAP:" << "Size Error: Cannot read Ethernet Frame";
             return;
         }

    }
    inputfile.close();

    qDebug() << "Counter Records:" << counterRecords;
    qDebug() << "Counter Records IPC:" << counterRecordsIPC;
    qDebug() << "Counter IPC Mesages:" << counterIPCMessages;

}

void DltImporter::dltFromMF4(QFile &outputfile,QString fileName,QWidget *parent)
{
    counterRecords = 0;
    counterRecordsDLT = 0;
    counterRecordsIPC = 0;
    counterDLTMessages = 0;
    counterIPCMessages = 0;

    QFile inputfile(fileName);

    if(!inputfile.open(QFile::ReadOnly))
    {
       qDebug() << "dltFromMF4:" << "Cannot open file" << fileName;
       return;
    }

    QProgressDialog progress("Import DLT from MF4...", "Abort Import", 0, inputfile.size()/1000, parent);
    progress.setWindowTitle("Import DLT from MF4");
    QLabel label(&progress);
    progress.setWindowModality(Qt::WindowModal);
    progress.setLabel(&label);
    quint64 progressCounter = 0;

    qDebug() << "Import DLT from MF4 file:" << fileName;

    if(inputfile.read((char*)&mdfIdblock,sizeof(mdf_idblock_t))!=sizeof(mdf_idblock_t))
    {
        inputfile.close();
        qDebug() << "counterRecords:" << counterRecords;
        qDebug() << "counterDLTMessages:" << counterDLTMessages;
        qDebug() << "dltFromMF4:" << "Size Error: Cannot reard Id Block";
        return;
    }

    mdf_hdr_t mdfHeader,mdfDgHeader,mdfCgHeader,mdfCnHeader;
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
                qDebug() << "counterRecords:" << counterRecords;
                qDebug() << "counterDLTMessages:" << counterDLTMessages;
                qDebug() << "dltFromMF4:" << "Size Error: Cannot reard HD Block";
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
                    qDebug() << "counterRecords:" << counterRecords;
                    qDebug() << "counterDLTMessages:" << counterDLTMessages;
                    qDebug() << "dltFromMF4:" << "Size Error: Cannot reard DG Block";
                    return;
                }
                if(mdfDgHeader.id[0]=='#' && mdfDgHeader.id[1]=='#' && mdfDgHeader.id[2]=='D' && mdfDgHeader.id[3]=='G')
                {
                    //qDebug() << "\tDG:";
                    mdf_dgblocklinks_t mdfDgBlockLinks;
                    if(inputfile.read((char*)&mdfDgBlockLinks,sizeof(mdf_dgblocklinks_t))!=sizeof(mdf_dgblocklinks_t))
                    {
                        inputfile.close();
                        qDebug() << "counterRecords:" << counterRecords;
                        qDebug() << "counterDLTMessages:" << counterDLTMessages;
                        qDebug() << "dltFromMF4:" << "Size Error: Cannot reard DG Block";
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
                            qDebug() << "counterRecords:" << counterRecords;
                            qDebug() << "counterDLTMessages:" << counterDLTMessages;
                            qDebug() << "dltFromMF4:" << "Size Error: Cannot reard CG Block";
                            return;
                        }
                        if(mdfCgHeader.id[0]=='#' && mdfCgHeader.id[1]=='#' && mdfCgHeader.id[2]=='C' && mdfCgHeader.id[3]=='G')
                        {
                            //qDebug() << "\t\tCG:";
                            mdf_cgblocklinks_t mdfCgBlockLinks;
                            if(inputfile.read((char*)&mdfCgBlockLinks,sizeof(mdf_cgblocklinks_t))!=sizeof(mdf_cgblocklinks_t))
                            {
                                inputfile.close();
                                qDebug() << "counterRecords:" << counterRecords;
                                qDebug() << "counterDLTMessages:" << counterDLTMessages;
                                qDebug() << "dltFromMF4:" << "Size Error: Cannot reard CG Block";
                                return;
                            }
                            //qDebug() << "\t\cg_record_id =" << mdfCgBlockLinks.cg_record_id;
                            //qDebug() << "\t\cg_data_bytes =" << mdfCgBlockLinks.cg_data_bytes;
                            ptrCg=mdfCgBlockLinks.cg_cg_next;
                            // Iterate through all channels
                            quint64 ptrCh = mdfCgBlockLinks.cg_cn_first;
                            while(ptrCh)
                            {
                                inputfile.seek(ptrCh);
                                if(inputfile.read((char*)&mdfCnHeader,sizeof(mdf_hdr_t))!=sizeof(mdf_hdr_t))
                                {
                                    qDebug() << "counterRecords:" << counterRecords;
                                    qDebug() << "counterDLTMessages:" << counterDLTMessages;
                                    qDebug() << "dltFromMF4:" << "Size Error: Cannot reard CN Block";
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
                                        qDebug() << "counterRecords:" << counterRecords;
                                        qDebug() << "counterDLTMessages:" << counterDLTMessages;
                                        qDebug() << "dltFromMF4:" << "Size Error: Cannot reard CN Block";
                                        return;
                                    }
                                    ptrCh=mdfChBlockLinks.cn_cn_next;
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
            QByteArray recordData;
            while(posDt<(mdfHeader.length-sizeof(mdf_hdr_t)))
            {
                progressCounter++;
                if(progressCounter%1000==0)
                {
                    progress.setValue(inputfile.pos()/1000);
                    label.setText(QString("Found DLT %1 IPC %2").arg(counterDLTMessages).arg(counterIPCMessages));
                }

                if (progress.wasCanceled())
                {
                    inputfile.close();

                    qDebug() << "counterRecords:" << counterRecords;
                    qDebug() << "counterDLTMessages:" << counterDLTMessages;
                    qDebug() << "dltFromMF4:" << "Import Stopped";
                    return;
                }

                //qDebug() << "posDt =" << posDt;
                inputfile.seek(pos+sizeof(mdf_hdr_t)+posDt);
                counterRecords++;
                if(inputfile.read((char*)&recordId,sizeof(quint16))!=sizeof(quint16))
                {
                    inputfile.close();
                    qDebug() << "counterRecords:" << counterRecords;
                    qDebug() << "counterDLTMessages:" << counterDLTMessages;
                    qDebug() << "dltFromMF4:" << "Size Error: Cannot read Record";
                    return;
                }
                posDt += sizeof(quint16);
                if(recordId==6 || recordId==4 ||recordId==2)
                {
                    if(inputfile.read((char*)&lengthVLSD,sizeof(quint32))!=sizeof(quint32))
                    {
                        inputfile.close();
                        qDebug() << "counterRecords:" << counterRecords;
                        qDebug() << "counterDLTMessages:" << counterDLTMessages;
                        qDebug() << "dltFromMF4:" << "Size Error: Cannot read Record";
                        return;
                    }
                    posDt += sizeof(quint32);
                    posDt += lengthVLSD;
                    recordData = inputfile.read(lengthVLSD);
                    //qDebug() << "recordId =" << recordId << "length =" << lengthVLSD;
                }
                else if(recordId==5 || recordId==3 ||recordId==1)
                {
                    if(inputfile.read((char*)&ethFrame.timeStamp,sizeof(quint64))!=sizeof(quint64))
                    {
                        inputfile.close();
                        qDebug() << "counterRecords:" << counterRecords;
                        qDebug() << "counterDLTMessages:" << counterDLTMessages;
                        qDebug() << "dltFromMF4:" << "Size Error: Cannot read Record";
                        return;
                    }
                    if(inputfile.read((char*)&ethFrame.asynchronous,sizeof(quint8))!=sizeof(quint8))
                    {
                        inputfile.close();
                        qDebug() << "counterRecords:" << counterRecords;
                        qDebug() << "counterDLTMessages:" << counterDLTMessages;
                        qDebug() << "dltFromMF4:" << "Size Error: Cannot read Record";
                        return;
                    }
                    if(inputfile.read((char*)&ethFrame.source,6)!=6)
                    {
                        inputfile.close();
                        qDebug() << "counterRecords:" << counterRecords;
                        qDebug() << "counterDLTMessages:" << counterDLTMessages;
                        qDebug() << "dltFromMF4:" << "Size Error: Cannot read Record";
                        return;
                    }
                    if(inputfile.read((char*)&ethFrame.destination,6)!=6)
                    {
                        inputfile.close();
                        qDebug() << "counterRecords:" << counterRecords;
                        qDebug() << "counterDLTMessages:" << counterDLTMessages;
                        qDebug() << "dltFromMF4:" << "Size Error: Cannot read Record";
                        return;
                    }
                    if(inputfile.read((char*)&ethFrame.etherType,sizeof(quint16))!=sizeof(quint16))
                    {
                        inputfile.close();
                        qDebug() << "counterRecords:" << counterRecords;
                        qDebug() << "counterDLTMessages:" << counterDLTMessages;
                        qDebug() << "dltFromMF4:" << "Size Error: Cannot read Record";
                        return;
                    }
                    if(inputfile.read((char*)&ethFrame.crc,sizeof(quint32))!=sizeof(quint32))
                    {
                        inputfile.close();
                        qDebug() << "counterRecords:" << counterRecords;
                        qDebug() << "counterDLTMessages:" << counterDLTMessages;
                        qDebug() << "dltFromMF4:" << "Size Error: Cannot read Record";
                        return;
                    }
                    if(inputfile.read((char*)&ethFrame.receivedDataByteCount,sizeof(quint32))!=sizeof(quint32))
                    {
                        inputfile.close();
                        qDebug() << "counterRecords:" << counterRecords;
                        qDebug() << "counterDLTMessages:" << counterDLTMessages;
                        qDebug() << "dltFromMF4:" << "Size Error: Cannot read Record";
                        return;
                    }
                    if(inputfile.read((char*)&ethFrame.dataLength,sizeof(quint32))!=sizeof(quint32))
                    {
                        inputfile.close();
                        qDebug() << "counterRecords:" << counterRecords;
                        qDebug() << "counterDLTMessages:" << counterDLTMessages;
                        qDebug() << "dltFromMF4:" << "Size Error: Cannot read Record";
                        return;
                    }
                    if(inputfile.read((char*)&ethFrame.dataBytes,sizeof(quint64))!=sizeof(quint64))
                    {
                        inputfile.close();
                        qDebug() << "counterRecords:" << counterRecords;
                        qDebug() << "counterDLTMessages:" << counterDLTMessages;
                        qDebug() << "dltFromMF4:" << "Size Error: Cannot read Record";
                        return;
                    }
                    posDt += 43;
                    //qDebug() << "recordId =" << recordId << "length =" << 43;
                    int pos = 0;
                    if(!dltFromEthernetFrame(outputfile,recordData,pos,ethFrame.etherType,(hdBlockLinks.start_time_ns+ethFrame.timeStamp)/1000000,(hdBlockLinks.start_time_ns+ethFrame.timeStamp)%1000000/1000))
                    {
                        inputfile.close();
                        qDebug() << "counterRecords:" << counterRecords;
                        qDebug() << "counterDLTMessages:" << counterDLTMessages;
                        qDebug() << "dltFromMF4: ERROR:" << "Size Error: Cannot read Ethernet Frame";
                        return;
                    }
                    pos = 0;
                    if(!ipcFromEthernetFrame(outputfile,recordData,pos,ethFrame.etherType,(hdBlockLinks.start_time_ns+ethFrame.timeStamp)/1000000,(hdBlockLinks.start_time_ns+ethFrame.timeStamp)%1000000/1000))
                    {
                        inputfile.close();
                        qDebug() << "counterRecords:" << counterRecords;
                        qDebug() << "counterDLTMessages:" << counterDLTMessages;
                        qDebug() << "dltFromMF4: ERROR:" << "Size Error: Cannot read Ethernet Frame";
                        return;
                    }
                }
                else
                {
                    qDebug() << "dltFromMF4: ERROR: Unknown recordId =" << recordId;
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

    qDebug() << "dltFromMF4: counterRecords:" << counterRecords;
    qDebug() << "dltFromMF4: counterRecordsDLT:" << counterRecordsDLT;
    qDebug() << "dltFromMF4: counterDLTMessages:" << counterDLTMessages;
    qDebug() << "dltFromMF4: counterRecordsIPC:" << counterRecordsIPC;
    qDebug() << "dltFromMF4: counterIPCMessages:" << counterIPCMessages;

    qDebug() << "dltFromMF4: Import finished";
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


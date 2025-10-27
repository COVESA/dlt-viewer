#ifndef QDLTIMPORTER_H
#define QDLTIMPORTER_H

#include <QMap>
#include <QThread>
#include <QObject>
#include <QFile>

#include "export_rules.h"
#include "dlt_common.h"

#include <optional>

typedef struct pcap_hdr_s {
        quint32 magic_number;   /* magic number */
        quint16 version_major;  /* major version number */
        quint16 version_minor;  /* minor version number */
        qint32  thiszone;       /* GMT to local correction */
        quint32 sigfigs;        /* accuracy of timestamps */
        quint32 snaplen;        /* max length of captured packets, in octets */
        quint32 network;        /* data link type */
} PACKED pcap_hdr_t;

typedef struct pcaprec_hdr_s {
        quint32 ts_sec;         /* timestamp seconds */
        quint32 ts_usec;        /* timestamp microseconds */
        quint32 incl_len;       /* number of octets of packet saved in file */
        quint32 orig_len;       /* actual length of packet */
} PACKED pcaprec_hdr_t;

typedef struct plp_header {
        quint16 probeId;
        quint16 counter;
        quint8 version;
        quint8 plpType;
        quint16 msgType;
        quint16 reserved;
        quint16 probeFlags;
} PACKED plp_header_t;

typedef struct plp_header_data {
        quint32 busSpecId;
        quint32 timeStampHigh;
        quint32 timeStampLow;
        quint16 length;
        quint16 dataFlags;
} PACKED plp_header_data_t;

typedef struct mdf_idblock {
        char id_file[8];
        char id_vers[8];
        char id_prog[8];
        unsigned char id_reserved1[4];
        quint16 id_ver;
        unsigned char id_reserved2[30];
        quint16 id_unfin_flags;
        quint16 id_custom_unfin_flags;
} PACKED mdf_idblock_t;

typedef struct mdf_hdblocklinks {
        quint64 hd_dg_first;
        quint64 hd_fh_first;
        quint64 hd_ch_first;
        quint64 hd_at_first;
        quint64 hd_ev_first;
        quint64 hd_md_comment;
        quint64 start_time_ns;
        qint16 hd_tz_offset_min;
        qint16 hd_dst_offset_min;
        quint8 hd_time_flags;
        quint8 hd_time_class;
        quint8 hd_flags;
} PACKED mdf_hdblocklinks_t;

typedef struct mdf_dgblocklinks {
        quint64 dg_dg_next;
        quint64 dg_cg_first;
        quint64 dg_data;
        quint64 dg_md_comment;
} PACKED mdf_dgblocklinks_t;

typedef struct mdf_dlblocklinks {
    quint8 dl_flags;
    quint8 reserved[3];
    quint32 dl_count;
} PACKED mdf_dlblocklinks_t;

typedef struct mdf_cgblocklinks {
        quint64 cg_cg_next;
        quint64 cg_cn_first;
        quint64 cg_tx_acq_name;
        quint64 cg_si_acq_source;
        quint64 cg_sr_first;
        quint64 cg_md_comment;
        quint64 cg_record_id;
        quint64 cg_cycle_count;
        quint16 cg_flags;
        quint16 cg_path_separator;
        char cg_reserved[4];
        quint32 cg_data_bytes;
        quint32 cg_inval_bytes;
} PACKED mdf_cgblocklinks_t;

typedef struct mdf_cnblocklinks {
        quint64 cn_cn_next;
        quint64 cn_composition;
        quint64 cn_tx_name;
        quint64 cn_si_source;
        quint64 cn_cc_conversion;
        quint64 cn_data;
        quint64 cn_md_unit;
        quint64 cn_md_comment;
        quint64 cn_at_reference;
} PACKED mdf_cnblocklinks_t;

typedef struct mdf_ethFrame {
        quint64 timeStamp;
        quint8 asynchronous;
        quint8 busChannel[6];
        quint8 source[6];
        quint8 destination[6];
        quint16 etherType;
        quint32 crc;
        quint32 receivedDataByteCount;
        quint64 beaconTimeStamp;
        quint32 dataLength;
        quint64 dataBytes;
} PACKED mdf_ethFrame_t;

typedef struct mdf_plpRaw {
        quint64 timeStamp;
        quint8 asynchronous;
        quint16 probeId;
        quint16 msgType;
        quint16 probeFlags;
        quint16 dataFlags;
        quint16 dataCounter;
        quint16 dataLength;
        quint32 dataBytes;
} PACKED mdf_plpRaw_t;

typedef struct mdf_dltFrame {
        quint64 timeStamp;
        quint8 asynchronous;
        quint16 currentFragmentNumber;
        quint16 lastFragmentNumber;
        quint32 ecuId;
        quint32 dataLength;
        quint32 dataBytes;
} PACKED mdf_dltFrame_t;

typedef struct mdf_hdr {
        char id[4];                /* id */
        unsigned char reserved[4]; /* reserved */
        quint64 length;         /* length of block */
        quint64 link_count;     /* number of links in link section */
} PACKED mdf_hdr_t;

class QDLT_EXPORT QDltImporter : public QThread
{
    Q_OBJECT

public:

    explicit QDltImporter(QFile *outputfile, QStringList fileNames ,QObject *parent=0);
    explicit QDltImporter(QFile *outputfile, QString fileName = "",QObject *parent=0);
    ~QDltImporter();

    void run() override;

    void dltIpcFromPCAP(QString fileName);
    void dltIpcFromMF4(QString fileName);

    void setOutputfile(QFile *newOutputfile);

    void setPcapPorts(const QString &importPcapPorts);

    struct DltStorageHeaderTimestamp {
        quint32 sec;
        quint32 usec;
    };
    static DltStorageHeader makeDltStorageHeader(std::optional<DltStorageHeaderTimestamp> timestamp = std::nullopt);

  private:

    bool dltFrame(QByteArray &record,int pos,quint32 sec = 0,quint32 usec = 0);
    bool dltFromEthernetFrame(QByteArray &record,int pos,quint16 etherType,quint32 sec = 0,quint32 usec = 0);
    bool ipcFromEthernetFrame(QByteArray &record,int pos,quint16 etherType,quint32 sec = 0,quint32 usec = 0);
    bool ipcFromPlpRaw(mdf_plpRaw_t *plpRaw, QByteArray &record,quint32 sec = 0,quint32 usec = 0);

    void writeDLTMessageToFile(QByteArray &bufferHeader,char* bufferPayload,quint32 bufferPayloadSize,QString ecuId,quint32 sec = 0,quint32 usec = 0);

    mdf_idblock_t mdfIdblock;
    mdf_hdblocklinks_t hdBlockLinks;

    quint64 counterRecords = 0;
    quint64 counterDLTMessages = 0;
    quint64 counterIPCMessages = 0;
    quint64 counterRecordsDLT = 0;
    quint64 counterRecordsIPC = 0;

    bool inSegment = false;
    QByteArray segmentBuffer;
    QByteArray segmentBufferUDP;

    QMap<quint16,int> channelGroupLength;
    QMap<quint16,QString> channelGroupName;

    QFile *outputfile;
    QStringList fileNames;

    QList<unsigned short> pcapPorts;

signals:

    void progress(QString name,int status, int progress);
    void resultReady(const QString &s);

};

#endif // QDLTIMPORTER_H


#include <dlt/dlt_common.h>
#include <dlt/dlt_types.h>

#include <QFile>

#include <benchmark/benchmark.h>

#include <string>
#include <vector>
#include <iostream>

const int READ_BUF_SZ = 1024 * 1024; // 1 MB

std::vector<qint64> parseMsgPositions(QFile& file) {
    quint8 version=1;
    int counter_header = 0;
    qint64 storageLength=0;
    qint64 lengthOffset=2;
    quint16 message_length = 0;
    qint64 current_message_pos = 0;
    qint64 next_message_pos = 0;
    char lastFound = 0;
    qint64 file_size = file.size();
    qint64 pos = 0;

    std::vector<qint64> positions;

    while(true)
    {
        /* read buffer from file */
        QByteArray buf = file.read(READ_BUF_SZ);
        if(buf.isEmpty())
            break; // EOF

        /* Use primitive buffer for faster access */
        int cbuf_sz = buf.size();
        const char *cbuf = buf.constData();

        /* find marker in buffer */
        for(int num=0;num<cbuf_sz;num++) {
            // search length of DLT message
            if(counter_header>0)
            {
                counter_header++;
                if(storageLength==13 && counter_header==13)
                {
                    storageLength += ((unsigned char)cbuf[num]) + 1;
                }
                else if (counter_header==storageLength)
                {
                    // Read DLT protocol version
                    version = (((unsigned char)cbuf[num])&0xe0)>>5;
                    if(version==1)
                    {
                        lengthOffset = 2;
                    }
                    else if(version==2)
                    {
                        lengthOffset = 5;
                    }
                    else
                    {
                        lengthOffset = 2;  // default
                    }
                }
                else if (counter_header==(storageLength+lengthOffset)) // was 16
                {
                    // Read low byte of message length
                    message_length = (unsigned char)cbuf[num];
                }
                else if (counter_header==(storageLength+1+lengthOffset)) // was 17
                {
                    // Read high byte of message length
                    counter_header = 0;
                    message_length = (message_length<<8 | ((unsigned char)cbuf[num])) + storageLength;
                    next_message_pos = current_message_pos + message_length;
                    if(next_message_pos==file_size)
                    {
                        // last message found in file
                        positions.push_back(current_message_pos);
                        break;
                    }
                    // speed up move directly to next message, if inside current buffer
                    if((message_length > storageLength+2+lengthOffset)) // was 20
                    {
                        if((num+message_length-(storageLength+2+lengthOffset)<cbuf_sz))  // was 20
                        {
                            num+=message_length-(storageLength+2+lengthOffset);  // was 20
                        }
                    }
                }
            }
            else if(cbuf[num] == 'D')
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
            else if(lastFound == 'T' && (cbuf[num] == 0x01 || cbuf[num] == 0x02))
            {
                if(next_message_pos == 0)
                {
                    // first message detected or first message after error
                    current_message_pos = pos+num-3;
                    counter_header = 3;
                    if(cbuf[num] == 0x01)
                        storageLength = 16;
                    else
                        storageLength = 13;
                    if(current_message_pos!=0)
                    {
                        // first messages not at beginning or error occurred before
                        //errors_in_file++;
                    }
                    // speed up move directly to message length, if inside current buffer
                    if(num+9<cbuf_sz)
                    {
                        num+=9;
                        counter_header+=9;
                    }
                }
                else if( next_message_pos == (pos+num-3) )
                {
                    // Add message only when it is in the correct position in relationship to the last message
                    positions.push_back(current_message_pos);
                    current_message_pos = pos+num-3;
                    counter_header = 3;
                    if(cbuf[num] == 0x01)
                        storageLength = 16;
                    else
                        storageLength = 13;
                    // speed up move directly to message length, if inside current buffer
                    if(num+9<cbuf_sz)
                    {
                        num+=9;
                        counter_header+=9;
                    }
                }
                else if(next_message_pos > (pos+num-3))
                {
                  // Header detected before end of message
                }
                else
                {
                    // Header detected after end of message
                    // start search for new message back after last header found
                    file.seek(current_message_pos+4);
                    pos = current_message_pos+4;
                    buf = file.read(READ_BUF_SZ);
                    cbuf_sz = buf.size();
                    cbuf = buf.constData();
                    num=0;
                    next_message_pos = 0;
                }
                lastFound = 0;
            }
            else
            {
                lastFound = 0;
            }
        }
        pos += cbuf_sz;
    }

    return positions;
}

static void BM_ReadFile(benchmark::State& state, const std::string& path) {
    QFile file(QString::fromStdString(path));

    if (!file.open(QIODevice::ReadOnly)) {
        state.SkipWithError("Failed to open file");
        return;
    }
    std::vector<qint64> positions;
    for (auto _ : state) {
        positions = parseMsgPositions(file);
    }

}

std::vector<qint64> parseMsgPositionsUsingLibDlt(const std::string& filePath) {
    std::vector<qint64> positions;

    DltFile file;

    if (dlt_file_init(&file, 0) != DLT_RETURN_OK) {
        std::cerr << "Failed to initialize DLT file structure." << std::endl;
        return positions;
    }

    // Initialize DltFile structure
    if (dlt_file_open(&file, filePath.c_str(), 0) != DLT_RETURN_OK) {
        std::cerr << "Failed to open DLT file: " << filePath << std::endl;
        return positions;
    }

    while (dlt_file_read(&file, 0) >= DLT_RETURN_OK) {
        // dlt_file_read updates file.position to the start of the message
        positions.push_back(file.position);

    }

    dlt_file_close(&file, 0);
    return positions;
}

std::vector<qint64> parseMsgPositionsUsingLibDlt1(const std::string& filePath) {
    std::vector<qint64> positions;

    DltFile file;

    if (dlt_file_init(&file, 0) != DLT_RETURN_OK) {
        std::cerr << "Failed to initialize DLT file structure." << std::endl;
        return positions;
    }

    // Initialize DltFile structure
    if (dlt_file_open(&file, filePath.c_str(), 0) != DLT_RETURN_OK) {
        std::cerr << "Failed to open DLT file: " << filePath << std::endl;
        return positions;
    }

    int32_t position = 0;
    while (dlt_file_read_header(&file, 0) >= DLT_RETURN_OK) {
        // dlt_file_read updates file.position to the start of the message
        positions.push_back(position);
        //std::cout << "file position: " << file.file_position << std::endl;
        //std::cout << "header size: " << file.msg.headersize << std::endl;
        //std::cout << "data size: " << file.msg.datasize << std::endl;
        position += file.msg.headersize + file.msg.datasize;
        //std::cout << "msg position: " << position << std::endl;
        //file.file_position = position;//ftell(file.handle);
        fseek(file.handle,position,SEEK_SET);
    }

    dlt_file_close(&file, 0);
    return positions;
}

static void BM_parseWithDltlib(benchmark::State& state, const std::string& path) {

    std::vector<qint64> positions;
    for (auto _ : state) {
        positions = parseMsgPositionsUsingLibDlt(path);
        benchmark::DoNotOptimize(positions);
    }
}

static void BM_parseWithDltlib1(benchmark::State& state, const std::string& path) {

    std::vector<qint64> positions;
    for (auto _ : state) {
        positions = parseMsgPositionsUsingLibDlt1(path);
        benchmark::DoNotOptimize(positions);
    }
}

int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);

    std::string path = argv[1];

    benchmark::RegisterBenchmark(("BM_ReadFile/" + path).c_str(), BM_ReadFile, path);
    benchmark::RegisterBenchmark(("BM_parseWithDltlib/" + path).c_str(), BM_parseWithDltlib, path);
    benchmark::RegisterBenchmark(("BM_parseWithDltlib1/" + path).c_str(), BM_parseWithDltlib1, path);

    //parseMsgPositionsUsingLibDlt1(path);

    benchmark::RunSpecifiedBenchmarks();

    return 0;
}

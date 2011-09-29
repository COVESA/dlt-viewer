#include "file.h"
#include <iostream>
#include <fstream>
using namespace std;


File::File():QTreeWidgetItem()
{

}

File::File(QTreeWidgetItem *parent):QTreeWidgetItem(parent)
{
    receivedPackages = 0;
    writtenBytes=0;
    //fileSerialNumber = -1;
    //packages = -1;
    //receivedPackages = -1;
    //sizeInBytes = -1;
    //buffer = -1;
    this->setText(COLUMN_STATUS, "In progress");
    this->setTextColor(COLUMN_STATUS,Qt::white);
    this->setBackgroundColor(COLUMN_STATUS,Qt::red);
    this->setText(COLUMN_RECPACKAGES, "0");

}

File::~File()
{

}

QString File::getFilename(){
    QStringList pathList = filename.split("/");
    return pathList.last();
}
QString File::getFileCreationDate(){
    return fileCreationDate;
}

QString File::getFilenameOnTarget(){
    return filename;
}
QString File::getFileSerialNumber(){
    QString str;
    str.append(QString("%1").arg(fileSerialNumber));

    return str;
}
unsigned int File:: getPackages(){
    return packages;
}
unsigned int File::getReceivedPackages(){
    return receivedPackages;
}
unsigned int File::getSizeInBytes(){
    return sizeInBytes;
}
unsigned int File::getBufferSize(){
    return buffer;
}



void File::setFilename(QString f){
    filename = f;
    this->setText(COLUMN_FILENAME, filename);
}

void File::setFileCreationDate(QString f){
    fileCreationDate = f;
    this->setText(COLUMN_FILEDATE, fileCreationDate);
}

void File::setFileSerialNumber(QString s){
    fileSerialNumber = s.toInt();
    this->setText(COLUMN_FILEID, s);

}

void File::setPackages(QString p){
    packages = p.toInt();
    data = new QByteArray[packages];
    fullData = new QByteArray();
    dataSize = new int[packages];
    this->setText(COLUMN_PACKAGES, p);
}
void File::increaseReceivedPackages(){
    receivedPackages++;

    QString str;
    str.append(QString("%1").arg(receivedPackages));
    this->setText(COLUMN_RECPACKAGES, str);
}

void File::setSizeInBytes(QString s){
    sizeInBytes = s.toInt();
    this->setText(COLUMN_SIZE, s);
}

void File::setBuffersize(QString b){
    buffer = b.toInt();
    this->setText(COLUMN_BUFFERSIZE, b);
}

void File::setComplete(){
    this->setText(COLUMN_STATUS, "Complete");
    this->setBackgroundColor(COLUMN_STATUS,Qt::green);
    this->setTextColor(COLUMN_STATUS,Qt::black);
}

void File::errorHappens(QString filename, QString errorCode1, QString errorCode2, QString time){
    setFilename(filename);

    fileSerialNumber = 1;
    packages = 2;
    receivedPackages = 3;
    sizeInBytes = 4;
    buffer = 5;

    QString str = errorCode1+", "+errorCode2;
    this->setText(COLUMN_APID,str);

    this->setText(COLUMN_FILEDATE,time);
    this->setText(COLUMN_STATUS, "ERROR");
    this->setTextColor(COLUMN_STATUS,Qt::white);
    this->setBackgroundColor(COLUMN_STATUS,Qt::red);

}

bool File::isComplete(){
    return receivedPackages==packages;
}

void File::appendData(QString packageNumber, int size, const QByteArray& ba)
{
    //Prints every incoming byte
    /*const char *data22 = ba.data();
     while (*data22) {
         cout << "[" << *data22 << "]" << endl;
         ++data22;
     }*/

    int i = packageNumber.toInt();
    data[i-1] = ba;
    dataSize[i-1] = size;
    increaseReceivedPackages();
    //Prints every stored byte
    /*const char *data23 = data[i-1].data();
    while (*data23) {
         cout << "[" << *data23 << "]" << endl;
         ++data23;
     }*/

}

bool File::saveFile(QString newFile){

    //QString newFile = directory.append("/").append(getFilename());
    if(QFile::exists(newFile)){
        if(!QFile::remove(newFile)){
            std::cout << "couldnt remove file" <<std::endl;
            return false;
        }
    }

    QFile file(newFile);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream stream( &file );
    writtenBytes=0;
    for(unsigned int i=0; i<packages;i++){
        int actuallyWrittenBytes = stream.writeRawData(data[i],data[i].size());
        writtenBytes+=actuallyWrittenBytes;

        if(actuallyWrittenBytes != dataSize[i]){
            return false;
        }

        //std::cout << "writtenBytes(" << i <<") : " << writtenBytes << std::endl;
        //stream<<data[i];
        //std::cout << data[i].data() << std::endl;
        /*char *data2 = data[i].data();
        while (*data2) {
            cout << "[" << *data2 << "]" << endl;
            ++data2;
        }*/
    }

    file.close();

    if((unsigned int)writtenBytes != sizeInBytes){
        return false;
    }

    return true;
}

QString File::saveAsTmpFile()
{
    QString tmpFileName = QDir::tempPath();
    tmpFileName += "/";
    tmpFileName += getFilename();

    if(saveFile(tmpFileName)){
        return tmpFileName;
    }
    else
    {
        return QString();
    }
}
bool File::removeTmpFile(QString tmp)
{
    if(QFile::exists(tmp))
    {
        if(!QFile::remove(tmp))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }

}
int File::getWrittenBytes()
{
    return writtenBytes;
}

QByteArray File::getData(){
   QByteArray tmp;
    for(unsigned int i=0; i<packages;i++){
       tmp.append(data[i]);
    }
    return tmp;
}

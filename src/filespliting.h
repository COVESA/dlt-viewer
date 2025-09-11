#ifndef FILESPLITING_H
#define FILESPLITING_H

#include <qmainwindow.h>
#include <QFile>

class FileSpliting : public QWidget
{
    Q_OBJECT

  public:

    explicit FileSpliting(QWidget *parent = nullptr);

    void splitDLTFile_triggered(QFile &file,QStringList path); //Split DLT Files
    void splitOutputFile(QStringList filePath,qint64 maxChunkSizeBytes, const QString &destinationFolder);

    void setFile(QFile *file);



  private:

    QFile *m_file = nullptr;

};

#endif // FILESPLITING_H

#include <mainwindow.h>
#include <filespliting.h>

#include "qmessagebox.h"
#include <QDialog>
#include <QFileDialog>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>

FileSpliting::FileSpliting(QWidget *parent)
    : QWidget(parent){

}

void FileSpliting::setFile(QFile *file)
{
    m_file = file;
}


//The function is triggered when Split DLT File is clicked which is under File Menu.
//A dialog box asking for the splitting size is opened where the user can specify the size in KB,MB or GB.
//The size is casted toqint64 and them parsed to splitOutputFile function.
//The destination path can also be selected by user which will also be parsend to splitOutFile function.

void FileSpliting::splitDLTFile_triggered(QFile &file,QStringList path){
    qDebug() << "Split File Triggered";

    QDialog dialog(this);
    dialog.setWindowTitle("Enter Split Size");

    QLabel *sizeLabel = new QLabel("Size (number):");
    QLineEdit *sizeEdit = new QLineEdit();
    sizeEdit->setValidator(new QDoubleValidator(0, 999999, 2, this)); // Accepts decimal input

    QLabel *unitLabel = new QLabel("Unit:");
    QComboBox *unitCombo = new QComboBox();
    unitCombo->addItem("KB");
    unitCombo->addItem("MB");
    unitCombo->addItem("GB");

    QPushButton *okButton = new QPushButton("OK");
    QPushButton *cancelButton = new QPushButton("Cancel");

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(sizeLabel);
    inputLayout->addWidget(sizeEdit);
    inputLayout->addWidget(unitLabel);
    inputLayout->addWidget(unitCombo);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(buttonLayout);
    dialog.setLayout(mainLayout);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        double sizeValue = sizeEdit->text().toDouble();
        QString sizeUnit = unitCombo->currentText();

        qint64 multiplier = 1;
        if (sizeUnit == "KB") multiplier = 1024LL;
        else if (sizeUnit == "MB") multiplier = 1024LL * 1024;
        else if (sizeUnit == "GB") multiplier = 1024LL * 1024 * 1024;

        qint64 maxChunkSizeBytes = static_cast<qint64>(sizeValue * multiplier);

               //FileSaveDialog Implementation
        QString folderPath = QFileDialog::getExistingDirectory(
            this,
            "Select Folder to Save Split Files",
            QDir::homePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
            );

        if (folderPath.isEmpty()) {
            QMessageBox::warning(this, "No Folder Selected", "Split operation canceled.");
            return;
        }


        if (folderPath.isEmpty()) {
            QMessageBox::warning(this, "No Folder Selected", "Split operation canceled.");
            return;
        }

        splitOutputFile(path,maxChunkSizeBytes, folderPath);
    }
}

//The outputfile (i.e) DLT File is opened in ReadOnly mode and splitted on basis of requirement given by the user.
//If the same files are splitted twice the olde files will be deleted.
//chunk carry is done to avoid data loss
void FileSpliting::splitOutputFile(QStringList filePath,qint64 maxChunkSizeBytes, const QString &destinationFolder){

    if (!m_file->isOpen()) {
        qWarning() << "Failed to open Output File for File Splitting";
        return;
    }

    QString fullPath = filePath[0];

    QFileInfo fileInfo(fullPath);
    QString baseName = fileInfo.completeBaseName();
    QString extension = fileInfo.completeSuffix();

    QDir dir(destinationFolder);
    QString pattern = QString("%1_%2.%3").arg(baseName).arg("*").arg(extension);
    QStringList oldFiles = dir.entryList(QStringList() << pattern, QDir::Files);
    for (const QString &file : oldFiles) {
        QString fullFilePath = dir.filePath(file);
        if (QFile::remove(fullFilePath)) {
            qDebug() << "Deleted old split:" << QDir::toNativeSeparators(fullFilePath);
        } else {
            qWarning() << "Failed to delete:" << fullFilePath;
        }
    }

    int fileIndex = 1;
    qint64 accumulatedSize = 0;
    QByteArray buffer;

           // Reset to start
    m_file->seek(0);

    while (!m_file->atEnd()) {
        // Read DLT standard header (first 4 bytes)
        QByteArray header = m_file->read(4);
        if (header.size() < 4) {
            qDebug() << "Reached EOF while reading header.";
            break;
        }

               // Extract payload length from bytes 2 and 3
        quint16 payloadLen = ((quint8)header[2] << 8) | (quint8)header[3];
        quint32 msgLen = payloadLen + 4;  // total = header + payload

               // Check if enough bytes remain in file
        if (m_file->bytesAvailable() < (msgLen - 4)) {
            qWarning() << "Unexpected EOF: file ends before message fully read.";
            break;
        }

               // Read payload
        QByteArray payload = m_file->read(msgLen - 4);
        QByteArray completeMessage = header + payload;

               // Check if adding this message exceeds current chunk size
        if (accumulatedSize + completeMessage.size() > maxChunkSizeBytes && !buffer.isEmpty()) {
            // Write buffer to new file
            QString outputFileName = QString("%1/%2_%3.%4")
                                         .arg(destinationFolder)
                                         .arg(baseName)
                                         .arg(fileIndex++)
                                         .arg(extension);

            QFile output(outputFileName);
            if (output.open(QIODevice::WriteOnly)) {
                output.write(buffer);
                output.close();
                qDebug() << "Written split:" << QDir::toNativeSeparators(outputFileName);
            } else {
                qWarning() << "Failed to create" << outputFileName;
                break;
            }

                   // Reset buffer
            buffer.clear();
            accumulatedSize = 0;
        }

               // Append message to buffer
        buffer.append(completeMessage);
        accumulatedSize += completeMessage.size();
    }

           // Write remaining buffer if not empty
    if (!buffer.isEmpty()) {
        QString outputFileName = QString("%1/%2_%3.%4")
        .arg(destinationFolder)
            .arg(baseName)
            .arg(fileIndex++)
            .arg(extension);

        QFile output(outputFileName);
        if (output.open(QIODevice::WriteOnly)) {
            output.write(buffer);
            output.close();
            qDebug() << "Written last split:" << QDir::toNativeSeparators(outputFileName);
        }
    }

    m_file->close();
}

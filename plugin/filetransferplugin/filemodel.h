#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <QObject>
#include "qdlt.h"
#include "form.h"
#include "file.h"
#include "globals.h"
#include <QTreeWidgetItem>
#include <QList>
#include <QItemDelegate>

class FileModel : public QObject
{
    Q_OBJECT
public:
    //explicit filemodel(QObject *parent = 0, Form *f = 0);
    FileModel();
    FileModel(Form *f);
    ~FileModel();
    void doFLST(QDltMsg *msg);
    void doFLDA(QDltMsg *msg);
    void doFLFI(QDltMsg *msg);
    void doFLIF(QDltMsg *msg);
    void doFLER(QDltMsg *msg);
    Form *form;

};

#endif // FILEMODEL_H

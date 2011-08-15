#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QMutex>

#include "project.h"
#include "qdlt.h"

#define DLT_VIEWER_LIST_BUFFER_SIZE 100024

extern "C"
{
        #include "dlt_common.h"
        #include "dlt_user_shared.h"
}

class TreeItem;

class TreeModel : public QAbstractTableModel
{
Q_OBJECT

public:
TreeModel(const QString &data, QObject *parent = 0);
~TreeModel();

QVariant data(const QModelIndex &index, int role) const;
QVariant headerData(int section, Qt::Orientation orientation,
         int role = Qt::DisplayRole) const;
int rowCount(const QModelIndex &parent = QModelIndex()) const;
int columnCount(const QModelIndex &parent = QModelIndex()) const;

/* pointer to the current loaded file */
QDltFile *qfile;
int size;
int showApIdDesc;
int showCtIdDesc;
QMutex *mutex;
Project *project;
void modelChanged();

};

#endif // TREEMODEL_H

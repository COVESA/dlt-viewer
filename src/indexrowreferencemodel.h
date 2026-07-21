#ifndef INDEXROWREFERENCEMODEL_H
#define INDEXROWREFERENCEMODEL_H

#include <QAbstractTableModel>
#include <QPointer>
#include <QVector>

class IndexRowReferenceModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit IndexRowReferenceModel(QObject *parent = nullptr);

    void setSourceModel(QAbstractItemModel *sourceModel);
    void setRowReferences(const QVector<int> &rows);

    int sourceRowAt(int row) const;
    const QVector<int>& rowReferences() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QPointer<QAbstractItemModel> sourceModel;
    QVector<int> sourceRows;
};

#endif // INDEXROWREFERENCEMODEL_H

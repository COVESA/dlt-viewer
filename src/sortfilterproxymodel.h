#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QObject>

class SortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum SortType
    {
        ALPHABETICALLY,
        TIMESTAMP
    };

public:
    SortFilterProxyModel(QObject *parent = 0);

    void changeSortingType(SortType type);
    void changeSortingOrder(Qt::SortOrder order);
protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
private:
    SortType sort_type = ALPHABETICALLY;
};

class EcuIdFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit EcuIdFilterProxyModel(QObject* parent = nullptr);

    void setEcuId(const QString& ecuId);
    void setEcuIdList(const QSet<QString> &ids);
    void setEcuColumn(int column);

protected:
    bool filterAcceptsRow(int row, const QModelIndex& parent) const override;

private:
    QString ecu;
    QSet<QString> ecuIdList;
    int ecuColumn = 4;
};

#endif // SORTFILTERPROXYMODEL_H

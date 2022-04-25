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

#endif // SORTFILTERPROXYMODEL_H

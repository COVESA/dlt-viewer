#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QObject>
#include <QSet>
#include <QString>

class QDltFile;
class SortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum SortType
    {
        ALPHABETICALLY,
        TIMESTAMP,
        INDEX
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
    void setCrlfFilter(bool enabled);
    void setDltFile(QDltFile* file);
    
    // Override data method to preserve original indices
    QVariant data(const QModelIndex &index, int role) const override;

protected:
    bool filterAcceptsRow(int row, const QModelIndex& parent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    QString ecu;
    QSet<QString> ecuIdList;
    int ecuColumn = 4;
    bool crlfFilterEnabled = false;
    QDltFile* dltFile = nullptr;
    
    // Helper function to check if message contains CRLF
    bool containsCrlf(int row, const QModelIndex& parent) const;
};

#endif // SORTFILTERPROXYMODEL_H

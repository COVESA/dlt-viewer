#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QObject>
#include <QSet>
#include <QString>

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

    /**
     * @brief Installs the source model and resets incremental filter index state.
     * @param sourceModel Source model to proxy.
     */
    void setSourceModel(QAbstractItemModel *sourceModel) override;

    void setEcuId(const QString& ecuId);
    void setEcuIdList(const QSet<QString> &ids);
    void setEcuColumn(int column);
    
    // Override data method to preserve original indices
    QVariant data(const QModelIndex &index, int role) const override;

protected:
    bool filterAcceptsRow(int row, const QModelIndex& parent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    /**
     * @brief Triggers filter invalidation using Qt-version-appropriate API.
     */
    void updateFilter();

    /**
     * @brief Evaluates whether one source row matches the current ECU criteria.
     * @param row Source row index.
     * @param parent Source parent index.
     * @return True if row matches filter conditions.
     */
    bool rowMatchesCurrentEcuFilter(int row, const QModelIndex &parent) const;

    /**
     * @brief Rebuilds the accepted-row index from the full source model.
     */
    void rebuildAcceptedRowsIndex();

    /**
     * @brief Incrementally extends the accepted-row index for a row range.
     * @param fromRow First source row.
     * @param toRow Last source row.
     */
    void buildAcceptedRowsIncremental(int fromRow, int toRow);

    /**
     * @brief Handles source model layout changes by rebuilding index state.
     */
    void onSourceLayoutChanged();

    /**
     * @brief Handles source model reset by clearing and rebuilding index state.
     */
    void onSourceModelReset();

    QString ecu;
    QSet<QString> ecuIdList;
    int ecuColumn = 4;
    mutable QSet<int> acceptedRows;
    mutable int indexedRowCount = 0;
    mutable bool indexReady = false;
    int knownSourceRowCount = 0;
};

#endif // SORTFILTERPROXYMODEL_H

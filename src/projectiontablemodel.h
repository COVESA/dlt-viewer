#ifndef PROJECTIONTABLEMODEL_H
#define PROJECTIONTABLEMODEL_H

#include <QAbstractTableModel>
#include <QPointer>

#include <vector>

class ProjectionTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    //! Create a projection model with an optional QObject parent.
    explicit ProjectionTableModel(QObject *parent = nullptr);

    //! Attach the source model used for projected row access.
    void setSourceModel(QAbstractItemModel *sourceModel);
    //! Return the currently attached source model.
    QAbstractItemModel *sourceModel() const;

    //! Replace the projected row mapping with source-row indices.
    void setProjectionRows(const std::vector<int> &rows);
    //! Clear the active row projection.
    void clearProjection();

    //! Translate a projected row index into the source row index.
    int sourceRowForRow(int row) const;

    //! Return projected row count.
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    //! Return column count from the source model.
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    //! Return projected cell data for the requested role.
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    //! Return header data from the source model.
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private slots:
    //! Forward relevant source data changes into projected ranges.
    void onSourceDataChanged(const QModelIndex &topLeft,
                             const QModelIndex &bottomRight,
                             const QVector<int> &roles);
    //! Forward source header changes.
    void onSourceHeaderDataChanged(Qt::Orientation orientation, int first, int last);
    //! Handle full source reset notifications.
    void onSourceModelReset();
    //! Handle source layout changes.
    void onSourceLayoutChanged();

private:
    //! Rewire all source-model signal connections.
    void reconnectSourceSignals(QAbstractItemModel *sourceModel);

    QPointer<QAbstractItemModel> m_sourceModel;
    std::vector<int> m_projectionRows;
};

#endif // PROJECTIONTABLEMODEL_H

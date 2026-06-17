#ifndef CRLFINDEXVIEWMODEL_H
#define CRLFINDEXVIEWMODEL_H

#include <QAbstractTableModel>

class QDltFile;

/**
 * @brief Table model wrapper for CRLF window rows.
 *
 * Maps CRLF-only row references to the original table model and exposes
 * message index metadata through user roles.
 */
class CrlfIndexViewModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the CRLF index model.
     * @param parent QObject parent.
     */
    explicit CrlfIndexViewModel(QObject *parent = nullptr);

    /**
     * @brief Sets the source table model used for data forwarding.
     * @param sourceModel Source model from the main viewer table.
     */
    void setSourceModel(QAbstractTableModel *sourceModel);

    /**
     * @brief Sets the DLT file used to resolve global message positions.
     * @param dltFile DLT file instance.
     */
    void setDltFile(QDltFile *dltFile);

    /**
     * @brief Replaces the mapped source-row references.
     * @param rowRefs Source model row indices representing CRLF matches.
     */
    void setSourceRowRefs(QVector<int> rowRefs);

    /**
     * @brief Clears all mapped rows.
     */
    void clear();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QAbstractTableModel *m_sourceModel{nullptr};
    QDltFile *m_dltFile{nullptr};
    QVector<int> m_sourceRowRefs;
};

#endif // CRLFINDEXVIEWMODEL_H

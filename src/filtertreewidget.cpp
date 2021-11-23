#include "filtertreewidget.h"
#include <QDropEvent>
#include <QMimeData>

FilterTreeWidget::FilterTreeWidget(QObject *parent) :
    QTreeWidget(qobject_cast<QWidget *>(parent))
{
}

void FilterTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void FilterTreeWidget::dropEvent(QDropEvent *event)
{
    QStringList types = event->mimeData()->formats();
    for(int i=0;i < types.size(); i++)
    {
        /* QT advertises our filteritems with this MIME identifier.
         * We use this to identify when a filter item was dropped,
         * instead of some other random data. */
        if(types[i] == "application/x-qabstractitemmodeldatalist")
        {
            QTreeWidget::dropEvent(event);
            emit filterItemDropped();
            event->accept();
            break;
         }
    }
}

void FilterTreeWidget::deleteSelected()
{
    // delete selected items
    // and select the first item that replaces the first deleted item

    QList<QTreeWidgetItem *> list = selectedItems();
    if (list.size() == 0)
        return;

    auto select_idx = this->indexOfTopLevelItem(list.at(0));
    for (auto& item : list) {
        auto idx = indexOfTopLevelItem(item);
        delete takeTopLevelItem(idx);
    }

    // select the item that replaced the first deleted item,
    // but handle the case where all following items were removed, in
    // that case select the last item available
    if (select_idx >= topLevelItemCount())
        select_idx = topLevelItemCount() - 1;

    setCurrentItem(topLevelItem(select_idx));

    emit filterCountChanged();
}

void FilterTreeWidget::keyReleaseEvent(QKeyEvent *event)
{
    // support some hotkeys in the filter list
    if (event->key() == Qt::Key::Key_Delete)
    {
        deleteSelected();
    }
}

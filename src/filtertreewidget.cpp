#include "filtertreewidget.h"
#include "project.h"
#include <QDropEvent>
#include <QMimeData>

FilterTreeWidget::FilterTreeWidget(QObject *parent) :
    QTreeWidget(qobject_cast<QWidget *>(parent))
{
}

void FilterTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    // block dragging a FilterGroupItem onto another FilterGroupItem
    QTreeWidgetItem *target = itemAt(event->pos());
    if(target && target->type() == filtergroup_type)
    {
        for(auto *dragged : selectedItems())
        {
            if(dragged->type() == filtergroup_type)
            {
                event->ignore();
                return;
            }
        }
    }
    event->accept();
}

void FilterTreeWidget::dropEvent(QDropEvent *event)
{
    // block group-into-group nesting before Qt processes the drop
    QTreeWidgetItem *target = itemAt(event->pos());
    if(target && target->type() == filtergroup_type)
    {
        for(auto *dragged : selectedItems())
        {
            if(dragged->type() == filtergroup_type)
            {
                event->ignore();
                return;
            }
        }
    }

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
    // delete selected items (handles both top-level and child items)
    QList<QTreeWidgetItem *> list = selectedItems();
    if (list.size() == 0)
        return;

    // Track first top-level index for post-delete selection
    int select_idx = indexOfTopLevelItem(list.at(0));
    if(select_idx < 0 && list.at(0)->parent())
        select_idx = indexOfTopLevelItem(list.at(0)->parent());

    for (auto& item : list) {
        QTreeWidgetItem *parent = item->parent();
        if(parent)
        {
            parent->removeChild(item);
            delete item;
        }
        else
        {
            int idx = indexOfTopLevelItem(item);
            if(idx >= 0)
                delete takeTopLevelItem(idx);
        }
    }

    // select the item that replaced the first deleted item
    if (select_idx >= topLevelItemCount())
        select_idx = topLevelItemCount() - 1;

    if(select_idx >= 0)
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

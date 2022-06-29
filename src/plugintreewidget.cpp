#include "plugintreewidget.h"
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>

PluginTreeWidget::PluginTreeWidget(QObject *parent) :
    QTreeWidget(qobject_cast<QWidget *>(parent))
{
    setDragDropMode(QAbstractItemView::InternalMove);
}

void PluginTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    const QTreeWidgetItem* target_item = this->itemAt(event->pos());

    // Ignore event if moving over a child item
    if(target_item != nullptr && target_item->parent() != nullptr){
        event->ignore();
    }
    else{
        QTreeWidget::dragMoveEvent(event);
    }
}


void PluginTreeWidget::dropEvent(QDropEvent *event)
{
    QTreeWidgetItem* dragged_item = this->currentItem();
    const QTreeWidgetItem* target_item = this->itemAt(event->pos());

    // Allow drop only over top level items (or at the end)
    if(target_item == nullptr || target_item->parent() == nullptr){
        QTreeWidget::dropEvent(event);
        this->setCurrentItem(dragged_item);
        int new_position = this->indexOfTopLevelItem(dragged_item);

        emit pluginOrderChanged(dragged_item->text(0), new_position);
    }
    else{
        event->ignore();
    }
}


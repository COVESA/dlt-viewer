#include "plugintreewidget.h"
#include "project.h"
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>

PluginTreeWidget::PluginTreeWidget(QObject *parent) :
    QTreeWidget(qobject_cast<QWidget *>(parent))
{
    setDragDropMode(QAbstractItemView::InternalMove);
}

void PluginTreeWidget::sortAccordingPriority(const QStringList& prio_list)
{
    int prio = 0;
    for(QStringList::const_iterator it = prio_list.constBegin(); it != prio_list.constEnd(); ++it) {
        if ( setPluginPriority(*it, prio) ) {
            emit pluginOrderChanged(*it, prio);
            ++prio;
        }
    }
}

bool PluginTreeWidget::setPluginPriority(const QString& name, unsigned int prio)
{
    for(int num = 0; num < topLevelItemCount(); num++) {
        PluginItem *pItem = (PluginItem*) topLevelItem(num);
        if(pItem->getName() == name)
        {
            if(prio != num)
            {
                takeTopLevelItem(num);
                insertTopLevelItem(prio, pItem);
            }
            return true;
        }
    }
    return false;
}

void PluginTreeWidget::raisePluginPriority(int index)
{
    if (index > 0) {
        QTreeWidgetItem *pItem = takeTopLevelItem(index);
        insertTopLevelItem(index - 1, pItem);
        setCurrentItem(pItem);

        emit pluginOrderChanged(((PluginItem*)pItem)->getName(), index - 1);
    }
}

void PluginTreeWidget::decreasePluginPriority(int index)
{
    if (index >= 0) {
        QTreeWidgetItem *pItem = takeTopLevelItem(index);
        insertTopLevelItem(index + 1, pItem);
        setCurrentItem(pItem);

        emit pluginOrderChanged(((PluginItem*)pItem)->getName(), index + 1);
    }
}

void PluginTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    const QTreeWidgetItem* dragged_item = this->currentItem();
    const QTreeWidgetItem* target_item = this->itemAt(event->pos());

    // Ignore event if moving over a child item
    if(dragged_item == nullptr || dragged_item->parent() != nullptr || (target_item != nullptr && target_item->parent() != nullptr)){
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

    // Avoid dropping children
    // Allow drop only over top level items (or at the end)
    if( dragged_item != nullptr && dragged_item->parent() == nullptr &&
         (target_item == nullptr || target_item->parent() == nullptr)){
        QTreeWidget::dropEvent(event);
        this->setCurrentItem(dragged_item);
        int new_position = this->indexOfTopLevelItem(dragged_item);

        emit pluginOrderChanged(((PluginItem*)dragged_item)->getName(), new_position);
    }
    else{
        event->ignore();
    }
}


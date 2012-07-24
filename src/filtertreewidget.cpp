#include "filtertreewidget.h"
#include <QDropEvent>

FilterTreeWidget::FilterTreeWidget(QObject *parent) :
    QTreeWidget(qobject_cast<QWidget *>(parent))
{
    filterButton = 0;

}

void FilterTreeWidget::setFilterButton(QPushButton *btn)
{
    filterButton = btn;
}

void FilterTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
    if(filterButton)
    {
        filterButton->setIcon(QIcon(":/toolbar/png/weather-storm.png"));
        filterButton->setText("Filters changed. Please enable filtering.");

        filterButton->setChecked(Qt::Unchecked);
        filterButton->setText("Enable filters");
    }
}


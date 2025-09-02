#include "dlttableview.h"

#include <QDebug>
#include <QWheelEvent>

DltTableView::DltTableView(QWidget *parent) :
    QTableView(parent)
{
}

/*!
    Paints the table on receipt of the given paint event \a event.
*/
void DltTableView::paintEvent(QPaintEvent *event)
{
    if(paintMutex.tryLock())
    {
        QTableView::paintEvent(event);
        paintMutex.unlock();
    }
}

void DltTableView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        auto val = event->angleDelta().y();
        emit changeFontSize((0 < val) - (val < 0));
        event->accept();
    } else {
        QTableView::wheelEvent(event);
    }
}

void DltTableView::lock()
{
    paintMutex.lock();
}

void DltTableView::unlock()
{
    paintMutex.unlock();
}

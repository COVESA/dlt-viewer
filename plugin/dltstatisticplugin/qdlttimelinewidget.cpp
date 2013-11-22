#include <QPainter>
#include <QModelIndexList>
#include <time.h>

#include "qdlttimelinewidget.h"

QDltTimelineWidget::QDltTimelineWidget(QWidget *parent) :
    QWidget(parent)
{
    borderTop = 12;
    borderBottom = 22;
    borderLeft = 12;
    borderRight = 12;

    clear();
}

void QDltTimelineWidget::clear()
{
    cursorTime1 = 0;
    cursorTime2 = 0;

    beginTime = 0;
    endTime = 0;

    data.clear();

    maxValue = 0;
}

void QDltTimelineWidget::paintEvent ( QPaintEvent * event )
{
    Q_UNUSED(event);
    QPainter qp(this);

    // show debug info
    if(false)
    {
        QFont pen_debug("Courier New",8);
        qp.setFont(pen_debug);
        QPen font_debug(Qt::red, 1, Qt::SolidLine);
        qp.setPen(font_debug);
        QString text;
        text += QString("width|height: %1|%2\n").arg(frameSize().width()).arg(frameSize().height());
        text += QString("beginTime|endTime: %1|%2\n").arg(beginTime).arg(endTime);
        text += QString("data.size: %1\n").arg(data.size());
        text += QString("maxValue: %1\n").arg(maxValue);
        qp.drawText(20,20,frameSize().width(),frameSize().height(),Qt::AlignLeft|Qt::AlignTop,text );
    }

    // show empty Text if empty
    if(data.isEmpty())
    {
        // no data to display
        QFont font_empty("Courier New",8);
        QPen font_pen(Qt::blue, 1, Qt::SolidLine);
        qp.setPen(font_pen);
        qp.setFont(font_empty);
        qp.drawText(0,0,frameSize().width(),frameSize().height(),Qt::AlignHCenter|Qt::AlignVCenter,QString("Empty") );
        return;
    }

    // paint x scale
    QPen pen_scalex(Qt::blue, 1, Qt::SolidLine);
    qp.setPen(pen_scalex);
    QFont font_scalex("Courier New",8);
    qp.setFont(font_scalex);
    for(time_t time=beginTime;time<=endTime;time++)
    {
        if(time%60==0)
            qp.drawLine(toX(time), frameSize().height()-borderBottom+2, toX(time) ,frameSize().height()-borderBottom+12);
        else
            qp.drawLine(toX(time), frameSize().height()-borderBottom+2, toX(time) ,frameSize().height()-borderBottom+7);
    }
    qp.drawText(toX(beginTime), 0, frameSize().width() ,frameSize().height(),Qt::AlignLeft|Qt::AlignBottom,timeString(beginTime));
    qp.drawText(0, 0, toX(endTime) ,frameSize().height(),Qt::AlignRight|Qt::AlignBottom,timeString(endTime));
    qp.drawText(toX(beginTime), 0, toX(endTime),frameSize().height(),Qt::AlignHCenter|Qt::AlignBottom,timeString(beginTime+(endTime-beginTime)/2));

    // paint y scale
    QPen pen_scaley(Qt::blue, 1, Qt::SolidLine);
    qp.setPen(pen_scaley);
    QFont font_scaley("Courier New",8);
    qp.setFont(font_scaley);
    for(int num=0;num<=maxValue;num++)
    {
        qp.drawLine(0,toY(num), 5 ,toY(num));
    }
    qp.drawLine(0, toY(0), 10 ,toY(0));
    qp.drawLine(0, toY(maxValue/2), 10 ,toY(maxValue/2));
    qp.drawText(12,toY(maxValue/2),frameSize().width(),toY(maxValue/2),Qt::AlignLeft,QString("%1").arg(maxValue/2) );
    qp.drawLine(0, toY(maxValue), 10 ,toY(maxValue));
    qp.drawText(12,toY(maxValue),frameSize().width(),toY(maxValue),Qt::AlignLeft,QString("%1 %2").arg(maxValue).arg(yUnitText) );

    // paint all values
    QPen font_pen(Qt::black, 1, Qt::SolidLine);
    qp.setPen(font_pen);
    QMapIterator<time_t,int> i(data);
    int x=-1,y=0;
    while (i.hasNext()) {
        i.next();
        if(toX(i.key())==x )
        {
            y = toY(i.value());
        }
        else
        {
            if(x!=-1)
                qp.drawLine(x, toY(0), x,y);
            x = toX(i.key());
            y = toY(i.value());
        }
    }
    qp.drawLine(x, toY(0), x,y);

    // paint cursor
    QString text;
    if(cursorTime1)
    {
        QPen pen_cursor(Qt::red, 1, Qt::DashLine);
        qp.setPen(pen_cursor);
        QFont font_cursor("Courier New",8);
        qp.setFont(font_cursor);
        qp.drawLine( toX(cursorTime1) , borderTop , toX(cursorTime1) , frameSize().height()-borderBottom );
        qp.drawText( toX(cursorTime1) +2 , borderTop , frameSize().width() , frameSize().height()-borderBottom , Qt::AlignLeft|Qt::AlignTop,QString("1"));
        text += timeString(cursorTime1);
    }
    if(cursorTime2)
    {
        QPen pen_cursor(Qt::red, 1, Qt::DashLine);
        qp.setPen(pen_cursor);
        QFont font_cursor("Courier New",8);
        qp.setFont(font_cursor);
        qp.drawLine( toX(cursorTime2) , borderTop , toX(cursorTime2) , frameSize().height()-borderBottom );
        qp.drawText( toX(cursorTime2) +2 , borderTop , frameSize().width() , frameSize().height()-borderBottom , Qt::AlignLeft|Qt::AlignTop,QString("2"));
        text += QString(", ") + timeString(cursorTime2);
    }
    if(cursorTime1 || cursorTime2)
    {
        QFont font_cursor("Courier New",8);
        qp.setFont(font_cursor);
        if(!cursorTime2)
            qp.drawText( 0 , 0 , frameSize().width() , frameSize().height() , Qt::AlignRight|Qt::AlignTop,QString("(%1)").arg(timeString(cursorTime1)));
        else if(!cursorTime1)
            qp.drawText( 0 , 0 , frameSize().width() , frameSize().height() , Qt::AlignRight|Qt::AlignTop,QString("(%1)").arg(timeString(cursorTime2)));
        else if(cursorTime1>cursorTime2)
            qp.drawText( 0 , 0 , frameSize().width() , frameSize().height() , Qt::AlignRight|Qt::AlignTop,QString("(%1 - %2 , %3s)").arg(timeString(cursorTime2)).arg(timeString(cursorTime1)).arg(cursorTime1-cursorTime2));
        else
            qp.drawText( 0 , 0 , frameSize().width() , frameSize().height() , Qt::AlignRight|Qt::AlignTop,QString("(%1 - %2 , %3s)").arg(timeString(cursorTime1)).arg(timeString(cursorTime2)).arg(cursorTime2-cursorTime1));

    }
}

void QDltTimelineWidget::mousePressEvent ( QMouseEvent * event )
{
    if(data.isEmpty())
        return;

    if(event->buttons()&Qt::LeftButton)
    {
        cursorTime1 = toTime(event->pos().x());
        emit cursorTime1Changed(cursorTime1);
    }

    update();
}

void QDltTimelineWidget::contextMenuEvent ( QContextMenuEvent * event )
{
    if(data.isEmpty())
        return;

    cursorTime2 = toTime(event->pos().x());
    emit cursorTime2Changed(cursorTime2);

    update();
}

void QDltTimelineWidget::setBeginTime(time_t time)
{
    beginTime = time;

    update();
}

void QDltTimelineWidget::setEndTime(time_t time)
{
    endTime = time;

    update();

}

void QDltTimelineWidget::setCursorTime1(time_t time)
{
    cursorTime1 = time;

    update();

}

void QDltTimelineWidget::setCursorTime2(time_t time)
{
    cursorTime2 = time;

    update();

}

void QDltTimelineWidget::setData(QMap<time_t,int> &_data)
{
    data = _data;

    // calculate maxValue
    maxValue = 0;
    QMapIterator<time_t,int> imax(data);
    while (imax.hasNext()) {
        imax.next();

        if(imax.value()>maxValue)
            maxValue = imax.value();
    }

    update();
}

QString QDltTimelineWidget::timeString(time_t time)
{
    char strtime[256];
    strtime[0]=0;
    struct tm *time_tm;
    time_tm = localtime(&time);
    if(time_tm)
        strftime(strtime, 256, "%Y/%m/%d %H:%M:%S", time_tm);
    return QString(strtime);
}

int QDltTimelineWidget::toX(time_t time)
{
    return borderLeft + ((time - beginTime) * (frameSize().width()-borderLeft-borderRight) /
           (endTime - beginTime));
}

int QDltTimelineWidget::toY(int value)
{
    return (frameSize().height() - borderBottom) - (value * (frameSize().height()-borderTop-borderBottom) / maxValue);
}

time_t QDltTimelineWidget::toTime(int x)
{
    if(x>=toX(endTime))
        return endTime;

    if(x<=toX(beginTime))
        return beginTime;

    return beginTime + ((x - borderLeft)*(endTime - beginTime)/(frameSize().width()-borderLeft-borderRight));
}

int QDltTimelineWidget::toValue(int y)
{
    if(y>=toY(0))
        return 0;

    if(y<=toY(maxValue))
        return maxValue;

    return maxValue - ((y - borderTop)* maxValue/(frameSize().height()-borderTop-borderBottom));

}


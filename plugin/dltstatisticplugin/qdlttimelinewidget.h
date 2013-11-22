#ifndef QDLTTIMELINEWIDGET_H
#define QDLTTIMELINEWIDGET_H

#include "qdlt.h"

#include <QWidget>
#include <QMouseEvent>
#include <QMap>

class QDltTimelineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QDltTimelineWidget(QWidget *parent = 0);

    void clear();

    void setBeginTime(time_t time);
    void setEndTime(time_t time);

    void setCursorTime1(time_t time);
    void setCursorTime2(time_t time);

    void setYUnitText(QString text) { yUnitText = text; }
    void setXUnitText(QString text) { xUnitText = text; }

    void setData(QMap<time_t,int> &_data);

protected:

    void paintEvent ( QPaintEvent * event );
    void mousePressEvent ( QMouseEvent * event );
    void contextMenuEvent ( QContextMenuEvent * event );

signals:
    
    void cursorTime1Changed(time_t time);
    void cursorTime2Changed(time_t time);

public slots:

private:

    QString stringToHtml(QString str);

    QString timeString(time_t time);
    int toX(time_t time);
    int toY(int value);

    time_t toTime(int x);
    int toValue(int y);

    int borderTop,borderBottom,borderLeft,borderRight;

    time_t beginTime;
    time_t endTime;

    time_t cursorTime1;
    time_t cursorTime2;

    QString yUnitText;
    QString xUnitText;

    int maxValue;

    QMap<time_t,int> data;


};

#endif // QDLTTIMELINEWIDGET_H

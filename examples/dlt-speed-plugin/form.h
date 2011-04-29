#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include <qwt_color_map.h>
#include "plugininterface.h"
namespace Ui {
    class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

    void setSpeedLCD(QDltArgument currentSpeed,unsigned int time);

private:
    Ui::Form *ui;

    QwtPlotCurve *curve1;
    QwtPointArrayData *dataArray;

    QVector< double > speedX;
    QVector< double > timeY;
};

#endif // FORM_H

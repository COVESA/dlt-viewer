#include "dltuiutils.h"

DltUiUtils::DltUiUtils()
{
}



QColor DltUiUtils::optimalTextColor(QColor forColor)
{
    int d = 0;

    // Counting the perceptive luminance - human eye favors green color...
    double a = 1 - ( 0.299 * forColor.redF() + 0.587 * forColor.green() + 0.114 * forColor.blue())/255;

    if (a < 0.5)
    {
        d = 0; // bright colors - black font
    }
    else
    {
        d = 255; // dark colors - white font
    }
    return  QColor(d,d,d);
}

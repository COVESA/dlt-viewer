#include "dltuiutils.h"

DltUiUtils::DltUiUtils()
{
}



QColor DltUiUtils::optimalTextColor(QColor forColor)
{
    int d = 0;

    // Counting the perceptive luminance - human eye favors green color...
    int a = ( 0.299 * forColor.red() + 0.587 * forColor.green() + 0.114 * forColor.blue());

    if (a > 166)
    {
        d = 0; // bright colors - black font
    }
    else
    {
        d = 255; // dark colors - white font
    }
    return  QColor(d,d,d);
}

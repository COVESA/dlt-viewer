#ifndef DLTUIUTILS_H
#define DLTUIUTILS_H

#include <QColor>
class DltUiUtils
{
public:
  DltUiUtils();

  // http://stackoverflow.com/questions/1855884/determine-font-color-based-on-background-color
  static QColor optimalTextColor(QColor forColor);
};

#endif // DLTUIUTILS_H

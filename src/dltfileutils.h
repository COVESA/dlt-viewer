#ifndef DLTFILEUTILS_H
#define DLTFILEUTILS_H

#include <QDir>
#include "settingsdialog.h"

class DltFileUtils : QObject
{
    Q_OBJECT
public:
    DltFileUtils();
    static QString createTempFile(QDir path);
    static QDir getTempPath(SettingsDialog *settings);
};

#endif // DLTFILEUTILS_H

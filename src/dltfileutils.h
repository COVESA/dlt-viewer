#ifndef DLTFILEUTILS_H
#define DLTFILEUTILS_H

#include <QDir>

class DltFileUtils : QObject
{
    Q_OBJECT
public:
    DltFileUtils();
    static QString createTempFile(QDir path,  bool silentmode);
    static QDir getTempPath(bool silentmode);
};

#endif // DLTFILEUTILS_H

#ifndef DLTSETTINGSMANAGER_H
#define DLTSETTINGSMANAGER_H

#include <qsettings.h>

class DltSettingsManager
{
// Singleton pattern
public:
    static DltSettingsManager* instance();
    static void close();

private:
    DltSettingsManager();
    ~DltSettingsManager();
    static DltSettingsManager *m_instance;
    QSettings *settings;

// QSettings delegates
public:
    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    void clear();
    QString fileName() const;

};

#endif // DLTSETTINGSMANAGER_H

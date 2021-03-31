#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QString>

class Configuration
{
public:
    Configuration();
    ~Configuration();

    QString getFlstTag();
    QString getFldaTag();
    QString getFlfiTag();
    QString getFlerTag();
    QString getFlifTag();
    QString getFlCtIdTag();
    QString getAutoSavePath();

    void setFlstTag(QString newTag);
    void setFldaTag(QString newTag);
    void setFlfiTag(QString newTag);
    void setFlerTag(QString newTag);
    void setFlifTag(QString newTag);
    void setFlCtIdTag(QString newTag);
    void setAutoSavePath(QString newTag);

    void setDefault();

private:
    QString tagFLST;
    QString tagFLDA;
    QString tagFLFI;
    QString tagFLER;
    QString tagFLIF;
    QString tagFLCtId;
    QString AutoSavePath;

};

#endif // CONFIGURATION_H

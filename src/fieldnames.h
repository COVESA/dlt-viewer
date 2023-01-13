#ifndef FIELDNAMES_H
#define FIELDNAMES_H

#include "qdltsettingsmanager.h"
#include <QObject>
#include <QString>

class FieldNames : public QObject
{
    Q_OBJECT
public:
    explicit FieldNames(QObject *parent = 0);

    /* Order of columns */
    enum Fields
    {
        Index,
        Time,
        TimeStamp,
        Counter,
        EcuId,
        AppId,
        ContextId,
        SessionId,
        Type,
        Subtype,
        Mode,
        MessageId,
        ArgCount,
        Payload,
        Arg0  //Arg0 must always be last field or tablemodels have to be changed as well
    };

    /* Get name for one column/field.
     * Settings needed for App and Context Description change.
     * Pass null for settings to get ID only always.
     * Use case: Exports */
    static QString getName(Fields cn, QDltSettingsManager *settings = NULL);
    static int getColumnWidth(Fields cn, QDltSettingsManager *settings = NULL);
    static QVariant getColumnAlignment(Fields cn, QDltSettingsManager *settings = NULL);
    static bool getColumnShown(Fields cn,QDltSettingsManager *settings = NULL);

signals:
    
public slots:
    
};

#endif // FIELDNAMES_H

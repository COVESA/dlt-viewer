#include "fieldnames.h"

FieldNames::FieldNames(QObject *parent) :
    QObject(parent)
{
}

QString FieldNames::getName(Fields cn, QDltSettingsManager *settings)
{
    switch(cn)
    {
    case Index:
        return QString("Index");
    case Time:
        return QString("Time");
    case TimeStamp:
        return QString("Timestamp");
    case Counter:
        return QString("Count");
    case EcuId:
        return QString("Ecuid");
    case AppId:
        if(settings == NULL)
        {
            return QString("Apid");
        }
        switch(settings->showApIdDesc){
        case 0:
             return QString("Apid");
        case 1:
             return QString("Apid Desc");
        }
    case ContextId:
        if(settings == NULL)
        {
            return QString("Ctid");
        }
        switch(settings->showCtIdDesc){
        case 0:
             return QString("Ctid");
        case 1:
             return QString("Ctid Desc");
        }
    case SessionId:
        if(settings == NULL)
        {
            return QString("SessionId");
        }
        switch(settings->showSessionName){
        case 0:
             return QString("SessionId");
        case 1:
             return QString("SessionName");
        }
    case Type:
        return QString("Type");
    case Subtype:
        return QString("Subtype");
    case Mode:
        return QString("Mode");
    case ArgCount:
        return QString("#Args");
    case Payload:
        return QString("Payload");
    default:
        if (cn>=Arg0)
        {
            return QString("Arg%1").arg(cn-Arg0);
        }
        else
        return QString();
    }
}

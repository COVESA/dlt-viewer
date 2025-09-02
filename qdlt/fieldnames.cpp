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
    case MessageId:
        return QString("MsgID");
    default:
        if (cn>=Arg0)
        {
            return QString("Arg%1").arg(cn-Arg0);
        }
        else
        return QString();
    }
}

int FieldNames::getColumnWidth(Fields cn, QDltSettingsManager *settings)
{
    Q_UNUSED(settings)
    int width;
    switch(cn)
    {
    case FieldNames::Index       : width=  50;break;
    case FieldNames::Time        : width= 150;break;
    case FieldNames::TimeStamp   : width=  70;break;
    case FieldNames::Counter     : width=  40;break;
    case FieldNames::EcuId       : width=  40;break;
    case FieldNames::AppId       : width=  40;break;
    case FieldNames::ContextId   : width=  40;break;
    case FieldNames::SessionId   : width=  70;break;
    case FieldNames::Type        : width=  50;break;
    case FieldNames::Subtype     : width=  50;break;
    case FieldNames::Mode        : width=  50;break;
    case FieldNames::ArgCount    : width=  40;break;
    case FieldNames::Payload     : width=1200;break;
    case FieldNames::MessageId   : width=  90;break;
    default                      : width=  50;break;
    }
    return width;
}

QVariant FieldNames::getColumnAlignment(Fields cn, QDltSettingsManager *settings)
{
    switch(cn)
    {
        case FieldNames::Index:         return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
        case FieldNames::Time:          return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
        case FieldNames::TimeStamp:     return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
        case FieldNames::Counter:       return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
        case FieldNames::EcuId:         return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
        case FieldNames::AppId:
            switch(settings->showApIdDesc)
            {
            case 0:  return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 1:  return QVariant(Qt::AlignLeft   | Qt::AlignVCenter);
            default: return QVariant(Qt::AlignLeft   | Qt::AlignVCenter);
            }
        case FieldNames::ContextId:
            switch(settings->showCtIdDesc)
            {
            case 0:  return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 1:  return QVariant(Qt::AlignLeft   | Qt::AlignVCenter);
            default: return QVariant(Qt::AlignLeft   | Qt::AlignVCenter);
            }
        case FieldNames::Type:           return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
        case FieldNames::Subtype:        return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
        case FieldNames::Mode:           return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
        case FieldNames::ArgCount:       return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
        case FieldNames::Payload:        return QVariant(Qt::AlignLeft   | Qt::AlignVCenter);
        case FieldNames::MessageId:      return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
        case FieldNames::SessionId:      return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
        default:                         return QVariant(Qt::AlignLeft   | Qt::AlignVCenter);
    }

}
bool FieldNames::getColumnShown(Fields cn,QDltSettingsManager *settings)
{
    switch (cn)
    {
        case(FieldNames::Index       ):return  settings->showIndex     ;
        case(FieldNames::Time        ):return  settings->showTime      ;
        case(FieldNames::TimeStamp   ):return  settings->showTimestamp ;
        case(FieldNames::Counter     ):return  settings->showCount     ;
        case(FieldNames::EcuId       ):return  settings->showEcuId     ;
        case(FieldNames::AppId       ):return  settings->showApId      ;
        case(FieldNames::ContextId   ):return  settings->showCtId      ;
        case(FieldNames::SessionId   ):return  settings->showSessionId ;
        case(FieldNames::Type        ):return  settings->showType      ;
        case(FieldNames::Subtype     ):return  settings->showSubtype   ;
        case(FieldNames::Mode        ):return  settings->showMode      ;
        case(FieldNames::ArgCount    ):return  settings->showNoar      ;
        case(FieldNames::Payload     ):return  settings->showPayload   ;
        case(FieldNames::MessageId   ):return  settings->showMsgId     ;
        default: return true;
	}
return true;
}

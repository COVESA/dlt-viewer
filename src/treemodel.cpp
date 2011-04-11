#include <QtGui>
#include <qmessagebox.h>

#include "treemodel.h"

char buffer[DLT_VIEWER_LIST_BUFFER_SIZE];

 TreeModel::TreeModel(const QString & /*data*/, QObject *parent)
     : QAbstractTableModel(parent)
 {
     size = 0;
 }

 TreeModel::~TreeModel()
 {

 }

 int TreeModel::columnCount(const QModelIndex & /*parent*/) const
 {
     return 12;
 }

 QVariant TreeModel::data(const QModelIndex &index, int role) const
 {
     QDltMsg msg;
     QByteArray buf;

     if (!index.isValid())
         return QVariant();

     if (index.row() >= size && index.row()<0)
         return QVariant();

     if (role == Qt::DisplayRole)
     {
         /* get the message with the selected item id */
         buf = qfile->getMsgFilter(index.row());
         msg.setMsg(buf);
         for(int num = 0; num < project->plugin->topLevelItemCount (); num++)
         {
             PluginItem *item = (PluginItem*)project->plugin->topLevelItem(num);

             if(item->plugindecoderinterface && item->plugindecoderinterface->isMsg(msg))
             {
                 item->plugindecoderinterface->decodeMsg(msg);
                 break;
             }
         }

         switch(index.column())
         {
         case 0:
             /* display index */
             return QString("%1").arg(index.row());
         case 1:
             return QString("%1.%2").arg(msg.getTime().toString("yyyy/MM/dd hh:mm:ss")).arg(msg.getMicroseconds(),6,10,QLatin1Char('0'));
         case 2:
             return QString("%1.%2").arg(msg.getTimestamp()/10000).arg(msg.getTimestamp()%10000,4,10,QLatin1Char('0'));
         case 3:
             return QString("%1").arg(msg.getMessageCounter());
         case 4:
             return msg.getEcuid();
         case 5:
             return msg.getApid();
         case 6:
             return msg.getCtid();
         case 7:
             return msg.getTypeString();
         case 8:
             return msg.getSubtypeString();
         case 9:
             return msg.getModeString();
         case 10:
             return QString("%1").arg(msg.getNumberOfArguments());
         case 11:
             /* display payload */
             return msg.toStringPayload();
         }
     }

     if ( role == Qt::BackgroundRole ) {
         buf = qfile->getMsgFilter(index.row());
         msg.setMsg(buf);
         for(int num = 0; num < project->plugin->topLevelItemCount (); num++)
         {
             PluginItem *item = (PluginItem*)project->plugin->topLevelItem(num);

             if(item->plugindecoderinterface && item->plugindecoderinterface->isMsg(msg))
             {
                 item->plugindecoderinterface->decodeMsg(msg);
                 break;
             }
         }
         if(qfile->checkMarker(msg))
            return QVariant(QBrush(QColor(0,255,0)));
         else
             return QVariant(QBrush(QColor(255,255,255)));
     }

     if ( role == Qt::TextAlignmentRole ) {
        switch(index.column())
        {
            case 0:
                return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
            case 1:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 2:
                return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
            case 3:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 4:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 5:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 6:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 7:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 8:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 9:
                return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
            case 10:
                return QVariant(Qt::AlignRight  | Qt::AlignVCenter);
            case 11:
                return QVariant(Qt::AlignLeft   | Qt::AlignVCenter);
        }
    }

     return QVariant();
 }

  QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
 {
      if (role != Qt::DisplayRole)
          return QVariant();

      if (orientation == Qt::Horizontal)
      {
          switch(section)
          {
          case 0:
              return QString("Index");
          case 1:
              return QString("Time");
          case 2:
              return QString("Timestamp");
          case 3:
              return QString("Count");
          case 4:
              return QString("Ecuid");
          case 5:
              return QString("Apid");
          case 6:
              return QString("Ctid");
          case 7:
              return QString("Type");
          case 8:
              return QString("Subtype");
          case 9:
              return QString("Mode");
          case 10:
              return QString("#Args");
          case 11:
              return QString("Payload");
          }
      }

      return QVariant();
  }

 int TreeModel::rowCount(const QModelIndex & /*parent*/) const
 {
     return size;
 }

 void TreeModel::modelChanged()
 {
     QModelIndex lIndex = index(0, 1);
     QModelIndex lLeft = index(size-1, 0);
     QModelIndex lRight = index(size-1, columnCount() - 1);
     emit(layoutChanged());
 }

#ifndef ECUDIALOG_H
#define ECUDIALOG_H

#include <QDialog>

namespace Ui {
    class EcuDialog;
}

class EcuDialog : public QDialog {
    Q_OBJECT
public:
    EcuDialog(QString id,QString description,int interfacetype,QString hostname,unsigned int tcpport,QString port,int baudrate,
              int loglevel, int tracestatus, int verbosemode, bool sendSerialHeaderTcp, bool sendSerialHeaderSerial,bool syncSerialHeaderTcp, bool syncSerialHeaderSerial,
              bool timingPackets, bool sendGetLogInfo, bool update, QWidget *parent = 0);
    ~EcuDialog();

    QString id();
    QString description();
    int interfacetype();
    QString hostname();
    unsigned int tcpport();
    QString port();
    int baudrate();
    int loglevel();
    int tracestatus();
    int verbosemode();
    int sendSerialHeaderTcp();
    int sendSerialHeaderSerial();
    int syncSerialHeaderTcp();
    int syncSerialHeaderSerial();
    int timingPackets();
    int sendGetLogInfo();

    int update();

    QStringList getHostnameList();
    void setHostnameList(QStringList hostnames);

    QStringList getPortList();
    void setPortList(QStringList ports);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::EcuDialog *ui;
};

#endif // ECUDIALOG_H

#include "dialog.h"
#include "ui_dialog.h"
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtCore/QVariant>
#include "qextserialport.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

#ifdef Q_OS_WIN
    ui->portBox->addItems(QStringList()<<QLatin1String("COM1")<<QLatin1String("COM2")<<QLatin1String("COM3")<<QLatin1String("COM4"));
#else
    ui->portBox->addItems(QStringList()<<QLatin1String("/dev/ttyS0")<<QLatin1String("/dev/ttyS1")<<QLatin1String("/dev/ttyUSB0")<<QLatin1String("/dev/ttyUSB1"));
#endif
    //make sure user can input their own port name!
    ui->portBox->setEditable(true);

    ui->baudRateBox->addItem(QLatin1String("1200"), BAUD1200);
    ui->baudRateBox->addItem(QLatin1String("2400"), BAUD2400);
    ui->baudRateBox->addItem(QLatin1String("4800"), BAUD4800);
    ui->baudRateBox->addItem(QLatin1String("9600"), BAUD9600);
    ui->baudRateBox->addItem(QLatin1String("19200"), BAUD19200);
    ui->baudRateBox->setCurrentIndex(3);

    ui->parityBox->addItem(QLatin1String("NONE"), PAR_NONE);
    ui->parityBox->addItem(QLatin1String("ODD"), PAR_ODD);
    ui->parityBox->addItem(QLatin1String("EVEN"), PAR_EVEN);

    ui->dataBitsBox->addItem(QLatin1String("5"), DATA_5);
    ui->dataBitsBox->addItem(QLatin1String("6"), DATA_6);
    ui->dataBitsBox->addItem(QLatin1String("7"), DATA_7);
    ui->dataBitsBox->addItem(QLatin1String("8"), DATA_8);
    ui->dataBitsBox->setCurrentIndex(3);

    ui->stopBitsBox->addItem(QLatin1String("1"), STOP_1);
    ui->stopBitsBox->addItem(QLatin1String("2"), STOP_2);

    ui->queryModeBox->addItem(QLatin1String("Polling"), QextSerialPort::Polling);
    ui->queryModeBox->addItem(QLatin1String("EventDriven"), QextSerialPort::EventDriven);

    ui->led->turnOff();

    timer = new QTimer(this);
    timer->setInterval(40);
    PortSettings settings = {BAUD9600, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
    port = new QextSerialPort(ui->portBox->currentText(), settings, QextSerialPort::Polling);

    connect(ui->baudRateBox, SIGNAL(currentIndexChanged(int)), SLOT(onBaudRateChanged(int)));
    connect(ui->parityBox, SIGNAL(currentIndexChanged(int)), SLOT(onParityChanged(int)));
    connect(ui->dataBitsBox, SIGNAL(currentIndexChanged(int)), SLOT(onDataBitsChanged(int)));
    connect(ui->stopBitsBox, SIGNAL(currentIndexChanged(int)), SLOT(onStopBitsChanged(int)));
    connect(ui->queryModeBox, SIGNAL(currentIndexChanged(int)), SLOT(onQueryModeChanged(int)));
    connect(ui->timeoutBox, SIGNAL(valueChanged(int)), SLOT(onTimeoutChanged(int)));
    connect(ui->portBox, SIGNAL(editTextChanged(QString)), SLOT(onPortNameChanged(QString)));
    connect(ui->openCloseButton, SIGNAL(clicked()), SLOT(onOpenCloseButtonClicked()));
    connect(ui->sendButton, SIGNAL(clicked()), SLOT(onSendButtonClicked()));
    connect(timer, SIGNAL(timeout()), SLOT(onReadyRead()));
    connect(port, SIGNAL(readyRead()), SLOT(onReadyRead()));

    setWindowTitle(QLatin1String("QextSerialPort Demo"));
}

Dialog::~Dialog()
{
    delete ui;
    delete port;
}

void Dialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Dialog::onPortNameChanged(const QString & /*name*/)
{
    if (port->isOpen()) {
        port->close();
        ui->led->turnOff();
    }
}

void Dialog::onBaudRateChanged(int idx)
{
    port->setBaudRate((BaudRateType)ui->baudRateBox->itemData(idx).toInt());
}

void Dialog::onParityChanged(int idx)
{
    port->setParity((ParityType)ui->parityBox->itemData(idx).toInt());
}

void Dialog::onDataBitsChanged(int idx)
{
    port->setDataBits((DataBitsType)ui->dataBitsBox->itemData(idx).toInt());
}

void Dialog::onStopBitsChanged(int idx)
{
    port->setStopBits((StopBitsType)ui->stopBitsBox->itemData(idx).toInt());
}

void Dialog::onQueryModeChanged(int idx)
{
    port->setQueryMode((QextSerialPort::QueryMode)ui->queryModeBox->itemData(idx).toInt());
}

void Dialog::onTimeoutChanged(int val)
{
    port->setTimeout(val);
}

void Dialog::onOpenCloseButtonClicked()
{
    if (!port->isOpen()) {
        port->setPortName(ui->portBox->currentText());
        port->open(QIODevice::ReadWrite);
    }
    else {
        port->close();
    }

    if (port->isOpen()) {
        if (port->queryMode() == QextSerialPort::Polling)
            timer->start();
        ui->led->turnOn();
    }
    else {
        timer->stop();
        ui->led->turnOff();
    }
}

void Dialog::onSendButtonClicked()
{
    if (port->isOpen() && !ui->sendEdit->toPlainText().isEmpty())
        port->write(ui->sendEdit->toPlainText().toLatin1());
}

void Dialog::onReadyRead()
{
    //EventDriven test.
    if (qApp->arguments().contains(QLatin1String("--debug")) && port->queryMode()==QextSerialPort::EventDriven){
        ui->recvEdit->appendPlainText(QString::number(port->bytesAvailable()));
        return;
    }

    if (port->bytesAvailable()) {
        ui->recvEdit->moveCursor(QTextCursor::End);
        ui->recvEdit->insertPlainText(QString::fromLatin1(port->readAll()));
    }
}

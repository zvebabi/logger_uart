#include "uartdevice.h"

uartDevice::uartDevice(QObject *parent)
{
    device = new QSerialPort(this);
    connect(device, &QSerialPort::readyRead, this, &uartDevice::readData);
}

uartDevice::~uartDevice()
{
    qDebug() << "uartdevice destructor";
    if (device != NULL)
    {
        device->disconnect();
        delete device;
        qDebug() << "call disconnect port";
    }
}

void uartDevice::getListOfPort()
{
    ports.clear();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        ports.push_back(info.portName());
        emit sendPortName(info.portName());
        qDebug() << info.portName();
    }
    std::stringstream ss;
    ss << "Found " << ports.size() << " ports";
    emit sendDebugInfo(QString(ss.str().c_str()), 100);
}

bool uartDevice::initDevice(const QString &port, const QString &baudRate)
{
    device->setPortName(port);
    device->setBaudRate(baudRate.toInt());
    device->setDataBits(QSerialPort::Data8);
    device->setParity(QSerialPort::NoParity);
    device->setStopBits(QSerialPort::OneStop);
    device->setFlowControl(QSerialPort::NoFlowControl);
    if(device->open(QIODevice::ReadWrite)){
        qDebug() << "Connected to: " << device->portName();
        device->setDataTerminalReady(true);
        device->setRequestToSend(false);
//        device->write("i");
//        while( serNumber == -1 ) {};
        emit sendDebugInfo("Connected to: " + device->portName());
        return true;
    }
    else {
        qDebug() << "Can't open port" << port;
        emit sendDebugInfo("Can't open port" + port, 2000);
        return false;
    }
}

void uartDevice::readData()
{
    while (device->canReadLine()) processLine(device->readLine());
}

void uartDevice::sendData(QString &cmd_)
{
    if(device->isOpen())
    {
            device->write(cmd_.toStdString().c_str());
           // emit sendDebugInfo(QString("Send: ") + cmd_);
            qDebug() << "uartDevice::sendData: " + cmd_;
    }
    else
    {
        emit sendDebugInfo("Device disconnected");
        qDebug() << "Device disconnected";
    }
}

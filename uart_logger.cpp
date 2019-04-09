#include "uart_logger.h"

#include <QtMath>
#include <QDateTime>
#include <QTimer>
#include <thread>
#include <chrono>
#include <ctime>
#include <memory>

//#define REAL_DEVICE
#define SHOW_Debug_

uartReader::uartReader(QObject *parent)
  : uartDevice(parent), m_serNumber(-1), m_nSecs(3600)
  , readyToAskNextDevice(true)
{
    qRegisterMetaType<QtCharts::QAbstractSeries*>();
    qRegisterMetaType<QtCharts::QAbstractAxis*>();
    documentsPath = QDir::homePath()
            + QString("/Documents/")
            + QDateTime::currentDateTime().toString("yyyyMMdd_hhmm/");
}

uartReader::~uartReader()
{
#ifdef SHOW_Debug_
    qDebug() << "uart logger destructor";
#endif
}

void uartReader::initDevice(QString port, QString baudRate)
{
#ifndef REAL_DEVICE
    qDebug() << "Selected port: " << port;
    qDebug() << "Selected baudRate: " << baudRate;
#else
    if (!uartDevice::initDevice(port, baudRate))
    {
        qDebug() << "initDevice return false";
    }
    else
#endif
}


void uartReader::enableLogging(QString delay_, QVariantList serialNums_)
{
    qDebug() << "snVariant: "<<serialNums_;
    if(!QDir(documentsPath).exists())
        QDir().mkdir(documentsPath);

    if(!loggers.empty())
        loggers.clear();

    //create loggers
    loggers.reserve(serialNums_.size());
    for (auto& sn: serialNums_)
    {
        qDebug() << "sn: " + sn.toString();
        loggers.push_back({sn.toString()});
    }

    //open logfiles
    for (auto && device : loggers)
    {
        auto fileName = QString(documentsPath
                      + device.getSerialNumber() );
        if (!device.initLogger(fileName))
        {
            qDebug() << "Cannot create logger for device #"
                        + QString(device.getSerialNumber());
            emit sendDebugInfo("Cannot create logger for device #"
                             + QString(device.getSerialNumber()), 2000);
        }
    }
//    readyToAskNextDevice = true;
    //init timer
    m_logWriteDelay = delay_.toInt()*1000*60;
    if(!timer)
    {
        qDebug() << "Create timer";
        timer = std::make_shared<QTimer>();
        connect(timer.get(), &QTimer::timeout, this, &uartReader::requestDataFromDevices);
    }
    //run timer
    if(!timer->isActive())
    {
        timer->start(m_logWriteDelay);
        qDebug() << "timer: start";
    }
}

void uartReader::disableLogging()
{
    if(timer)
    {
        timer->stop();
        qDebug() << "timer: stop";
    }
}

void uartReader::prepareCommandToSend(QString cmd_)
{
    qDebug() << "prepareCommandToSend: " + cmd_;
    if (cmd_.length()>0)
        m_queueCommandsToSend.push_back(cmd_);
    sendDataToDevice(); //immediately send command
}

void uartReader::selectPath(QString pathForSave)
{
    qDebug() << documentsPath;
    documentsPath = pathForSave;
    qDebug() << pathForSave;
    qDebug() << documentsPath;
}

void uartReader::sendDataToDevice()
{
        while (!m_queueCommandsToSend.empty())
        {
            QString cmd_ = m_queueCommandsToSend[0];
#ifdef REAL_DEVICE
            sendData(cmd_);
#endif
            m_queueCommandsToSend.pop_front();
            emit sendDebugInfo(QString("Send: ") + cmd_);
            qDebug() << "sendDataToDevice: " + cmd_;
        }
}

void uartReader::processLine(const QByteArray &_line)
{
#ifdef SHOW_Debug_
    qDebug() << _line;
#endif //SHOW_Debug_

    QStringList line;
//    logFile->write(_line);

    //appendDataToseries/writeTofile

    //t C Um Ur Ud a b c n ce c0; 11val;
    //1 2 3  4  5  6 7 8 9 10 11
    for (auto w : _line.split(';'))
    {
        line.append(QString(w));
    }
    qDebug() << "_line size is " + QString(line.size());
    if(line[0] == "LOG")
        return;
    if(line[0] == "OK")
        readyToAskNextDevice = true;
    //create timer thread to hold flag while device unsleep


    if(!m_queueCommandsToSend.empty())
        sendDataToDevice();


//    dataProcessingHandler(tempPoint); //send data to ui
//    delayThread.join();

}

void uartReader::dataProcessingHandler(QVector<QPointF> tempPoint)
{
    //sendDataToUi
      //sendDataToFileOnce

}

void uartReader::processTemppoint(int num, double value)
{
    qDebug() << "processTemppoint, num: " << num << ", value " << value;
}

void uartReader::requestDataFromDevices()
{
    if(!readyToAskNextDevice)
        return;
    std::chrono::milliseconds timeoutDelay(200);
    QString cmd;
    for (auto&& dev: loggers) {
        auto _sn = dev.getSerialNumber();

        cmd = QString("AT+%1+LOG$\r").arg(_sn.toInt(),2,10, QChar('0'));
        qDebug() << "cmd is " <<cmd;
        readyToAskNextDevice = false;
        sendData(cmd);

        auto t1 = std::chrono::high_resolution_clock::now();
        auto delayThread = std::thread([&]() {
            auto t2 = std::chrono::high_resolution_clock::now();
            while( (std::chrono::duration<double, std::milli>(t2 - t1) < timeoutDelay) )
            {
                t2 = std::chrono::high_resolution_clock::now();
            }
            qDebug() << "timeout for#"+ _sn;
            emit sendDebugInfo("timeout for#"+ _sn,700);
            readyToAskNextDevice=true;
        });
        while (!readyToAskNextDevice) {;}
        delayThread.join();
    }
    qDebug() << "Request data from devices";
    //TODO
    //go throw all serial numbers with some delay and send cmd to uar t
}


void uartReader::buttonPressHandler(const QStringList &line)
{
    qDebug() << "signal from button";
}



















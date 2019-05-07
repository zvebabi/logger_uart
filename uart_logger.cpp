#include "uart_logger.h"

#include <QtMath>
#include <QDateTime>
#include <QTimer>
#include <thread>
#include <chrono>
#include <ctime>
#include <memory>

#define REAL_DEVICE
//#define SHOW_Debug_

uartReader::uartReader(QObject *parent)
  : uartDevice(parent), m_serNumber(-1), m_nSecs(3600)
  , m_currentConc("0"), currentDevice(0)
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
                      + device.getSerialNumber()
                      + ".csv");
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
    m_logWriteDelay = delay_.toInt()*1000;
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
    auto t1 = std::chrono::high_resolution_clock::now();

#ifdef SHOW_Debug_
    qDebug() << _line;
#endif //SHOW_Debug_

    //QStringList line;
//    logFile->write(_line);
    QVector<QString> splitLine;

    for (auto w : _line.split(';'))
    {
        //line.append(QString(w));
        splitLine.push_back(QString(w));
//        qDebug() << "element is:" << *line.rbegin();
    }
   // qDebug() << "splitLine size is " << splitLine.size();


    if(splitLine.empty())
    {
        qDebug() << "Empty splitLine!";
        return;
    }

    if(splitLine[0] == "LOG\r\n")
    {
        auto testT2 = std::chrono::duration<double, std::milli>(t1-testTime).count();
        qDebug() << "start processLine at" << testT2;
        qDebug() << "processLine started for dev" << currentDevice;
        return;
    }
    if(splitLine[0] == "OK\r\n")
    {
         qDebug() << "reciev OK";
         return;
    }
    if(splitLine[0] == "\r\n")
    {
        t1 = std::chrono::high_resolution_clock::now();
        auto testT2 = std::chrono::duration<double, std::milli>(t1-testTime).count();
        qDebug() << "end processLine at" << testT2;
        qDebug() << "processLine ended for dev" << currentDevice;
        currentDevice++;
        if (currentDevice == loggers.size())
        {
            qDebug()<< "end epoch, restart timer";
            timer->start(m_logWriteDelay);
            currentDevice = 0;
        }
        else
        {
            qDebug()<< "wait for next device";
            timer->start(100);
        }
        return;
    }
      //processDAtaLine

    dataProcessingHandler(splitLine);

}

//void uartReader::dataProcessingHandler(QStringList &line_)
void uartReader::dataProcessingHandler(QVector<QString> &line_)
{
    QString lineForLog;
    lineForLog.append(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss\t"));
    for (auto it = line_.begin()+1; it != line_.end()-1; it++)
    {
        lineForLog.append(*it);
        lineForLog.append("\t");
        //qDebug() << "element is:" << *it;
    }
    lineForLog.append(m_currentConc);
    lineForLog.append("\n");

   // qDebug() << "lineForLog: " << lineForLog;
    loggers[currentDevice].writeLine(lineForLog);

}

void uartReader::processTemppoint(int num, double value)
{
    qDebug() << "processTemppoint, num: " << num << ", value " << value;
}

void uartReader::requestDataFromDevices()
{
    //if(!readyToAskNextDevice)
    //    return;

    timer->stop();
    qDebug() << "Request data from devices";

    auto _sn = loggers[currentDevice].getSerialNumber();
    QString  cmd = QString("AT+%1+LOG$\r").arg(_sn.toInt(),2,16, QChar('0'));

    sendData(cmd);
    //currentDevice++;
    testTime = std::chrono::high_resolution_clock::now();
}


void uartReader::buttonPressHandler(const QStringList &line)
{
    qDebug() << "signal from button";
}



















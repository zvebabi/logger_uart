#include "uart_logger.h"

#include <QtMath>
#include <QDateTime>
#include <QTimer>
#include <thread>
#include <chrono>
#include <ctime>
#include <memory>

//#define REAL_DEVICE
//#define SHOW_Debug_

uartReader::uartReader(QObject *parent)
  : uartDevice(parent), m_serNumber(-1), m_nSamples(0), m_nSecs(150)
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
    emit makeSeries();
    qDebug() << "make series";
#else
    if (!uartDevice::initDevice(port, baudRate))
    {
        qDebug() << "initDevice return false";
    }
    else {
        emit makeSeries();
        qDebug() << "make series";
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
#define ONE_DEV_TWO_PARAMS
void uartReader::dataProcessingHandler(QVector<QString> &line_)
{
    QString lineForLog;
    QString dataToShow("");
    lineForLog.append(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss\t"));
    for (auto it = line_.begin()+1; it != line_.end()-1; it++)
    {
        lineForLog.append(*it);
        lineForLog.append("\t");
        if (it == line_.begin()+3 )
            dataToShow.append(*it);
#ifdef ONE_DEV_TWO_PARAMS
        if (it == line_.begin()+4 )
        {
            dataToShow.append(":");
            dataToShow.append(*it);
        }
#endif //ONE_DEV_TWO_PARAMS
        //qDebug() << "element is:" << *it;
    }
    lineForLog.append(m_currentConc);
    lineForLog.append("\n");

   // qDebug() << "lineForLog: " << lineForLog;
    loggers[currentDevice].writeLine(lineForLog);

#ifdef ONE_DEV_TWO_PARAMS //selector same parameter for 2 device or one device and two params
    if (currentDevice == 0) //for first deviceand two params
#else
    if (currentDevice < 2) //for two device and one param
#endif
    {
        double data =0.0;
        try {
            data = dataToShow.split(":")[0].toDouble();
        } catch (...) {
            qDebug() << "error convert data " << dataToShow << " to double";
        }
        update(currentDevice, QPointF(m_nSamples, data));
#ifdef ONE_DEV_TWO_PARAMS //selector same parameter for 2 device or one device and two params
        try {
            data = dataToShow.split(":").at(1).toDouble();
        } catch (...) {
            qDebug() << "error convert data " << dataToShow << " to double";
        }
        update(1, QPointF(m_nSamples, data)); //second params of first device
#endif
    }
    if(currentDevice == 0)
        m_nSamples++;
}

void uartReader::sendSeriesPointer(QtCharts::QAbstractSeries* series_
                                   , QtCharts::QAbstractAxis* Xaxis_)
{
    qDebug() << "sendSeriesPointer: " << series_;
    m_series.push_back(series_);
    m_axisX.push_back(Xaxis_);
    qDebug() << "getSeriesPointer: " << *m_series.rbegin();
}

void uartReader::update(int graphIdx, QPointF p)
{
    if (m_series[graphIdx]) {
        QtCharts::QXYSeries *xySeries =
                static_cast<QtCharts::QXYSeries *>(m_series[graphIdx]);
        // Use replace instead of clear + append, it's optimized for performance
        xySeries->append(p);//replace(lines.value(series));
    }
    else {
        return;
    }
    if(m_axisX[graphIdx]) {
        m_axisX[graphIdx]->setMin(p.rx()-m_nSecs);
        m_axisX[graphIdx]->setMax(p.rx());
    }
    if(m_series[graphIdx]->chart())
    {
        qDebug() << "chart exist";
        auto chart = m_series[graphIdx]->attachedAxes();
        if(m_data.size() < graphIdx + 1)
            m_data.resize(graphIdx+1);
        m_data[graphIdx].push_back(p);
        QPointF min = *std::min_element(m_data[graphIdx].begin(), m_data[graphIdx].end(),[](QPointF a, QPointF b){ return a.ry() < b.ry();});
        QPointF max = *std::max_element(m_data[graphIdx].begin(), m_data[graphIdx].end(),[](QPointF a, QPointF b){ return a.ry() < b.ry();});
        chart[1]->setMax(max.ry());
        chart[1]->setMin(min.ry());
    }
#ifdef SHOW_Debug_
    qDebug() << __func__;
#endif
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



















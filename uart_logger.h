#ifndef ANALIZERCDC_H
#define ANALIZERCDC_H

#include <QtCore/QObject>
#include <QVariant>
#include <QVector>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>
#include <QByteArray>
#include <QPointF>
#include <QAbstractSeries>
#include <QtCharts/QAbstractSeries>
#include <QtCharts/QXYSeries>
#include <QColor>
#include <QVariantList>
#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QPair>
#include <QFile>
#include <vector>
#include <fstream>
#include <typeinfo>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <memory>
#include <thread>

#include "uartdevice.h"
#include "statistic.h"
#include "analizer.h"


class uartReader : public uartDevice
{
    Q_OBJECT
public:
    explicit uartReader(QObject *parent = 0);
    ~uartReader();
public slots:
    void initDevice(QString port, QString baudRate);

    QString getDataPath() {return documentsPath;}
    void enableLogging(QString delay_, QVariantList serialNums_);
    void disableLogging();
//    void readData();
    void prepareCommandToSend(QString cmd_);

    void setCurrentConc(QString _conc){
        m_currentConc = _conc;//.toDouble();
    }

    void requestDataFromDevices();
    void selectPath(QString pathForSave);

private:
    void processLine(const QByteArray& line);
    void sendDataToDevice();
    void dataProcessingHandler(QVector<QPointF> tempPoint);
    void processTemppoint(int num, double value);
    void buttonPressHandler(const QStringList& line);

    int m_serNumber;
    int m_nSamples;
    int m_nSecs;
    QString m_currentConc;
    int m_logWriteDelay;
    std::shared_ptr<QTimer> timer;

    std::shared_ptr<QFile> logFile;
    QVector<QString> m_queueCommandsToSend;

    QString documentsPath;
    std::ofstream diagnosticLog;
    std::vector<Analizer> loggers;
    std::atomic_bool readyToAskNextDevice;

};

#endif // ANALIZERCDC_H

#ifndef UARTDEVICE_H
#define UARTDEVICE_H

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

class uartDevice : public QObject
{
    Q_OBJECT
public:
    explicit uartDevice(QObject *parent = 0);
    ~uartDevice();

    void sendData(QString &cmd_);

public slots:
    bool initDevice(const QString &port, const QString &baudRate);
    void readData();
    void getListOfPort();
signals:
    void sendPortName(QString port);
    void sendDebugInfo(QString data, int time=700);
private:
    virtual void processLine(const QByteArray& line) = 0;
    std::vector<QString> ports;
    QSerialPort* device = NULL;

};

#endif // UARTDEVICE_H

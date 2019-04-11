#ifndef ANALIZER_H
#define ANALIZER_H
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QDebug>
#include <memory>

class Analizer
{
public:
    Analizer(QString sn_) : m_serial(sn_)
    {
        qDebug() << "Analizer constructorSN #" + QString(m_serial);
    }
    ~Analizer();
    bool initLogger(QString fname_);
    void writeLine(QString line);
    QString getSerialNumber(){
        return m_serial;
    }
private:
    const QString m_serial;
    std::shared_ptr<QFile> logFile;
};

#endif // ANALIZER_H

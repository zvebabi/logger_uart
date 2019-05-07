#include "analizer.h"
#include <QDebug>

Analizer::~Analizer()
{
#ifdef SHOW_Debug_
    qDebug() << "analizer destructor " + QString(m_serial);
#endif
    if(logFile != nullptr)
        logFile->close();
}

bool Analizer::initLogger(QString fname_)
{
    logFile = std::make_shared<QFile>(fname_);
    if (!logFile->open(QIODevice::Append | QIODevice::Text))
    {
        return false;
    }
    QTextStream ts ( logFile.get() );
    //create header
    ts << QString("Time\tN\tC1\tC2\tUc1\tUc2\tUd1\tUd2\tConc\n");
    return true;
}

void Analizer::writeLine(QString line)
{
   // qDebug() << "writeLine " + line;
    //parce and write
    QTextStream ts ( logFile.get() );
    ts << line;
}

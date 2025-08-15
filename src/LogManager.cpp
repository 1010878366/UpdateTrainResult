#include "LogManager.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QMutexLocker>

LogManager::LogManager(QString strLogRootDir)
{
    m_textEdit = nullptr;
    m_strLogRootDir = strLogRootDir;
}

void LogManager::SetTextEdit(QTextEdit* textEdit)
{
    m_textEdit = textEdit;
}

void LogManager::AddOneMsg(QString strInfo, bool bUI)
{
    QMutexLocker locker(&m_mutex);  //保证多线程安全
    QString strTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString strMsg = strTime+"  "+strInfo;

    //UI显示
    if(bUI && m_textEdit)
        m_textEdit->append(strMsg);

    //日志写入文件
    QString strMonth = QDateTime::currentDateTime().toString("yyyy-MM");
    QString strDay = QDateTime::currentDateTime().toString("dd");

    QString strPath = QString("%1/%2/").arg(m_strLogRootDir).arg(strMonth);

    QDir dir;
    if(!dir.exists(strPath))
        dir.mkdir(strPath);

    QString strFilePath = strPath + strDay + "log.txt";
    QFile file(strFilePath);
    if(file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&file);
        out << strMsg << "\r\n";
        file.close();
    }
}

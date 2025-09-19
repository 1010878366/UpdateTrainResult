#include "ThreadManager.h"
#include "ConfigManager.h"
#include "DatabaseManager.h"
#include <QFile>
#include <QSettings>
#include <QTextStream>
#include <QDebug>

ThreadManager::ThreadManager(QObject *parent)
    : QThread(parent)
{
}

void ThreadManager::setWriteDBParams(const QString &reelTable, ConfigManager *cfgMgr, DatabaseManager *dbMgr)
{
    m_task = WriteDB;
    m_strReelTable = reelTable;
    m_configManager = cfgMgr;
    m_dbManager = dbMgr;
}

void ThreadManager::setInferParams(const QString &configPath, ConfigManager *cfgMgr)
{
    m_task = InferProcess;
    m_strConfigPath = configPath;
    m_configManager = cfgMgr;
}

void ThreadManager::run()
{
    if (m_task == WriteDB)
    {
        if (!m_dbManager || !m_configManager || m_strReelTable.isEmpty())
        {
            emit logMessage("ThreadManager: 写数据库参数未设置或无效");
            emit writeFinished(false, m_strReelTable);
            return;
        }

        QString strCsvPath = QString("F:/Inference/%1/result.csv").arg(m_strReelTable);
        QFile file(strCsvPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            emit logMessage(QString("无法打开CSV文件: %1").arg(strCsvPath));
            emit writeFinished(false, m_strReelTable);
            return;
        }

        if (!m_dbManager->ConnectToDB())
        {
            emit logMessage("数据库连接失败");
            emit writeFinished(false, m_strReelTable);
            return;
        }

        int nCount = 0;
        while (!file.atEnd())
        {
            QByteArray line = file.readLine();
            QString strLine(line);
            strLine = strLine.trimmed();
            if (strLine.isEmpty()) continue;

            QStringList parts = strLine.split(',');

            int nFileIndex = parts.value(0).toInt();
            int nDefectIndex = parts.value(1).toInt();
            int nDefectLevel = parts.value(2).toInt();
            float fCentreX = parts.value(3).toFloat();
            float fCentreY = parts.value(4).toFloat();
            float fLength = parts.value(5).toFloat();
            float fHeight = parts.value(6).toFloat();

            QString strRectCoordinate;
            if (fCentreX != -9.9999f && fCentreY != -9.9999f &&
                fLength != -9.9999f && fHeight != -9.9999f)
            {
                strRectCoordinate = QString("%1,%2,%3,%4").arg(fCentreX).arg(fCentreY).arg(fLength).arg(fHeight);
            }

            bool bUpdate = m_dbManager->UpdateDefectInfo(
                m_strReelTable,
                m_configManager->GetDefectName(nDefectIndex),
                nFileIndex,
                nDefectLevel,
                strRectCoordinate);

            if (!bUpdate)
                emit logMessage(QString("数据库更新失败，第%1行").arg(nCount));

            nCount++;
        }
        file.close();

        // 更新 config.ini 标志
        QString strReelConfigPath = QString("F:/Inference/%1/config.ini").arg(m_strReelTable);
        QSettings settings(strReelConfigPath, QSettings::IniFormat);
        settings.setValue("param/is_execute", 1);

        emit logMessage(QString("数据表【%1】写入完成，共 %2 条记录").arg(m_strReelTable).arg(nCount));
        emit writeFinished(true, m_strReelTable);

        //打印线程ID
        emit logMessage(QString("ThreadManager::run() 线程ID: %1").arg((quintptr)QThread::currentThreadId()));
    }
}

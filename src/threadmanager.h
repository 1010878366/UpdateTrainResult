#pragma once
#include <QThread>
#include <QString>
#include "configmanager.h"
#include "DatabaseManager.h"

class ThreadManager : public QThread
{
    Q_OBJECT
public:
    explicit ThreadManager(QObject *parent = nullptr);

    // 设置任务参数
    void setWriteDBParams(const QString& reelTable, ConfigManager* cfgMgr, DatabaseManager* dbMgr);
    void setInferParams(const QString& configPath, ConfigManager* cfgMgr);

    // 任务类型枚举
    enum TaskType { None, WriteDB, InferProcess } m_task = None;    //None：没有任务；WriteDB：写数据库任务；InferProcess：Python 推理任务

signals:
    void logMessage(const QString& msg);                  // 发送日志
    void writeFinished(bool success, const QString& tableName); // 写数据库完成
    void inferFinished(bool success);                     // Python推理完成

protected:
    void run() override;  // 线程入口

private:
    // 写数据库参数
    QString m_strReelTable;
    ConfigManager* m_configManager = nullptr;
    DatabaseManager* m_dbManager = nullptr;

    // 推理任务参数
    QString m_strConfigPath;
};

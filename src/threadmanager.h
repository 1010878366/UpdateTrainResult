#pragma once
#include <QThread>
#include <QString>
#include <QMutex>
#include "configmanager.h"
#include "DatabaseManager.h"

class ThreadManager : public QThread
{
    Q_OBJECT
public:
    explicit ThreadManager(QObject *parent = nullptr);

    // 任务类型枚举
    enum TaskType {
        None,           //没有任务
        WriteDB,        //写数据库任务
        InferProcess
    } m_task = None;

    void setDBTaskParams(const QString& reelTable, ConfigManager* cfgMgr, DatabaseManager* dbMgr, bool isAuto = false); //写数据库任务函数
    void setInferParams(const QString& configPath, ConfigManager* cfgMgr);  //推理任务函数
    void WriteToDB();       //写入数据库函数


signals:
    void logMessage(const QString& msg);                  //发送日志
    void writeFinished(bool success, const QString& tableName); //写数据库完成
    void inferFinished(bool success, const QString& tableName); //Python推理完成

protected:
    void run() override;  // 线程入口

private:
    QMutex m_mutex;             //线程锁
    QString m_strReelTable;     //卷号名
    QString m_strConfigPath;    //路径
    bool m_isAuto = false;      //更新数据库  true:自动，false：手动
    ConfigManager* m_configManager = nullptr;
    DatabaseManager* m_dbManager = nullptr;


};

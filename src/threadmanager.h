#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include <QObject>
#include <QString>
#include"DatabaseManager.h"
#include"configmanager.h"

class ThreadManager : public QObject
{
    Q_OBJECT
public:
    // 构造函数：接收必要参数（注意新增了ConfigManager指针）
    explicit ThreadManager(
        QString strReelTable,       // 数据表名（m_currentReelTable）
        QString dbHost,             // 数据库IP
        QString dbName,             // 数据库名
        QString dbPwd,              // 数据库密码
        ConfigManager* configMgr,   // 配置管理器（用于获取缺陷名，不转换QMap）
        QObject *parent = nullptr
    );

    ~ThreadManager();

signals:
    // 发送日志给主线程
    void sendLog(QString logMsg);
    // 通知主线程任务完成
    void workFinished(bool isSuccess, QString resultMsg);

public slots:
    // 子线程执行的核心任务
    void doWork();

private:
    QString m_strReelTable;       // 数据表名（保留原变量名）
    QString m_dbHost;             // 数据库IP
    QString m_dbName;             // 数据库名
    QString m_dbPwd;              // 数据库密码
    ConfigManager* m_configMgr;   // 配置管理器（新增，用于调用GetDefectName）
    DatabaseManager* m_dbMgr;     // 子线程自己的数据库管理器
};

#endif // THREADMANAGER_H

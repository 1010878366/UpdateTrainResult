#include"threadmanager.h"
#include <QFile>
#include <QSettings>

// 构造函数：初始化参数（新增configMgr）
ThreadManager::ThreadManager(
    QString strReelTable,
    QString dbHost,
    QString dbName,
    QString dbPwd,
    ConfigManager* configMgr,
    QObject *parent
) : QObject(parent)
    , m_strReelTable(strReelTable)
    , m_dbHost(dbHost)
    , m_dbName(dbName)
    , m_dbPwd(dbPwd)
    , m_configMgr(configMgr)  // 初始化配置管理器指针
    , m_dbMgr(nullptr)
{
}

// 析构函数：清理资源
ThreadManager::~ThreadManager()
{
    if (m_dbMgr) {
        m_dbMgr->DisConnectDB();
        delete m_dbMgr;
        m_dbMgr = nullptr;
    }
}

// 子线程核心工作（几乎复用你原来的WriteToDB逻辑）
void ThreadManager::doWork()
{
    // 1. 打开CSV文件（和你原来的代码一致）
    QString strCsvPath = QString("F:/Inference/%1/result.csv").arg(m_strReelTable);
    QFile file(strCsvPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit sendLog("CSV文件打开失败：" + strCsvPath);
        emit workFinished(false, "CSV文件打开失败");
        return;
    }
    emit sendLog("成功打开CSV文件：" + strCsvPath);

    // 2. 初始化子线程自己的数据库连接（不共用主线程的m_dbManager）
    m_dbMgr = new DatabaseManager(m_dbHost, m_dbName, m_dbPwd);
    if (!m_dbMgr->ConnectToDB()) {
        file.close();
        emit sendLog("数据库连接失败");
        emit workFinished(false, "数据库连接失败");
        return;
    }
    emit sendLog("数据库连接成功");

    // 3. 循环读取CSV并写入数据库（核心逻辑，变量名完全保留）
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString strLine(line.trimmed());
        if (strLine.isEmpty()) continue;

        QStringList parts = strLine.split(',');

        // 解析CSV数据（变量名和你原来的完全一样）
        int nFileIndex = 0;
        int nDefectIndex = 0;
        int nDefectLevel = 0;
        float fCentreX = -9.9999f;
        float fCentreY = -9.9999f;
        float fLength = -9.9999f;
        float fHeight = -9.9999f;

        if (parts.size() > 0 && !parts[0].isEmpty()) nFileIndex = parts[0].toInt();
        if (parts.size() > 1 && !parts[1].isEmpty()) nDefectIndex = parts[1].toInt();
        if (parts.size() > 2 && !parts[2].isEmpty()) nDefectLevel = parts[2].toInt();
        if (parts.size() > 3 && !parts[3].isEmpty()) fCentreX = parts[3].toFloat();
        if (parts.size() > 4 && !parts[4].isEmpty()) fCentreY = parts[4].toFloat();
        if (parts.size() > 5 && !parts[5].isEmpty()) fLength = parts[5].toFloat();
        if (parts.size() > 6 && !parts[6].isEmpty()) fHeight = parts[6].toFloat();

        // 处理坐标（和你原来的代码一致）
        QString strRectCoordinate = QString("%1,%2,%3,%4")
                                       .arg(fCentreX).arg(fCentreY).arg(fLength).arg(fHeight);
        if (strRectCoordinate.contains("-9.9999")) strRectCoordinate.clear();

        // 获取缺陷名：直接调用ConfigManager的GetDefectName（不转QStringList）
        // 因为m_strDefectName是只读的，多线程读取安全
        QString strDefectName = m_configMgr->GetDefectName(nDefectIndex);

        // 写入数据库（用子线程自己的m_dbMgr）
        bool bUpdate = m_dbMgr->UpdateDefectInfo(
            m_strReelTable, strDefectName, nFileIndex, nDefectLevel, strRectCoordinate
        );
        if (!bUpdate) {
            emit sendLog(QString("更新失败：文件索引%1，缺陷%2")
                         .arg(nFileIndex).arg(strDefectName));
        }
    }

    // 4. 清理资源
    file.close();
    m_dbMgr->DisConnectDB();
    emit sendLog("CSV文件处理完毕，数据库已断开连接");

    // 5. 更新INI文件（和你原来的代码一致）
    QString strReelConfigPath = QString("F:/Inference/%1/config.ini").arg(m_strReelTable);
    QSettings settings(strReelConfigPath, QSettings::IniFormat);
    settings.setValue("param/is_execute", "1");
    emit sendLog("已更新INI文件：" + strReelConfigPath);

    // 6. 通知主线程完成
    emit workFinished(true, "数据表【" + m_strReelTable + "】更新成功");
}

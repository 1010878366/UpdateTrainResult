#include "DatabaseManager.h"

DatabaseManager::DatabaseManager()
{
    m_pAdo = nullptr;
    m_strHost = "10.169.70.170";
    m_strDB = "DB_CENTRAL_UI";
    m_strPwd = "kexin2008";
}

bool DatabaseManager::ConnectToDB()
{
    if(m_pAdo)
    {
        delete m_pAdo;
        m_pAdo = nullptr;
    }
    m_pAdo = new ADOLinkToBase;
    return m_pAdo->Connection(m_strHost, m_strDB, m_strPwd, true);
}

DatabaseManager::~DatabaseManager()
{
    DisConnectDB();
}

bool DatabaseManager::DisConnectDB()
{
    if(m_pAdo)
    {
        delete m_pAdo;
        m_pAdo = nullptr;
    }
    return true;
}

bool DatabaseManager::UpdateDefectInfo(QString strTableName, QString strDefectName, int nIndex, int nLevel, QString strRectCoordinate)
{
    QString strSQL = QString("UPDATE [%1] SET "
                             "Feature_Name14 = '%2', "
                             "Feature_nValue14 = %3, "
                             "Feature_Name13 = '%4' "
                             "WHERE Index_InQue = %5")
                             .arg(strTableName)
                             .arg(strDefectName)
                             .arg(static_cast<float>(nLevel))
                             .arg(strRectCoordinate)
                             .arg(nIndex);
    if(!m_pAdo->Execute(strSQL))
        return false;
    return true;
}

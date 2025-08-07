#include "ADOLinkToBase.h"
#include<QMessageBox>

ADOLinkToBase::ADOLinkToBase()
{
    m_pQuery = nullptr;
    m_nConnectStatus = -1;
    m_nRecordsetStatus = -1;

}

ADOLinkToBase::~ADOLinkToBase()
{
    DisConnect();
    if(m_pQuery)
    {
        delete m_pQuery;
        m_pQuery = nullptr;
    }
}

bool ADOLinkToBase::Connection(const QString& strServer,const QString& strDataBase, const QString& strPwd, bool bConMode)
{
    if(m_dbConnection.isOpen())
        m_dbConnection.close();

    m_dbConnection = QSqlDatabase::addDatabase("QODBC");

    QString dsn;
    if(bConMode)    //远程IP登录
    {
        //SQLserver登录
        dsn = QString("Driver={SQL Server};Server=%1;Database=%2;Uid=sa;Pwd=%3").arg(strServer).arg(strDataBase).arg(strPwd);
    }
    else    //本地Windows账户登录
    {
        dsn = QString("Driver={SQL Server};Server=%1;Database=%2;Trusted_Connection=yes;").arg(strServer).arg(strDataBase);
    }

    m_dbConnection.setDatabaseName(dsn);
    if(m_dbConnection.open())
    {
        m_nConnectStatus = 0;
        return true;
    }
    else {
        QMessageBox::warning(nullptr,"警告",QString("数据库连接失败：%1").arg(m_dbConnection.lastError().text()));
        m_nConnectStatus = -1;
        return false;
    }
}

void ADOLinkToBase::DisConnect()
{
    if(m_pQuery)
    {
        delete m_pQuery;
        m_pQuery = nullptr;
    }

    if(m_dbConnection.isOpen())
        m_dbConnection.close();
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
}

bool ADOLinkToBase::Execute(const QString& strSQL)
{
    if(!m_dbConnection.isOpen())
        return false;

    if(m_pQuery)
        delete m_pQuery;

    m_pQuery = new QSqlQuery(m_dbConnection);
    if(!m_pQuery->exec(strSQL))
    {
        QMessageBox::warning(nullptr,"警告",QString("数据库查询失败：%1").arg(m_pQuery->lastError().text()));
        m_nRecordsetStatus = -1;
        return false;
    }

    m_nRecordsetStatus = 0;
    return true;
}

bool ADOLinkToBase::GetCollect(const QString& ColumnName,QString& strValue)
{
    if(!m_pQuery || !m_pQuery->isActive())
        return false;

    QVariant var = m_pQuery->value(ColumnName);
    if(var.isValid())
    {
        strValue = var2str(var);
        return true;
    }
    return false;
}

bool ADOLinkToBase::IsEmpty()
{
    if (!m_pQuery || !m_pQuery->isActive())
        return true;
    return m_pQuery->at() == QSql::AfterLastRow;
}

void ADOLinkToBase::NextRecd()
{
    if(m_pQuery)
        m_pQuery->next();
}

QString ADOLinkToBase::var2str(const QVariant &var)
{
    if(var.isNull() || !var.isValid())
        return "";

    switch (var.type()) {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Double:
        return QString::number(var.toDouble(), 'f', 4);
    case QVariant::Bool:
        return var.toBool() ? "TRUE" : "FALSE";
    case QVariant::Date:
    case QVariant::DateTime:
        return var.toDateTime().toString("yyyy-MM-dd HH:mm:ss");
    case QVariant::String:
    default:
        return var.toString();
    }
}

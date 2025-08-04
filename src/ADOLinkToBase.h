#include<QSqlDatabase>
#include<QSqlQuery>
#include<QSqlError>
#include<QString>
#include<QVariant>
#include<QDateTime>

class ADOLinkToBase
{
public:
    ADOLinkToBase();
    ~ADOLinkToBase();

public:
    QSqlDatabase m_dbConnection;    //数据库连接对象
    QSqlQuery* m_pQuery;            //查询对象指针
    int m_nConnectStatus;    // 数据库连接状态：0表示成功，非0表示错误代码
    int m_nRecordsetStatus;  // 查询执行状态：0表示成功，非0表示错误代码

public:
    bool Connection(const QString& strServer,const QString& strDataBase,const QString& strUser, const QString& strPwd, bool bConMode);  //连接数据库
    void DisConnect();      //断开数据库
    bool Execute(const QString& strSQL);    //执行sql语句
    bool GetCollect(const QString& ColumnName,QString& strValue);   //获取字段值
    bool IsEmpty();     //判断记录集是否为空或者达到末尾
    void NextRecd();        //移动到下一条记录
    QString var2str(const QVariant& var);   // 数据类型转换：QVariant转QString


};


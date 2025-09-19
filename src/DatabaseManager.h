#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#pragma once
#include <QString>
#include "ADOLinkToBase.h"

//数据库操作类
class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();
    bool ConnectToDB();     //连接数据库
    bool DisConnectDB();    //断开数据库
    bool UpdateDefectInfo(QString strTableName, QString strDefectName, int nIndex, int nLevel, QString strRectCoordinate); // 更新缺陷信息

private:
    ADOLinkToBase* m_pAdo;  //数据库对象
    QString m_strHost;  //数据库对象
    QString m_strDB;    //数据库名称
    QString m_strPwd;   //数据库密码

};

#endif // DATABASEMANAGER_H

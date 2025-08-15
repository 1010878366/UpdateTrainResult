#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include<QString>

//管理配置文件和缺陷映射表
class ConfigManager
{
public:
    ConfigManager(QString strPathConfig); //加载路径配置和缺陷映射表

    QString GetFilePath();  //获取全局文件路径配置
    QString GetDefectName(int nIndex);  //根据索引获得缺陷名称
    QString GetReelTable(QString strReelConfigPath);    //获得[卷号]/config.ini中reel_table值
    bool SetIsExecute(QString strReelConfigPath,bool bExecute); //设置是否经过深度学习标记

private:
    QString m_strFilePath;  //
    QString m_strDefectName[64];    //缺陷名称映射表
    void LoadDefectMap();   //读取缺陷映射表


};

#endif // CONFIGMANAGER_H

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include<QString>
#include<QVector>
#include<QMap>

//管理配置文件和缺陷映射表
class ConfigManager
{
public:
    ConfigManager(QString strPathConfig); //加载路径配置和缺陷映射表

    QString& GetFilePath();  //return m_strFilePath  存储上次找到的文件路径(path_config.ini中的file_path的值)
    QString GetNewFilePath(QString strPathConfig);   //获取最新的文件路径(path_config.ini中的file_path的值)
    QString GetDefectName(int nIndex);  //根据索引获得缺陷名称
    QString GetReelTable(QString strReelConfigPath);    //获得[卷号]/config.ini中reel_table值
    bool SetIsExecute(QString strReelConfigPath,bool bExecute); //设置是否经过深度学习标记
    void LoadDefectMap();           //读取缺陷映射表

private:
    QString m_strFilePath;          //存储初始化中的文件路径(path_config.ini中的file_path的值)
    QString m_strNewFilePath;       //存储最新的文件路径(path_config.ini中的file_path的值)
    //QString m_strDefectName[64];    //缺陷名称映射表
    QMap<int,QString> m_strDefectName;   //缺陷名称映射表



};

#endif // CONFIGMANAGER_H

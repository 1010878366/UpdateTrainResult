#include "ConfigManager.h"
#include<QSettings>
#include <QTextCodec>
#pragma execution_character_set("utf-8")

ConfigManager::ConfigManager(QString strPathConfig)
{
    //读取全局文件路径
    //QString strSavedPathConfig = strPathConfig;
    QSettings setFile(strPathConfig,QSettings::IniFormat);
    m_strFilePath = setFile.value("param/file_path","NULL").toString();
    LoadDefectMap();    //加载缺陷映射表
    GetReelTable(m_strFilePath);

}

QString& ConfigManager::GetFilePath()
{
    return m_strFilePath;
}

QString ConfigManager::GetNewFilePath(QString strPathConfig)
{
    QSettings setFile(strPathConfig,QSettings::IniFormat);
    m_strNewFilePath = setFile.value("param/file_path","NULL").toString();
    return m_strNewFilePath;
}

QString ConfigManager::GetDefectName(int nIndex)
{
    if(nIndex >= 0 && nIndex < 64)
        return m_strDefectName[nIndex];
    return "无分类";
}

QString ConfigManager::GetReelTable(QString strReelConfigPath)
{
    //strReelConfigPath += "/config.ini";
    QSettings settings(strReelConfigPath, QSettings::IniFormat);
    QString strReelTable = settings.value("param/reel_table","NULL").toString();
    return strReelTable;
}

bool ConfigManager::SetIsExecute(QString strReelConfigPath,bool bExecute)
{
    QSettings settings(strReelConfigPath, QSettings::IniFormat);
    settings.setValue("param/is_execute", bExecute ? 1 : 0);    //1为完成深度学习，0为未完成
    settings.sync();    //立即执行
    return true;
}

void ConfigManager::LoadDefectMap()
{
    //读取缺陷映射表文件 
    QSettings setDefectMap("D:/DefectMap.ini",QSettings::IniFormat);
    setDefectMap.setIniCodec(QTextCodec::codecForName("GB2312"));
    setDefectMap.beginGroup("Param");
    for(int i=0;i<64;i++)
    {
        QString strKey = QString::number(i);
        m_strDefectName[i]=setDefectMap.value(strKey,"无分类").toString();
    }
    setDefectMap.endGroup();
}

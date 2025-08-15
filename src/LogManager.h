#ifndef LOGMANAGER_H
#define LOGMANAGER_H
#pragma once
#include <QString>
#include <QTextEdit>
#include <QMutex>
#include <QTextCodec>
#pragma execution_character_set("utf-8")

//日志管理类
class LogManager
{
public:
    LogManager(QString strLogRootDir);
    void AddOneMsg(QString strInfo,bool bUI = true);    //写日志
    void SetTextEdit(QTextEdit* textEdit);  //设置UI显示控件

private:
    QTextEdit* m_textEdit;                  //UI文本框
    QString m_strLogRootDir;                //日志根目录
    QMutex m_mutex;                         //日志线程安全锁
};

#endif // LOGMANAGER_H

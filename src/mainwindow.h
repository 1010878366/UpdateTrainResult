#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QFile>
#include "ConfigManager.h"
#include "DatabaseManager.h"
#include "LogManager.h"
#include "traymanager.h"
#include "timermanager.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    ConfigManager* m_configManager;     //配置管理
    DatabaseManager* m_dbManager;       //数据库管理
    LogManager* m_logManager;           //日志管理
    TrayManager* m_trayManager;         //托盘管理
    TimerManager* m_timerManager;       //定时器管理


    QString m_strReelConfigPath;        //存储卷号config文件的路径([卷号]/config.ini的路径)
    QString m_strReelTable;				//存储获取到的卷号([卷号]/config.ini中reel_table的值)
    QString m_strPathConfig;            //存储"F:/Inference/path_config.ini"路径
    QString m_currentReelTable;         //暂存表名


private slots:
    void OpenButton();                  //读取配置按钮
    void WriteButton();                 //写入数据库按钮
    void AutomaticUpdateDatabase(QString strReelTable);  //自动写入数据库
    bool terminateProcessByName(const QString &procName); //关闭进程


public:
    bool WriteToDB(const QString& strReelTable);
    void ToTray();                        //最小化到托盘
    void ExistNewReel();                  //检测新卷号
    void HandleInferProcess();            //处理深度学习程序
    void DeleteTray();

};
#endif // MAINWINDOW_H

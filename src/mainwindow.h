#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QFile>
#include "ConfigManager.h"
#include "CsvParser.h"
#include "DatabaseManager.h"
#include "LogManager.h"
#include "ui_mainwindow.h"

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

    QString m_strReelConfigPath;        //存储卷号config文件的路径([卷号]/config.ini的路径)
    QString m_strReelTable;				//存储获取到的卷号([卷号]/config.ini中reel_table的值)
    QString m_strPathConfig;            //存储"F:/Inference/path_config.ini"路径

    QTimer* m_timer1;
    QTimer* m_timer2;
    QTimer* m_timer3;

    QSystemTrayIcon* m_trayIcon;        //托盘图标

private slots:
    void OpenButton();                    //手动选择配置文件
    void AutomaticUpdateDatabase(QString strReelTable);  //自动写入数据库

    void ToTray();                        //最小化到托盘
    void DeleteTray();                    //删除托盘图标

    void onTimer1();                      //定时器1回调
    void onTimer2();                      //定时器2回调
    void onTimer3();                      //定时器3回调

    void ExistNewReel();                  //检测新卷号
    void HandleInferProcess();            //处理深度学习程序
    bool terminateProcessByName(const QString &procName); //关闭进程

public:
    bool WriteToDB(QString strReelTable);

};
#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"ADOLinkToBase.h"
#include<QFile>
#include<QFileDialog>
#include<QSettings>
#include<QMessageBox>
#include<QTimer>
#include<QListWidget>
#include<QTextStream>
#include<QSystemTrayIcon>
#include<QMenu>
#include<QAction>
#include<QProcess>


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
    QListWidget* m_listInfo;
    QFile m_logFile;
    QSystemTrayIcon *m_trayIcon = nullptr;        //程序最小化到托盘区的指针

    ADOLinkToBase *m_pAdo;
    QString m_strDefectName[64];
    QString m_strFilePath;					//存储上次找到的文件路径(path_config.ini中的file_path的值)
    QString m_strReelConfigPath;			//存储卷号config文件的路径([卷号]/config.ini的路径)
    //QString m_strReelName;				//存储获取到的卷名([卷号]/config.ini中reel_name的值)
    QString m_strReelTable;					//存储获取到的卷号([卷号]/config.ini中reel_table的值)

    QTimer* m_timer1;
    QTimer* m_timer2;
    QTimer* m_timer3;

private slots:
    void OpenButton();
    //void MiniToTray();
    void ToTray();    //最小化到托盘区
    void DeleteTray();

    void onTimer1();
    void onTimer2();
    void onTimer3();

public:
    //QString m_strReelName;  //获取到的文件路径

public:
    bool WriteToDB(QString strReelTable);
    bool ConnectToDatabase(ADOLinkToBase *&pAdo);
    void DisConnectFromDatabase(ADOLinkToBase *&pAdo);
    void AddOneMsg(QString strInfo);
    void AddOneLog(QString strMonth, QString strDay, QString strInfo);
    void AutomaticUpdateDatebase(QString strReelTable);
    void UpdateDefectInfo(ADOLinkToBase* pAdo,QString strTableName,QString strInfo,int nIndex,int nLevel,QString strRectCoordinate);
    QString escapeSingleQuotes(const QString &str); //转义SQL中的单引号，防止SQL注入和语法错误
    void ExistNewReel();
    void HandleInferProcess();
    bool terminateProcessByName(const QString &procName);

    //bool MakeDirectory(const QString &strPathName);

};
#endif // MAINWINDOW_H

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QSettings>
#include <QProcess>
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>

#pragma execution_character_set("utf-8")

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_trayIcon(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("更新训练结果 V1.1.1");

    // 初始化核心对象
    m_configManager = new ConfigManager("F:/Inference/path_config.ini");
    m_dbManager = new DatabaseManager("10.169.70.170", "DB_CENTRAL_UI", "kexin2008");
    m_logManager = new LogManager(QCoreApplication::applicationDirPath() + "/../../UpdateLog/");
    m_logManager->SetTextEdit(ui->textEdit_Info);

    // 定时器
    m_timer1 = new QTimer(this);
    connect(m_timer1, &QTimer::timeout, this, &MainWindow::onTimer1);
    m_timer1->start(2000);

    m_timer2 = new QTimer(this);
    connect(m_timer2, &QTimer::timeout, this, &MainWindow::onTimer2);
    m_timer2->setSingleShot(true);

    m_timer3 = new QTimer(this);
    connect(m_timer3, &QTimer::timeout, this, &MainWindow::onTimer3);
    m_timer3->setSingleShot(true);
    m_timer3->start(3000);

    // 按钮绑定
    connect(ui->btn_Open, &QPushButton::clicked, this, &MainWindow::OpenButton);
    connect(ui->btn_MiniToTray, &QPushButton::clicked, this, &MainWindow::ToTray);
    connect(ui->btn_Close, &QPushButton::clicked, this, &MainWindow::close);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_configManager;
    delete m_dbManager;
    delete m_logManager;
    if(m_trayIcon)
        delete m_trayIcon;
}

// 手动选择配置文件
void MainWindow::OpenButton()
{
    //1.手动选择[卷号]config.ini文件
    QString strReelConfigPath = QFileDialog::getOpenFileName(
        this, tr("选择配置文件"), QCoreApplication::applicationDirPath(), tr("INI文件(*.ini)"));
    if(strReelConfigPath.isEmpty())
        return;

    //2.显示路径到lineEdit
    ui->lineEdit_Path->setText(strReelConfigPath);

    //3.读取reel_table参数
    QString strReelTable = m_configManager->GetReelTable(strReelConfigPath);

    m_logManager->AddOneMsg(tr("正在写入数据库"));
    QString strInfo;
    bool bWrite = WriteToDB(strReelTable);
    if (bWrite)
        strInfo = tr("数据表：%1 完成手动更新！").arg(strReelTable);
    else
        strInfo = tr("数据表：%1 手动更新失败，请重试或检查文件！").arg(strReelTable);
    m_logManager->AddOneMsg(strInfo);
}

bool MainWindow::WriteToDB(QString strReelTable)
{
    QString strCsvPath = QString("F:/Inference/%1/result.csv").arg(strReelTable);
    QFile file(strCsvPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    if(!m_dbManager->ConnectToDB())
        return false;

    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString strLine(line);
        strLine = strLine.trimmed();
        if(strLine.isEmpty())
            continue;

        QStringList parts = strLine.split(',');

        int nFileIndex = 0;
        int nDefectIndex = 0;
        int nDefectLevel = 0;
        float fCentreX = -9.9999f;
        float fCentreY = -9.9999f;
        float fLength = -9.9999f;
        float fHeight = -9.9999f;

        if (parts.size() > 0 && !parts[0].isEmpty())
            nFileIndex = parts[0].toInt();
        if (parts.size() > 1 && !parts[1].isEmpty())
            nDefectIndex = parts[1].toInt();
        if (parts.size() > 2 && !parts[2].isEmpty())
            nDefectLevel = parts[2].toInt();
        if (parts.size() > 3 && !parts[3].isEmpty())
            fCentreX = parts[3].toFloat();
        if (parts.size() > 4 && !parts[4].isEmpty())
            fCentreY = parts[4].toFloat();
        if (parts.size() > 5 && !parts[5].isEmpty())
            fLength = parts[5].toFloat();
        if (parts.size() > 6 && !parts[6].isEmpty())
            fHeight = parts[6].toFloat();

        QString strRectCoordinate = QString("%1,%2,%3,%4").arg(fCentreX).arg(fCentreY).arg(fLength).arg(fHeight);

        if(strRectCoordinate.contains("-9.9999"))
            strRectCoordinate.clear();

        //QString strDefectName[64];
        //m_dbManager->UpdateDefectInfo(strReelTable,strDefectName[nDefectIndex],nFileIndex,nDefectLevel,strRectCoordinate);
        bool bUpdate = m_dbManager->UpdateDefectInfo(strReelTable,m_configManager->GetDefectName(nDefectIndex),nFileIndex,nDefectLevel,strRectCoordinate);
        if(!bUpdate)
            m_logManager->AddOneMsg("数据库更新失败！");
    }
    file.close();

    QSettings settings(m_strReelConfigPath,QSettings::IniFormat);
    settings.setValue("param/is_execute","1");

    return true;
}

// 自动更新数据库
void MainWindow::AutomaticUpdateDatabase(QString strReelTable)
{
    QString strInfo;
    bool bWrite = WriteToDB(strReelTable);
    if(bWrite)
        strInfo = tr("数据表：%1 完成自动更新!").arg(strReelTable);
    else
        strInfo = tr("数据表：%1 自动更新失败，请重试或手动更新!").arg(strReelTable);
    m_logManager->AddOneMsg(strInfo);
}

// 托盘
void MainWindow::ToTray()
{
    if(!QSystemTrayIcon::isSystemTrayAvailable())
    {
        m_logManager->AddOneMsg("当前系统不支持最小化到托盘");
        return;
    }

    if(!m_trayIcon)
    {
        m_trayIcon = new QSystemTrayIcon(this);
        m_trayIcon->setIcon(QIcon(":/src/icons/logo.png"));
        m_trayIcon->setToolTip("更新训练结果");

        QMenu *trayMenu = new QMenu(this);
        QAction *restoreAct = new QAction("显示窗口", this);
        QAction *quitAct = new QAction("退出", this);

        connect(restoreAct, &QAction::triggered, this, &MainWindow::showNormal);
        connect(quitAct, &QAction::triggered, qApp, &QApplication::quit);

        trayMenu->addAction(restoreAct);
        trayMenu->addSeparator();
        trayMenu->addAction(quitAct);

        m_trayIcon->setContextMenu(trayMenu);

        connect(m_trayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason){
            if(reason == QSystemTrayIcon::DoubleClick)
            {
                this->showNormal();
                this->activateWindow();
                DeleteTray();
            }
        });
    }

    m_trayIcon->show();
    this->hide();
}

void MainWindow::DeleteTray()
{
    if(m_trayIcon)
    {
        m_trayIcon->hide();
        delete m_trayIcon;
        m_trayIcon = nullptr;
    }
}

// 定时器回调
void MainWindow::onTimer1() { ExistNewReel(); }
void MainWindow::onTimer2() { HandleInferProcess(); m_timer2->stop(); }
void MainWindow::onTimer3() { ToTray(); m_timer3->stop(); }

// 检测新卷号
void MainWindow::ExistNewReel()
{
    QString strGetFilePath = m_configManager->GetFilePath();
    if(m_strReelConfigPath != strGetFilePath)
    {
        m_strReelConfigPath = QString("%1/config.ini").arg(strGetFilePath);
        m_strReelTable = m_configManager->GetReelTable(m_strReelConfigPath);

        if(m_strReelTable != "NULL")
        {
            m_logManager->AddOneMsg(QString("找到新文件：%1").arg(m_strReelTable));

            QSettings setReelConfig(m_strReelConfigPath, QSettings::IniFormat);
            bool bIsExecute = setReelConfig.value("param/is_execute", 0).toInt() != 0;

            if(!bIsExecute)
            {
                bool bOpen = QProcess::startDetached("explorer.exe", QStringList() << "file:///C:/Users/adv/Desktop/test_script.sh");
                if(bOpen)
                    m_logManager->AddOneMsg(QString("%1 正在进行缺陷分类分级...").arg(m_strReelTable));
                else
                    m_logManager->AddOneMsg("深度学习程序未能成功启动，请检查文件路径或手动重试。");
            }
        }

        m_timer2->start(2000);
    }
}

// 处理 Python 深度学习程序
void MainWindow::HandleInferProcess()
{
    QSettings settings(m_strReelConfigPath, QSettings::IniFormat);
    int nIsInfer = settings.value("param/is_infer", 0).toInt();

    if(nIsInfer == 1)
    {
        bool bClose = terminateProcessByName("mintty.exe");
        if(!bClose)
            m_logManager->AddOneMsg("深度学习程序未能成功关闭");

        m_logManager->AddOneMsg("缺陷分类完成，正在写入数据库...");
        settings.setValue("param/is_infer", 0);
        settings.sync();    //立即写入

        AutomaticUpdateDatabase(m_strReelTable);
    }
    else if(nIsInfer == 2)
    {
        terminateProcessByName("mintty.exe");
        m_logManager->AddOneMsg("训练异常，请重试！");
    }
}

// 杀死进程
bool MainWindow::terminateProcessByName(const QString &procName)
{
#ifdef Q_OS_WIN
    int ret = QProcess::execute("taskkill", {"/IM", procName, "/F"});
#else
    int ret = QProcess::execute("pkill", {procName});
#endif
    return ret == 0;
}

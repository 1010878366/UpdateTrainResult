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
    //, m_trayIcon(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("更新训练结果 V1.1.5");

    m_strPathConfig = QString("F:/Inference/path_config.ini");

    // 初始化核心对象
    m_configManager = new ConfigManager(m_strPathConfig);
    m_dbManager = new DatabaseManager("10.169.70.170", "DB_CENTRAL_UI", "kexin2008");
    m_logManager = new LogManager(QCoreApplication::applicationDirPath() + "/../../UpdateLog/");
    m_logManager->SetTextEdit(ui->textEdit_Info);
    m_trayManager = new TrayManager(this);
    m_timerManager = new TimerManager(this,this);

    // 按钮绑定
    connect(ui->btn_Open, &QPushButton::clicked, this, &MainWindow::OpenButton);
    connect(ui->btn_Write, &QPushButton::clicked, this, &MainWindow::WriteButton);
    connect(ui->btn_MiniToTray, &QPushButton::clicked, this, &MainWindow::ToTray);
    connect(ui->btn_Close, &QPushButton::clicked, this, &MainWindow::close);

    m_logManager->AddOneMsg("自动更新程序已经启动！");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_configManager;
    delete m_dbManager;
    delete m_logManager;
    delete m_trayManager;
    delete m_timerManager;
}

// 手动选择配置文件
void MainWindow::OpenButton()
{
    //1.手动选择[卷号]config.ini文件
    //QString strReelConfigPath = QFileDialog::getOpenFileName(this, tr("选择配置文件"), QCoreApplication::applicationDirPath(), tr("INI文件(*.ini)"));
    QString strReelConfigPath = QFileDialog::getOpenFileName(this, tr("选择配置文件"), "F:/Inference/", tr("INI文件(*.ini)"));
    if(strReelConfigPath.isEmpty())
        return;

    //2.显示路径到lineEdit
    ui->lineEdit_Path->setText(strReelConfigPath);
    QString strInfo = tr("手动选择路径:%1").arg(strReelConfigPath);
    m_logManager->AddOneMsg(strInfo);

    //3.读取reel_table参数
    m_currentReelTable = m_configManager->GetReelTable(strReelConfigPath);

    // 提示用户可以点击写入按钮
    if(!m_currentReelTable.isEmpty())
    {
        strInfo = tr("已获取数据表信息：%1，请点击[写入数据库]按钮执行更新").arg(m_currentReelTable);
        m_logManager->AddOneMsg(strInfo);
    }
    else
    {
        m_logManager->AddOneMsg(tr("配置文件中未找到有效的数据表信息"));
        m_currentReelTable.clear(); // 清空无效数据
    }
}

void MainWindow::WriteButton()
{
    //1.检查是否有有效的表名
    if(m_currentReelTable.isEmpty())
    {
        QMessageBox::warning(this, tr("警告"), tr("请先通过[打开]按钮选择有效的配置文件"));
        return;
    }

    //2.记录开始写入的日志（主线程中记录）
    QString strInfo = tr("【%1】正在写入数据库").arg(m_currentReelTable);
    m_logManager->AddOneMsg(strInfo);

    //3.调用实际写入函数（后续可移到子线程）
    bool bWrite = performWriteToDB(m_currentReelTable);

    //4.记录写入结果日志（主线程中记录）
    if (bWrite)
        strInfo = tr("数据表【%1】完成手动更新！").arg(m_currentReelTable);
    else
        strInfo = tr("数据表【%1】手动更新失败，请重试或检查文件！").arg(m_currentReelTable);
    m_logManager->AddOneMsg(strInfo);
}

bool MainWindow::performWriteToDB(const QString& strReelTable)
{
    // 仅执行写入操作，不涉及任何UI或日志
    // 直接调用原有的WriteToDB函数
    return WriteToDB(strReelTable);
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
    QString strReelConfigPath = QString("F:/Inference/%1/config.ini").arg(strReelTable);
    QSettings settings(strReelConfigPath,QSettings::IniFormat);
    settings.setValue("param/is_execute","1");

    return true;
}

// 自动更新数据库
void MainWindow::AutomaticUpdateDatabase(QString strReelTable)
{
    QString strInfo;
    bool bWrite = WriteToDB(strReelTable);
    if(bWrite)
        strInfo = tr("数据表【%1】完成自动更新!").arg(strReelTable);
    else
        strInfo = tr("数据表【%1】自动更新失败，请重试或手动更新!").arg(strReelTable);
    m_logManager->AddOneMsg(strInfo);
}

// 托盘
void MainWindow::ToTray()
{
    if(!m_trayManager->isTrayAvailable())
    {
        m_logManager->AddOneMsg("当前系统不支持最小化到托盘");
        return;
    }

    m_trayManager->createTray(this);
}

void MainWindow::DeleteTray()
{
    m_trayManager->deleteTray();
}

// 检测新卷号
void MainWindow::ExistNewReel()
{
    QString strLog;
    //1.判断是否获取到最新文件
    QString strGetFilePath,strGetNewFilePath;
    strGetFilePath = m_configManager->GetFilePath();
    strGetNewFilePath = m_configManager->GetNewFilePath(m_strPathConfig);
    if(strGetFilePath != strGetNewFilePath)
    {
        m_configManager->GetFilePath() = strGetNewFilePath;
        m_strReelConfigPath = strGetNewFilePath+"/config.ini";
        m_strReelTable = m_configManager->GetReelTable(m_strReelConfigPath);
        if(m_strReelTable != "NULL")
        {
            strLog = QString("找到新文件【%1】").arg(m_strReelTable);
            m_logManager->AddOneMsg(strLog);

            //2.进行Python深度学习步骤

            QSettings setReelConfig(m_strReelConfigPath,QSettings::IniFormat);
            bool b_IsExecute = setReelConfig.value("param/is_execute",0).toInt()!=0;
            if(!b_IsExecute)
            {
                bool bOpen = QProcess::startDetached("explorer.exe", QStringList() << "file:///C:/Users/adv/Desktop/test_script.sh");
                if(bOpen)
                {
                    strLog = QString("【%1】正在进行缺陷分类分级...").arg(m_strReelTable);
                    m_logManager->AddOneMsg(strLog);
                }
                else
                    m_logManager->AddOneMsg("深度学习程序未能成功启动，请检查文件路径或者手动重试。");
            }
        }
        else
        {
            return;
        }
    }
    //3.循环判断Python深度学习步骤是否完成，完成立即关闭
    m_timerManager->startTimer2();
    //m_timer2->start(2000);
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

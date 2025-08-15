#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTextCodec>
#pragma execution_character_set("utf-8")

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("更新训练结果 V1.0.8");

    m_pAdo = nullptr;

    //读取路径配置
    QString strPathConfig = QString("F:/Inference/path_config.ini");
    QSettings setFilePath(strPathConfig,QSettings::IniFormat);

    m_strFilePath = setFilePath.value("param/file_path","NULL").toString();

    //读取缺陷名称映射表
    QSettings setDefectMap("D:/DefectMap.ini",QSettings::IniFormat);
    setDefectMap.setIniCodec(QTextCodec::codecForName("GB2312"));
    setDefectMap.beginGroup("Param");
    for(int i=0;i<64;i++)
    {
        QString strKey = QString::number(i);
        m_strDefectName[i]=setDefectMap.value(strKey,"无分类").toString();
    }
    setDefectMap.endGroup();

    //定时器
    m_timer1 = new QTimer(this);
    connect(m_timer1,&QTimer::timeout,this,&MainWindow::onTimer1);
    m_timer1->start(2000);

    m_timer2 = new QTimer(this);
    connect(m_timer2,&QTimer::timeout,this,&MainWindow::onTimer2);
    m_timer2->setSingleShot(true);  //单次触发模式
    //m_timer2->start(2000);

    m_timer3 = new QTimer(this);
    connect(m_timer3,&QTimer::timeout,this,&MainWindow::onTimer3);
    m_timer3->setSingleShot(true);
    m_timer3->start(3000);

    connect(ui->btn_Open,&QPushButton::clicked,this,&MainWindow::OpenButton);
    connect(ui->btn_MiniToTray,&QPushButton::clicked,this,&MainWindow::ToTray);
    connect(ui->btn_Close,&QPushButton::clicked,this,&MainWindow::close);

}

MainWindow::~MainWindow()
{
    delete ui;

    if(m_logFile.isOpen())
    {
        m_logFile.close();
    }
}


void MainWindow::OpenButton()
{
    //1.选择.ini文件
    QString strReelConfigPath  = QFileDialog::getOpenFileName(this,tr("选择配置文件"),QCoreApplication::applicationDirPath(),tr("INI文件(*.ini)"));
    if(strReelConfigPath.isEmpty())
        return; //用户取消选择

    //2.显示路径到LineEdit
    ui->lineEdit_Path->setText(strReelConfigPath);

    //3.读取reel_name参数
    QSettings settings(strReelConfigPath,QSettings::IniFormat);
    QString strReelTable = settings.value("param/reel_table","").toString();

    QString strInfo;
    strInfo = tr("正在写入数据库");
    AddOneMsg(strInfo);

    bool bWrite = WriteToDB(strReelTable);
    if (bWrite)
        strInfo = tr("数据表：%1 完成手动更新！").arg(strReelTable);
    else
        strInfo = tr("数据表：%1 手动更新失败，请重试或检查文件！").arg(strReelTable);
    AddOneMsg(strInfo);
}

void MainWindow::AutomaticUpdateDatebase(QString strReelTable)
{
    QString strInfo;
    bool bWrite = WriteToDB(strReelTable);
    if(bWrite)
        strInfo = tr("数据表：%1 完成自动更新!").arg(strReelTable);
    else
        strInfo = tr("数据表：%1 自动更新失败，请重试或手动更新!").arg(strReelTable);
    AddOneMsg(strInfo);
}

bool MainWindow::ConnectToDatabase(ADOLinkToBase *&pAdo)
{
    if(pAdo)
    {
        delete pAdo;
        pAdo = nullptr;
    }
    pAdo = new ADOLinkToBase;
    bool bDBConnect = pAdo->Connection(tr("10.169.70.170"),tr("DB_CENTRAL_UI"),tr("kexin2008"),true);
    //bool bDBConnect = pAdo->Connection(tr("127.0.0.1"),tr("DB_CENTRAL_UI"),tr("kexin2008"),true);
    return bDBConnect;
}

void MainWindow::DisConnectFromDatabase(ADOLinkToBase *&pAdo)
{
    if (pAdo)
    {
        delete pAdo;
        pAdo = nullptr;
    }
}

bool MainWindow::WriteToDB(QString strReelTable)
{
    QString strCsvPath = QString("F:/Inference/%1/result.csv").arg(strReelTable);

    QFile file(strCsvPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    if(!ConnectToDatabase(m_pAdo))
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

        UpdateDefectInfo(m_pAdo,strReelTable,m_strDefectName[nDefectIndex], nFileIndex,nDefectLevel,strRectCoordinate);
    }
    file.close();

    DisConnectFromDatabase(m_pAdo);

    QSettings settings(m_strReelConfigPath,QSettings::IniFormat);
    settings.setValue("param/is_execute","1");

    return true;
}

void MainWindow::UpdateDefectInfo(ADOLinkToBase* pAdo,QString strTableName,QString strInfo,int nIndex,int nLevel,QString strRectCoordinate)
{
    QString strSQL = QString("UPDATE [%1] SET "
                            "Feature_Name14 = '%2', "
                            "Feature_nValue14 = %3, "
                            "Feature_Name13 = '%4' "
                            "WHERE Index_InQue = %5")
            .arg(strTableName).arg(escapeSingleQuotes(strInfo)).arg(static_cast<float>(nLevel)).arg(escapeSingleQuotes(strRectCoordinate)).arg(nIndex);

    if(!pAdo->Execute(strSQL))
    {
        QString strErr = QString("数据库更新失败！SQL语句：%1").arg(strSQL);
        AddOneMsg(strErr);
    }
}

QString MainWindow::escapeSingleQuotes(const QString &str)
{
    QString result = str;
    return result.replace("'", "''"); // Qt字符串替换，将单引号转为两个单引号
}


void MainWindow::AddOneMsg(QString strInfo)
{
    //获取当前时间
    QDateTime time = QDateTime::currentDateTime();
    QString strTime = time.toString("yyyy-MM-dd HH:mm:ss");

    //拼接带时间的消息
    QString strMsg = strTime+"  "+strInfo;

    //添加到列表控件并滚动到最后
    ui->textEdit_Info->append(strMsg);

    //记录到日志文件
    QString strMonth = time.toString("yyyy-MM");
    QString strDay = time.toString("dd");
    AddOneLog(strMonth,strDay,strMsg);
}

void MainWindow::AddOneLog(QString strMonth, QString strDay, QString strInfo)
{
    QString appDir=QCoreApplication::applicationDirPath();
    QString strPath = QString("%1/../../UpdateLog/%2/").arg(appDir).arg(strMonth);
    QDir dir;

    if(!dir.exists(strPath))
    {
        dir.mkpath(strPath);
    }

    QString strFilePath = strPath+strDay+"log.txt";
    m_logFile.setFileName(strFilePath); //绑定日志文件路径

    if(m_logFile.open(QIODevice::Append|QIODevice::Text))
    {
        QTextStream out(&m_logFile);
        out <<strInfo<<"\r\n";
        m_logFile.close();
    }
}

void MainWindow::ToTray()
{
    if(!QSystemTrayIcon::isSystemTrayAvailable())
    {
        AddOneMsg("当前系统不支持最小化到托盘");
        return;
    }

    //创建托盘图标
    if(!m_trayIcon)
    {
        m_trayIcon = new QSystemTrayIcon(this);
        m_trayIcon->setIcon(QIcon(":/src/icons/logo.png"));
        m_trayIcon->setToolTip("更新训练结果");

        //创建托盘右键菜单
        QMenu *trayMenu = new QMenu(this);
        QAction *restoreAct = new QAction("显示窗口",this);
        QAction *quitAct = new QAction("退出",this);

        connect(restoreAct,&QAction::triggered,this,&MainWindow::showNormal);
        connect(quitAct, &QAction::triggered, qApp, &QApplication::quit);

        trayMenu->addAction(restoreAct);
        trayMenu->addSeparator();
        trayMenu->addAction(quitAct);

        m_trayIcon->setContextMenu(trayMenu);

        connect(m_trayIcon,&QSystemTrayIcon::activated,this,[=](QSystemTrayIcon::ActivationReason reason)
        {
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

void MainWindow::onTimer1()
{
    ExistNewReel();
}

void MainWindow::onTimer2()
{
    HandleInferProcess();
    m_timer2->stop();
}

void MainWindow::onTimer3()
{
    ToTray();
    m_timer3->stop();
}

void MainWindow::ExistNewReel()
{
    //1.判断是否获取到最新文件
    QString strPathConfig = "F:/Inference/path_config.ini";
    QString strGetFilePath;
    QString strLog;

    QSettings setFilePath(strPathConfig,QSettings::IniFormat);
    strGetFilePath = setFilePath.value("param/file_path","NULL").toString();

    if(m_strFilePath != strGetFilePath)
    {
        m_strFilePath = strGetFilePath;
        m_strReelConfigPath = QString("%1/config.ini").arg(strGetFilePath);

        QSettings setReelConfig(m_strReelConfigPath,QSettings::IniFormat);
        m_strReelTable = setReelConfig.value("param/reel_table","NULL").toString();

        if(m_strReelTable != "NULL")
        {
            strLog = QString("找到新文件：%1").arg(m_strReelTable);
            AddOneMsg(strLog);

            //2.进行Python深度学习步骤
            bool b_IsExecute = setReelConfig.value("param/is_execute",0).toInt()!=0;

            if(!b_IsExecute)
            {
                bool bOpen = QProcess::startDetached("explorer.exe", QStringList() << "file:///C:/Users/adv/Desktop/test_script.sh");
                if(bOpen)
                {
                    strLog=QString("%1 正在进行缺陷分类分级...").arg(m_strReelTable);
                    AddOneMsg(strLog);
                }
                else
                    AddOneMsg("深度学习程序未能成功启动，请检查文件路径或者手动重试。");
            }
        }
        else
        {
            return;
        }
    }
    //3.循环判断Python深度学习步骤是否完成，完成立即关闭
    m_timer2->start(2000);

}

void MainWindow::HandleInferProcess()
{
    QSettings settings(m_strReelConfigPath,QSettings::IniFormat);
    int nIsInfer = settings.value("param/is_infer",0).toInt();

    if(nIsInfer == 1)
    {
        //关闭外部程序
        bool bClose = terminateProcessByName("mintty.exe");
        if(!bClose)
            AddOneMsg("深度学习程序未能成功关闭");
        AddOneMsg("缺陷分类完成，正在写入数据库...");

        settings.setValue("param/is_infer",0);
        settings.sync();    //立即写入

        AutomaticUpdateDatebase(m_strReelTable);
    }
    else if(nIsInfer == 2)
    {
        bool bClose = terminateProcessByName("mintty.exe");
        if(!bClose)
            AddOneMsg("深度学习程序未能成功关闭");
        AddOneMsg("训练异常，请重试！");
    }
    else
    {
        return;
    }
}

bool MainWindow::terminateProcessByName(const QString &procName)
{
#ifdef Q_OS_WIN
    int ret = QProcess::execute("taskkill", {"/IM", procName, "/F"});
#else
    int ret = QProcess::execute("pkill", {procName});
#endif

    if(ret == 0)
        return true;    //命令执行成功，进程已结束
    else
        return false;   //命令执行失败，进程未结束或不存在
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTextCodec>
#pragma execution_character_set("utf-8")

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("更新训练结果 V1.0.4");




    connect(ui->btn_Open,&QPushButton::clicked,this,&MainWindow::OpenButton);
    connect(ui->btn_MiniToTray,&QPushButton::clicked,this,&MainWindow::ToTray);
    connect(ui->btn_Close,&QPushButton::clicked,this,&MainWindow::close);
}

MainWindow::~MainWindow()
{
    delete ui;
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
    QString strCsvPath = QString::fromLocal8Bit("F:\\Inference\\%s\\result.csv").arg(strReelTable);

    QFile file(strCsvPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    ConnectToDatabase(m_pAdo);

    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString strLine(line);
        strLine = strLine.trimmed();
        if(strLine.isEmpty())
            continue;

        QStringList parts = strLine.split(',');

        int nFileIndex = parts[0].toInt();
        int nDefectIndex = parts[1].toInt();
        int nDefectLevel = parts[2].toInt();
        float fCentreX = parts[3].toFloat();
        float fCentreY = parts[4].toFloat();
        float fLength = parts[5].toFloat();
        float fHeight = parts[6].toFloat();

        QString strRectCoordinate = QString("%1,%2,%3,%4").arg(fCentreX).arg(fCentreY).arg(fLength).arg(fHeight);

        if(strRectCoordinate.contains("-9.9999"))
            strRectCoordinate.clear();

        //UpdateDefectInfo(m_pAdo,strReelTable,m_strDefectName[nDefectIndex], nFileIndex,nDefectLevel,strRectCoordinate);
    }
    file.close();

    DisConnectFromDatabase(m_pAdo);

    QSettings settings(m_strReelConfigPath,QSettings::IniFormat);
    settings.setValue("param/is_execute","1");

    return true;
}

void MainWindow::AddOneMsg(QString strInfo)
{
    //获取当前时间
    QDateTime time = QDateTime::currentDateTime();
    QString strTime = time.toString("yyyy-MM-dd HH:mm:ss");

    //拼接带时间的消息
    QString strMsg = strTime+"  "+strInfo;

    //添加到列表控件并滚动到最后
    //m_listInfo->addItem(strInfo);
    //m_listInfo->setCurrentRow(m_listInfo->count()-1);
    ui->textEdit_Info->append(strMsg);


    //记录到日志文件
    QString strMonth = time.toString("yyyy-MM");
    QString strDay = time.toString("dd");
    AddOneLog(strMonth,strDay,strMsg);
}

void MainWindow::AddOneLog(QString strMonth, QString strDay, QString strInfo)
{
    QString strPath = QString("../../Log/%1/").arg(strMonth);
    QDir dir;

    if(!dir.exists(strPath))
    {
        dir.mkpath(strPath);
    }

    QString strFilePath = strPath+strDay+"log.txt";

    if(m_logFile.open(QIODevice::Append|QIODevice::Text))
    {
        QTextStream out(&m_logFile);
        out <<strInfo<<"/r/n";
        m_logFile.close();

    }
}

//bool MainWindow::MakeDirectory(const QString &strPathName)
//{
//    QDir dir;
//    return dir.mkpath(path);
//}

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
        m_trayIcon->setIcon(QIcon(":src/icons/logo.png"));
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


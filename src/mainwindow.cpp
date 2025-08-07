#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTextCodec>
#pragma execution_character_set("utf-8")

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);




    connect(ui->btn_Open,&QPushButton::clicked,this,&MainWindow::onOpenButtonClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onOpenButtonClicked()
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
    //AddOneMsg();

    bool bWriteToDB = WriteToDB(strReelTable);
    if (bWriteToDB)
    {
        strInfo = QString::fromLocal8Bit("数据表：%s 完成手动更新！").arg(strReelTable);
    }
    else
    {
        strInfo = QString::fromLocal8Bit("数据表：%s 手动更新失败，请重试或检查文件！").arg(strReelTable);
    }
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

        UpdateDefectInfo(m_pAdo,strReelTable,m_strDefectName[nDefectIndex], nFileIndex,nDefectLevel,strRectCoordinate);
    }
    file.close();

    DisConnectFromDatabase(m_pAdo);

    QSettings settings(m_strReelConfigPath,QSettings::IniFormat);
    settings.setValue("param/is_execute","1");

    return true;
}



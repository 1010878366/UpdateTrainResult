#include "TimerManager.h"
#include "mainwindow.h"

TimerManager::TimerManager(MainWindow* mainWindow, QObject* parent)
    : QObject(parent), m_mainWindow(mainWindow)
{
    // 定时器1
    m_timer1 = new QTimer(this);
    connect(m_timer1, &QTimer::timeout, this, &TimerManager::onTimer1);
    m_timer1->start(2000);

    // 定时器2
    m_timer2 = new QTimer(this);
    connect(m_timer2, &QTimer::timeout, this, &TimerManager::onTimer2);
    m_timer2->setSingleShot(true);

    // 定时器3
    m_timer3 = new QTimer(this);
    connect(m_timer3, &QTimer::timeout, this, &TimerManager::onTimer3);
    m_timer3->setSingleShot(true);
    m_timer3->start(3000);
}

TimerManager::~TimerManager()
{

}

void TimerManager::startTimer1()
{
    if (m_timer1) m_timer1->start(2000);
}

void TimerManager::startTimer2()
{
    if (m_timer2) m_timer2->start(2000);
}

void TimerManager::startTimer3()
{
    if (m_timer3) m_timer3->start(3000);
}

void TimerManager::stopTimers()
{
    if (m_timer1) m_timer1->stop();
    if (m_timer2) m_timer2->stop();
    if (m_timer3) m_timer3->stop();
}

// 定时器回调
void TimerManager::onTimer1()
{
    if (m_mainWindow) m_mainWindow->ExistNewReel();
}

void TimerManager::onTimer2()
{
    if (m_mainWindow)
        m_mainWindow->HandleInferProcess();
    if (m_timer2)
        m_timer2->stop();
}

void TimerManager::onTimer3()
{
    if (m_mainWindow) m_mainWindow->ToTray();
    if (m_timer3) m_timer3->stop();
}

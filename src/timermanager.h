#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include <QObject>
#include <QTimer>

class MainWindow; // 前向声明，避免循环依赖

class TimerManager : public QObject
{
    Q_OBJECT
public:
    explicit TimerManager(MainWindow* mainWindow, QObject* parent = nullptr);
    ~TimerManager();

    void startTimer1();
    void startTimer2();
    void startTimer3();
    void stopTimers();

private slots:
    void onTimer1();
    void onTimer2();
    void onTimer3();

private:
    MainWindow* m_mainWindow; // 回调到 MainWindow

    QTimer* m_timer1;
    QTimer* m_timer2;
    QTimer* m_timer3;
};

#endif // TIMERMANAGER_H

#ifndef TRAYMANAGER_H
#define TRAYMANAGER_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QObject>

class TrayManager : public QObject
{
    Q_OBJECT
public:
    explicit TrayManager(QObject *parent = nullptr);
    ~TrayManager();

    void createTray(QWidget *mainWindow);   // 创建并显示托盘
    void deleteTray();                      // 删除托盘
    bool isTrayAvailable() const;           // 判断系统是否支持托盘

private:
    QSystemTrayIcon* m_trayIcon;
};

#endif // TRAYMANAGER_H

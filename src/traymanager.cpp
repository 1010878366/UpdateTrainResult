#include "TrayManager.h"
#include <QApplication>
#include <QIcon>

TrayManager::TrayManager(QObject *parent)
    : QObject(parent), m_trayIcon(nullptr)
{
}

TrayManager::~TrayManager()
{
    deleteTray();
}

void TrayManager::createTray(QWidget *mainWindow)
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
        return;

    if (!m_trayIcon) {
        m_trayIcon = new QSystemTrayIcon(this);
        m_trayIcon->setIcon(QIcon(":/src/icons/logo1.png"));
        m_trayIcon->setToolTip("更新训练结果");

        QMenu *trayMenu = new QMenu(mainWindow);
        QAction *restoreAct = new QAction("显示窗口", trayMenu);
        QAction *quitAct = new QAction("退出", trayMenu);

        QObject::connect(restoreAct, &QAction::triggered, mainWindow, [mainWindow]() {
            mainWindow->showNormal();
            mainWindow->activateWindow();
        });
        QObject::connect(quitAct, &QAction::triggered, qApp, &QApplication::quit);

        trayMenu->addAction(restoreAct);
        trayMenu->addSeparator();
        trayMenu->addAction(quitAct);

        m_trayIcon->setContextMenu(trayMenu);

        QObject::connect(m_trayIcon, &QSystemTrayIcon::activated,
                         mainWindow, [=](QSystemTrayIcon::ActivationReason reason){
            if(reason == QSystemTrayIcon::DoubleClick) {
                mainWindow->showNormal();
                mainWindow->activateWindow();
                deleteTray();
            }
        });
    }

    m_trayIcon->show();
    mainWindow->hide();
}

void TrayManager::deleteTray()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
        delete m_trayIcon;
        m_trayIcon = nullptr;
    }
}

bool TrayManager::isTrayAvailable() const
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

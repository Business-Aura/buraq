//
// Created by talik on 5/29/2025.
//

#include "FramelessWindow.h"

#include <QStatusBar>
#include <QVBoxLayout>
#include <QSplitter>

#include "Config.h"
#include "../Filters/Toolbar/ToolBarEvent.h"
#include "CustomDrawer.h"
#include "IconButton.h"
#include "output_display/OutputDisplay.h"
#include "ToolBar.h"
#include "PluginManager.h"
#include "app_ui/AppUi.h"
#include "Filters/ThemeManager/ThemeManager.h"
#include "../settings/Dialog/SettingsDialog.h"
#include "settings/SettingManager/SettingsManager.h"

#include <QIcon>

#include "Frame/Frame.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#include <windowsx.h>
#endif

FramelessWindow::FramelessWindow(QWidget* parent)
    : QMainWindow(parent),
      themeManager(ThemeManager::instance()),
      userPreferences(SettingsManager::loadSettings())
{
    resize(QSize(1200, 700));

    // Set the m_window flag to remove the default frame
    setWindowFlags(Qt::FramelessWindowHint);

    // Generate the icon at runtime
    const auto appLogo = QIcon(":/icons/buraq.png");
    setWindowIcon(appLogo);

    m_Frame = new Frame(this, true, userPreferences.windowSize);

    // Initialize the ThemeManager instance
    installEventFilter(&themeManager);

    // Set the central widget for the QMainWindow
    setCentralWidget(m_Frame);

    // Custom Title Bar
    setupTitleBar();

    // Add Tool bar
    const auto toolkitBar = m_Frame->getToolKitBar();
    m_toolBar = new ToolBar(toolkitBar);
    m_toolBar->setFixedHeight(35);
    m_toolBar->addFileMenu(); // Add the File menu first
    if (const auto layout = toolkitBar->layout(); layout)
    {
        layout->addWidget(m_toolBar);
    }

    // Status bar
    const auto bottomPanel = m_Frame->getBottomPanelWidget();
    m_statusBar = new QStatusBar(bottomPanel);
    m_statusBar->setObjectName("appStatusBar");
    m_statusBar->setFixedHeight(25);
    bottomPanel->layout()->addWidget(m_statusBar);

    // Connections
    connect(this, &FramelessWindow::closeApp, this, &FramelessWindow::close);
    connect(this, &FramelessWindow::windowResize, m_Frame, &Frame::windowResizeSlot);
}

FramelessWindow::~FramelessWindow()
{
    // save the last window size & position
    userPreferences.windowSize = this->size();
    userPreferences.windowPosition = this->pos();
    SettingsManager::saveSettings(userPreferences);
}

void FramelessWindow::setupTitleBar()
{
    const auto titleBar = m_Frame->getTitleBar();
    
    m_minimizeButton = new QPushButton("—", titleBar);
    m_maximizeButton = new QPushButton("☐", titleBar);
    m_settingsButton = new QPushButton("⋮", titleBar);
    
    m_minimizeButton->setObjectName("minimizeButton");
    m_maximizeButton->setObjectName("maximizeButton");
    m_settingsButton->setObjectName("settingGearButton");
    m_settingsButton->setToolTip("IDE and Project Settings");

    m_settingsButton->setFixedSize(40, titleBar->height());
    m_minimizeButton->setFixedSize(40, titleBar->height());
    m_maximizeButton->setFixedSize(40, titleBar->height());

    const auto titleBarLayout = m_Frame->getExtraButtonsLayout();
    titleBarLayout->addStretch();
    titleBarLayout->addWidget(m_settingsButton);
    titleBarLayout->addWidget(m_minimizeButton);
    titleBarLayout->addWidget(m_maximizeButton);

    connect(m_minimizeButton, &QPushButton::clicked, this, &FramelessWindow::showMinimized);
    connect(m_maximizeButton, &QPushButton::clicked, this, &FramelessWindow::showMaximizeOrRestoreSlot);

    const auto settingsDialog = new SettingsDialog(this);
    connect(m_settingsButton, &QPushButton::clicked, settingsDialog, &SettingsDialog::exec);
    connect(settingsDialog, &SettingsDialog::applySettingChanges, this, &FramelessWindow::onApplySettingChanges);
}


bool FramelessWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
#if defined(Q_OS_WIN)
    MSG* msg = static_cast<MSG*>(message);

    if (msg->message == WM_NCHITTEST) {
        const LONG border_width = 8; //in pixels
        RECT winrect;
        GetWindowRect(reinterpret_cast<HWND>(winId()), &winrect);

        long x = GET_X_LPARAM(msg->lParam);
        long y = GET_Y_LPARAM(msg->lParam);

        bool resizeWidth = minimumWidth() != maximumWidth();
        bool resizeHeight = minimumHeight() != maximumHeight();

        if (resizeWidth) {
            //left border
            if (x >= winrect.left && x < winrect.left + border_width) {
                *result = HTLEFT;
                return true;
            }
            //right border
            if (x < winrect.right && x >= winrect.right - border_width) {
                *result = HTRIGHT;
                return true;
            }
        }
        if (resizeHeight) {
            //bottom border
            if (y < winrect.bottom && y >= winrect.bottom - border_width) {
                *result = HTBOTTOM;
                return true;
            }
            //top border
            if (y >= winrect.top && y < winrect.top + border_width) {
                *result = HTTOP;
                return true;
            }
        }
        if (resizeWidth && resizeHeight) {
            //bottom left corner
            if (x >= winrect.left && x < winrect.left + border_width &&
                y < winrect.bottom && y >= winrect.bottom - border_width) {
                *result = HTBOTTOMLEFT;
                return true;
            }
            //bottom right corner
            if (x < winrect.right && x >= winrect.right - border_width &&
                y < winrect.bottom && y >= winrect.bottom - border_width) {
                *result = HTBOTTOMRIGHT;
                return true;
            }
            //top left corner
            if (x >= winrect.left && x < winrect.left + border_width &&
                y >= winrect.top && y < winrect.top + border_width) {
                *result = HTTOPLEFT;
                return true;
            }
            //top right corner
            if (x < winrect.right && x >= winrect.right - border_width &&
                y >= winrect.top && y < winrect.top + border_width) {
                *result = HTTOPRIGHT;
                return true;
            }
        }
        
        // Check if the cursor is over the title bar area (m_Frame->getTitleBar())
        QPoint localMousePos = m_Frame->getTitleBar()->mapFromGlobal(QPoint(x, y));
        if (m_Frame->getTitleBar()->rect().contains(localMousePos)) {
            // Check if the cursor is over any of the buttons (use mapFromGlobal for each button)
            QPoint globalPos(x, y);
            if (m_minimizeButton->rect().contains(m_minimizeButton->mapFromGlobal(globalPos)) ||
                m_maximizeButton->rect().contains(m_maximizeButton->mapFromGlobal(globalPos)) ||
                m_settingsButton->rect().contains(m_settingsButton->mapFromGlobal(globalPos))) {
                // Let the button handle the event
                 return QMainWindow::nativeEvent(eventType, message, result);
            }
            *result = HTCAPTION;
            return true;
        }
    }
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

void FramelessWindow::closeWindowSlot()
{
    if (maybeSave())
    {
        emit closeApp();
    }
}

void FramelessWindow::showMaximizeOrRestoreSlot()
{
    if (this->isMaximized())
    {
        this->showNormal();
        m_maximizeButton->setText("☐"); // Maximize symbol
    }
    else
    {
        this->showMaximized();
        m_maximizeButton->setText("❐"); // Restore symbol
    }
}

void FramelessWindow::processStatusSlot(const QString& message, const int timeout) const
{
    if (m_statusBar)
    {
        m_statusBar->showMessage(message, timeout);
    }
}

PluginManager* FramelessWindow::getLangPluginManager() const
{
    return pluginManager;
}

void FramelessWindow::onApplySettingChanges()
{
    userPreferences = SettingsManager::loadSettings();
}

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

FramelessWindow::FramelessWindow(QWidget* parent)
    : QMainWindow(parent),
      themeManager(ThemeManager::instance()),
      m_outPutArea(std::make_unique<OutputDisplay>(this)),
      m_editor(std::make_unique<Editor>(this)),
      userPreferences(SettingsManager::loadSettings()),
      m_dragPosition(QPoint(0, 0))
{
    resize(QSize(1200, 700));
    //move(userPreferences.windowPosition);

    m_Frame = std::make_unique<Frame>(this, true, userPreferences.windowSize);

    // Initialize the ThemeManager instance
    installEventFilter(&themeManager);

    // Set the m_window flag to remove the default frame
    setWindowFlags(Qt::FramelessWindowHint);

    // Generate the icon at runtime
    const auto appLogo = QIcon(":/icons/buraq.ico");
    setWindowIcon(appLogo);

    // Set the central widget for the QMainWindow
    setCentralWidget(m_Frame.get());

    // Custom Title Bar
    const auto m_titleBar_ = m_Frame->getTitleBar();

    m_minimizeButton = std::make_unique<QPushButton>("—", m_titleBar_); // Underscore for minimize
    m_minimizeButton->setObjectName("minimizeButton"); // For specific styling

    m_maximizeButton = std::make_unique<QPushButton>("☐", m_titleBar_); // Square for maximize/restore
    m_maximizeButton->setObjectName("maximizeButton"); // For specific styling

    m_settingsButton = std::make_unique<QPushButton>("⋮", m_titleBar_);
    m_settingsButton->setToolTip("IDE and Project Settings");
    m_settingsButton->setObjectName("settingGearButton");

    m_settingsButton->setFixedSize(40, m_titleBar_->height());
    m_minimizeButton->setFixedSize(40, m_titleBar_->height());
    m_maximizeButton->setFixedSize(40, m_titleBar_->height());

    const auto titleBarLayout = m_Frame->getExtraButtonsLayout();
    titleBarLayout->addStretch();
    titleBarLayout->addWidget(m_settingsButton.get());
    titleBarLayout->addWidget(m_minimizeButton.get());
    titleBarLayout->addWidget(m_maximizeButton.get());

    // Add Tool bar
    const auto toolkitBar = m_Frame->getToolKitBar();
    m_toolBar = std::make_unique<ToolBar>(toolkitBar);
    m_toolBar->setFixedHeight(35);
    m_toolBar->addFileMenu(); // Add the File menu first
    if (const auto layout = toolkitBar->layout(); layout)
    {
        layout->addWidget(m_toolBar.get());
    }

    // Content Area (example)
    initContentAreaLayout(m_Frame->getMainContentWidget());

    // Status bar
    const auto bottomPanel = m_Frame->getBottomPanelWidget();
    m_statusBar = new QStatusBar(bottomPanel);
    m_statusBar->setObjectName("appStatusBar");
    m_statusBar->setFixedHeight(25);
    bottomPanel->layout()->addWidget(m_statusBar);

    // Connections
    connect(m_maximizeButton.get(), &QPushButton::clicked, this, &FramelessWindow::showMaximizeOrRestoreSlot);
    connect(m_minimizeButton.get(), &QPushButton::clicked, this, &FramelessWindow::showMinimized);
    connect(this, &FramelessWindow::closeApp, this, &FramelessWindow::close);
    connect(this, &FramelessWindow::windowResize, m_Frame.get(), &Frame::windowResizeSlot);

    // Create an instance of your settings dialog
    const auto settingsDialog = new SettingsDialog(this);
    // Connect the button's click signal to open the dialog
    connect(m_settingsButton.get(), &QPushButton::clicked, settingsDialog, &SettingsDialog::exec);
}

// smart pointers will be cleaned up by std::unique_ptr
FramelessWindow::~FramelessWindow()
{
    // save the last window size & position
    userPreferences.windowSize = this->size();
    userPreferences.windowPosition = m_dragPosition;
    SettingsManager::saveSettings(userPreferences);
};

void FramelessWindow::closeWindowSlot()
{
    emit closeApp();
}

void FramelessWindow::showMaximizeOrRestoreSlot()
{
    emit windowResize(this->size());
    if (this->isMaximized())
    {
        this->showNormal();
        m_maximizeButton->setText("☐"); // Restore symbol
    }
    else
    {
        this->showMaximized();
        m_maximizeButton->setText("❐"); // Actual maximize symbol (might need specific font or icon)
    }
    m_Frame->setMinimumSize(this->size());
}

void FramelessWindow::initContentAreaLayout(QWidget* mainContentArea)
{
    m_drawer = std::make_unique<CustomDrawer>(m_editor.get());

    const auto contentArea = new QWidget(mainContentArea);
    mainContentArea->layout()->addWidget(contentArea);

    // 1. MAIN LAYOUT: A horizontal layout to separate the left controls from the main content.
    QHBoxLayout* mainLayout = new QHBoxLayout(contentArea);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 2. LEFT CONTROL PANEL: A vertical layout for the top and bottom buttons.
    const auto leftPanelLayout = m_Frame->getLeftSidePanelLayout();

    m_folderButton = std::make_unique<QPushButton>("🗀", contentArea);
    m_folderButton->setIconSize(QSize(32, 32));
    m_folderButton->setFixedSize(35, 35);

    connect(m_folderButton.get(), &QPushButton::clicked, this, &FramelessWindow::updateDrawer);

    m_outputButton = std::make_unique<QPushButton>("❯_", contentArea);
    m_outputButton->setFixedSize(35, 35);

    connect(m_outputButton.get(), &QPushButton::clicked, this, &FramelessWindow::onShowOutputButtonClicked);

    leftPanelLayout->addWidget(m_folderButton.get()); // Add button to the top
    leftPanelLayout->addStretch(); // Pushes buttons to top and bottom
    leftPanelLayout->addWidget(m_outputButton.get()); // Add button to the bottom

    // 3a. Top Area (Editor + Drawer)
    QWidget* topAreaWidget = new QWidget();
    QHBoxLayout* topAreaLayout = new QHBoxLayout(topAreaWidget);
    topAreaLayout->setSpacing(0);
    topAreaLayout->setContentsMargins(0, 0, 0, 0);

    // 3. MAIN VERTICAL SPLITTER: Separates the top (editor/drawer) from the bottom (output).
    rightSideSplitter = std::make_unique<QSplitter>(Qt::Vertical);

    // 4. HORIZONTAL SPLITTER: This will go in the top section of the vertical splitter.
    // It separates the drawer from the editor.
    topAreaSplitter = std::make_unique<QSplitter>(Qt::Horizontal);

    // Add the drawer and editor to the HORIZONTAL splitter
    topAreaSplitter->addWidget(m_drawer.get());
    topAreaSplitter->addWidget(m_editor.get());
    topAreaSplitter->setSizes({250, 750}); // Initial widths for drawer and editor

    // 5. BOTTOM AREA (Output)

    // 6. ASSEMBLE THE VERTICAL SPLITTER:
    // Add the horizontal splitter (as the top widget) and the output area (as the bottom widget).
    rightSideSplitter->addWidget(topAreaSplitter.get());
    rightSideSplitter->addWidget(m_outPutArea.get());
    rightSideSplitter->setSizes({500, 200}); // Initial heights for top and bottom sections

    // 7. ASSEMBLE THE MAIN LAYOUT
    mainLayout->addWidget(rightSideSplitter.get(), 1); // The '1' stretch factor allows it to expand
}

void FramelessWindow::processStatusSlot(const QString& message, const int timeout) const
{
    if (m_statusBar)
    {
        m_statusBar->showMessage(message, timeout);
    }
}

void FramelessWindow::processResultSlot(const int exitCode, const QString& output, const QString& error) const
{
    if (m_outPutArea == nullptr) return;

    if (m_outPutArea->show(); exitCode == 0)
    {
        m_outPutArea->log(output, error);

        processStatusSlot(error.isEmpty() ? "Completed!" : "Completed with errors.");
    }
    else
    {
        processStatusSlot("Process failed!");
        m_outPutArea->log("", error);
    }
}

void FramelessWindow::updateDrawer() const
{
    qDebug() << "Open or close Drawer";
    if (m_drawer == nullptr) return;

    if (m_drawer->isHidden())
    {
        m_drawer->show();
    }
    else
    {
        m_drawer->hide();
    }
}

void FramelessWindow::onShowOutputButtonClicked() const
{
    qDebug() << "Open or close Output";
    if (m_outPutArea == nullptr) return;

    if (m_outPutArea->isHidden())
    {
        m_outPutArea->show();
    }
    else
    {
        m_outPutArea->hide();
    }
}

Editor* FramelessWindow::getEditor() const
{
    return m_editor.get();
}

PluginManager* FramelessWindow::getLangPluginManager() const
{
    return pluginManager.get();
}

Qt::Edges FramelessWindow::calculateEdges(const QPoint& pos, const int margin) const
{
    Qt::Edges edges;
    if (pos.x() < margin) edges |= Qt::LeftEdge;
    if (pos.x() > width() - margin) edges |= Qt::RightEdge;
    if (pos.y() < margin) edges |= Qt::TopEdge;
    if (pos.y() > height() - margin) edges |= Qt::BottomEdge;
    return edges;
}

void FramelessWindow::updateCursorShape(const QPoint& pos)
{
    if (m_resizing)
        return;

    m_resizeEdges = calculateEdges(pos, m_resizeMargin);

    if (m_resizeEdges == (Qt::TopEdge | Qt::LeftEdge) || m_resizeEdges == (Qt::BottomEdge | Qt::RightEdge))
        setCursor(Qt::SizeFDiagCursor);
    else if (m_resizeEdges == (Qt::TopEdge | Qt::RightEdge) || m_resizeEdges == (Qt::BottomEdge | Qt::LeftEdge))
        setCursor(Qt::SizeBDiagCursor);
    else if (m_resizeEdges & (Qt::LeftEdge | Qt::RightEdge))
        setCursor(Qt::SizeHorCursor);
    else if (m_resizeEdges & (Qt::TopEdge | Qt::BottomEdge))
        setCursor(Qt::SizeVerCursor);
    else
        setCursor(Qt::ArrowCursor);
}

void FramelessWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (m_resizeEdges != 0)
        {
            m_resizing = true;
            m_dragPosition = event->globalPosition().toPoint();
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void FramelessWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (m_resizing)
    {
        const QPoint currentPos = event->globalPosition().toPoint();
        const QPoint delta = currentPos - m_dragPosition;
        QRect newGeometry = geometry();

        if (m_resizeEdges & Qt::LeftEdge) newGeometry.setLeft(newGeometry.left() + delta.x());
        if (m_resizeEdges & Qt::RightEdge) newGeometry.setRight(newGeometry.right() + delta.x());
        if (m_resizeEdges & Qt::TopEdge) newGeometry.setTop(newGeometry.top() + delta.y());
        if (m_resizeEdges & Qt::BottomEdge) newGeometry.setBottom(newGeometry.bottom() + delta.y());

        if (newGeometry.width() < minimumWidth()) newGeometry.setLeft(geometry().left());
        if (newGeometry.height() < minimumHeight()) newGeometry.setTop(geometry().top());

        setGeometry(newGeometry);
        m_dragPosition = currentPos;
        emit windowResize(this->size());
    }
    else if (m_dragging)
    {
        move(event->globalPosition().toPoint() - m_dragPosition);
    }
    else
    {
        updateCursorShape(event->pos());
    }
    QWidget::mouseMoveEvent(event);
}

void FramelessWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = false;
        m_resizing = false;
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mouseReleaseEvent(event);
}

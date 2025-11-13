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
      m_frameContainer(std::make_unique<QWidget>(this)),
      m_titleBar(std::make_unique<QWidget>(this)),
      m_topPanel(std::make_unique<QWidget>(this)),
      m_rightSidePanel(std::make_unique<QWidget>(this)),
      m_bottomPanel(std::make_unique<QWidget>(this)),
      m_centralWidget(std::make_unique<QWidget>(this)),
      userPreferences(SettingsManager::loadSettings()),
      m_dragPosition(QPoint(0, 0))
{
    resize(userPreferences.windowSize);
    //move(userPreferences.windowPosition);

    // Initialize the ThemeManager instance
    installEventFilter(&themeManager);

    // Set the m_window flag to remove the default frame
    setWindowFlags(Qt::FramelessWindowHint);

    // Generate the icon at runtime
    const auto appLogo = QIcon(":/icons/buraq.ico");
    setWindowIcon(appLogo);

    // Set the central widget for the QMainWindow
    // 1. Create a main container widget. This will be the single central widget.
    setCentralWidget(m_frameContainer.get());
    m_frameContainer->setObjectName("Frame");

    // 2. Create the grid layout that the container will use.
    QGridLayout* mainGridLayout = new QGridLayout(m_frameContainer.get());
    mainGridLayout->setContentsMargins(0, 0, 0, 0); // No margins for a seamless frame
    mainGridLayout->setSpacing(0); // No spacing between frame parts

    // titleBar
    {
        const auto m_titleBar_ = m_titleBar.get();
        m_titlebarEvents = std::make_unique<ToolBarEvent>(m_titleBar_);
        m_titleBar_->setFixedHeight(35); // Set your desired title bar height
        m_titleBar_->setObjectName("customTitleBar"); // For styling

        const auto titleBarLayout = new QHBoxLayout(m_titleBar_);
        titleBarLayout->setContentsMargins(0, 0, 0, 0);
        titleBarLayout->setSpacing(0);

        m_closeButton = std::make_unique<QPushButton>("✕", m_titleBar_); // X for close
        m_closeButton->setObjectName("closeButton"); // For specific styling
        m_closeButton->setFixedSize(m_titleBar_->height(), m_titleBar_->height());

        m_minimizeButton = std::make_unique<QPushButton>("—", m_titleBar_); // Underscore for minimize
        m_minimizeButton->setObjectName("minimizeButton"); // For specific styling

        m_maximizeButton = std::make_unique<QPushButton>("☐", m_titleBar_); // Square for maximize/restore
        m_maximizeButton->setObjectName("maximizeButton"); // For specific styling

        m_settingsButton = std::make_unique<QPushButton>("⋮", m_titleBar_);
        m_settingsButton->setToolTip("IDE and Project Settings");
        m_settingsButton->setObjectName("settingGearButton");

        m_settingsButton->setFixedSize(35, m_titleBar_->height());
        m_minimizeButton->setFixedSize(35, m_titleBar_->height());
        m_maximizeButton->setFixedSize(35, m_titleBar_->height());

        // logo and version
        const auto logoAndVersion = new QWidget(m_titleBar_);
        const auto logoAndVersionLayout = new QHBoxLayout(logoAndVersion);
        logoAndVersionLayout->setContentsMargins(0, 0, 0, 0);

        QPushButton* iconButton = new QPushButton(m_titleBar_);
        iconButton->setIcon(parent->windowIcon());
        iconButton->setIconSize(QSize(32, 32));
        iconButton->setFixedSize(m_titleBar_->height(), m_titleBar_->height()); // Give some padding around the icon
        iconButton->setFlat(true); // Makes it look less like a bulky button
        iconButton->move(20, 70);

        const auto version = new QLabel(m_titleBar_);
        version->setObjectName("titleText");
        version->setText("v2.0.0");

        logoAndVersionLayout->addWidget(iconButton);
        logoAndVersionLayout->addStretch();
        logoAndVersionLayout->addWidget(version);

        titleBarLayout->addWidget(logoAndVersion);

        // end Logo and Version

        titleBarLayout->addStretch();
        titleBarLayout->addStretch();
        titleBarLayout->addWidget(m_settingsButton.get());
        titleBarLayout->addWidget(m_minimizeButton.get());
        titleBarLayout->addWidget(m_maximizeButton.get());
        titleBarLayout->addWidget(m_closeButton.get());
    }

    // Toolkit
    {
        m_topPanel->setFixedHeight(35); // Set your desired title bar height
        const auto toolKitLayout = new QHBoxLayout(m_topPanel.get());
        toolKitLayout->setContentsMargins(0, 0, 0, 0);
        toolKitLayout->setSpacing(0);
        toolKitLayout->setObjectName("topPanel");

        // Add Tool bar
        m_toolBar = std::make_unique<ToolBar>(m_topPanel.get());
        m_toolBar->setFixedHeight(35);
        m_toolBar->addFileMenu(); // Ad d the File menu first
        toolKitLayout->addWidget(m_toolBar.get());
    }

    // left side panel
    {
        m_leftSidePanel = std::make_unique<QWidget>(this);
        m_leftSidePanelLayout = std::make_unique<QVBoxLayout>(m_leftSidePanel.get());
        m_leftSidePanelLayout->setContentsMargins(0, 0, 0, 0); // No margins for the main layout
        m_leftSidePanelLayout->setSpacing(0);

        m_leftSidePanel->setFixedWidth(35);
        m_leftSidePanel->setObjectName("leftPanel");
    }

    // Central area
    {
        m_mainLayout = std::make_unique<QVBoxLayout>(m_centralWidget.get()); // Apply layout to the central widget
        m_mainLayout->setContentsMargins(0, 0, 0, 0); // No margins for the main layout
        m_mainLayout->setSpacing(0);
        m_centralWidget->setObjectName("centralDiv");

        initContentAreaLayout();
    }

    // right sine panel
    {
        m_rightSidePanel = std::make_unique<QWidget>(this);
        m_rightSidePanelLayout = std::make_unique<QVBoxLayout>(m_rightSidePanel.get());
        m_rightSidePanelLayout->setContentsMargins(0, 0, 0, 0); // No margins for the main layout
        m_rightSidePanelLayout->setSpacing(0);
        m_rightSidePanel->setFixedWidth(35);
        m_rightSidePanel->setObjectName("rightPanel");
    }

    // bottom panel
    {
        m_bottomPanelLayout = std::make_unique<QHBoxLayout>(m_bottomPanel.get());
        m_bottomPanelLayout->setContentsMargins(0, 0, 0, 0); // No margins for the main layout
        m_bottomPanelLayout->setSpacing(0);
        m_bottomPanel->setFixedHeight(25);
        m_bottomPanel->setObjectName("bottomPanel");

        // Status bar
        m_statusBar = new QStatusBar(m_bottomPanel.get());
        m_statusBar->setObjectName("appStatusBar");
        m_statusBar->setFixedHeight(25);
        m_bottomPanelLayout->addWidget(m_statusBar);
    }

    // Grid layouts
    {
        const auto middleContentContainer = new QWidget(this);
        QHBoxLayout* mainContentLayout = new QHBoxLayout(middleContentContainer);
        mainContentLayout->setContentsMargins(0, 0, 0, 0); // No margins for a seamless frame
        mainContentLayout->setSpacing(0);

        mainContentLayout->addWidget(m_leftSidePanel.get(), 0);
        mainContentLayout->addWidget(m_centralWidget.get(), 1);
        mainContentLayout->addWidget(m_rightSidePanel.get(), 0);

        // 4. Add the widgets to the grid layout at specific row/column positions.
        // The format is: addWidget(widget, row, column, rowSpan, columnSpan)
        mainGridLayout->addWidget(m_titleBar.get(), 0, 0, 1, 3); // Row 0, Col 0, spans 1 row, 3 columns
        mainGridLayout->addWidget(m_topPanel.get(), 1, 0, 1, 2); // Row 1, Col 1
        mainGridLayout->addWidget(middleContentContainer, 2, 0); // Row 2, Col 0
        mainGridLayout->addWidget(m_bottomPanel.get(), 3, 0); // Row 3, Col 0, spans 1 row, 3 columns

        // Set other rows/columns to have 0 stretch so they don't expand.
        mainGridLayout->setRowStretch(0, 0);
        mainGridLayout->setRowStretch(1, 1);
        mainGridLayout->setColumnStretch(2, 0);
    }

    // Connections
    connect(m_closeButton.get(), &QPushButton::clicked, this, &FramelessWindow::close);
    connect(m_maximizeButton.get(), &QPushButton::clicked, this, &FramelessWindow::showMaximizeOrRestoreSlot);
    connect(m_minimizeButton.get(), &QPushButton::clicked, this, &FramelessWindow::showMinimized);
    connect(this, &FramelessWindow::closeApp, this, &FramelessWindow::close);

    // Create an instance of your settings dialog
    const auto settingsDialog = new SettingsDialog(this);
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
}

void FramelessWindow::initContentAreaLayout()
{
    m_drawer = std::make_unique<CustomDrawer>(m_editor.get());

    const auto contentArea = new QWidget(m_centralWidget.get());
    m_mainLayout->addWidget(contentArea);

    // 1. MAIN LAYOUT: A horizontal layout to separate the left controls from the main content.
    QHBoxLayout* mainLayout = new QHBoxLayout(contentArea);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 2. LEFT CONTROL PANEL: A vertical layout for the top and bottom buttons.

    m_folderButton = std::make_unique<QPushButton>("🗀", contentArea);
    m_folderButton->setIconSize(QSize(32, 32));
    m_folderButton->setFixedSize(35, 35);

    connect(m_folderButton.get(), &QPushButton::clicked, this, &FramelessWindow::updateDrawer);

    m_outputButton = std::make_unique<QPushButton>("❯_", contentArea);
    m_outputButton->setFixedSize(35, 35);

    connect(m_outputButton.get(), &QPushButton::clicked, this, &FramelessWindow::onShowOutputButtonClicked);

    m_leftSidePanelLayout->addWidget(m_folderButton.get()); // Add button to the top
    m_leftSidePanelLayout->addStretch(); // Pushes buttons to top and bottom
    m_leftSidePanelLayout->addWidget(m_outputButton.get()); // Add button to the bottom

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

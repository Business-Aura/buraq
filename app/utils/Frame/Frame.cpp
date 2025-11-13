//
// Created by talik on 9/1/2025.
//

#include "Frame.h"

#include "app_version.h"
#include "CustomLabel.h"
#include "Filters/Toolbar/ToolBarEvent.h"
#include "settings/SettingManager/SettingsManager.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QSize>

#include "ToolBar.h"

Frame::Frame(QWidget* parent, const bool hasToolBar, const QSize minSize)
    : QWidget(parent),
      m_titleBar(std::make_unique<QWidget>(this)),
      m_topPanel(std::make_unique<QWidget>(this)),
      m_rightSidePanel(std::make_unique<QWidget>(this)),
      m_bottomPanel(std::make_unique<QWidget>(this)),
      m_centralWidget(std::make_unique<QWidget>(this)),
      m_dragPosition(QPoint(0, 0))
{
    // 1. Create a main container widget. This will be the single central widget.
    m_frameContainer = new QWidget(this);
    m_frameContainer->setObjectName("Frame");
    // m_frameContainer->resize(parent->size());

    // 2. Create the grid layout that the container will use.
    QGridLayout* mainGridLayout = new QGridLayout(m_frameContainer);
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

        // m_extraButtons
        m_extraButtons = std::make_unique<QWidget>(m_titleBar_);
        m_extraButtonsLayout = std::make_unique<QHBoxLayout>(m_extraButtons.get());
        m_extraButtonsLayout->setContentsMargins(0, 0, 0, 0);
        m_extraButtonsLayout->setSpacing(0);

        m_closeButton = std::make_unique<QPushButton>("✕", m_titleBar_); // X for close
        m_closeButton->setObjectName("closeButton"); // For specific styling
        m_closeButton->setFixedSize(m_titleBar_->height(), m_titleBar_->height());

        // logo and version
        const auto logoAndVersion = new QWidget(m_titleBar_);
        const auto logoAndVersionLayout = new QHBoxLayout(logoAndVersion);
        logoAndVersionLayout->setContentsMargins(0, 0, 0, 0);

        const auto iconButton = new QPushButton(m_titleBar_);
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
        titleBarLayout->addWidget(m_extraButtons.get());
        titleBarLayout->addWidget(m_closeButton.get());

        // Adjust width
        logoAndVersion->setMinimumWidth(m_extraButtons->width());
    }

    // Toolkit
    {
        m_topPanel->setFixedHeight(35); // Set your desired title bar height
        const auto toolKitLayout = new QHBoxLayout(m_topPanel.get());
        toolKitLayout->setContentsMargins(0, 0, 0, 0);
        toolKitLayout->setSpacing(0);
        toolKitLayout->setObjectName("topPanel");
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
    }

    const auto middleContentContainer = new QWidget(this);
    QHBoxLayout* mainContentLayout = new QHBoxLayout(middleContentContainer);
    mainContentLayout->setContentsMargins(0, 0, 0, 0); // No margins for a seamless frame
    mainContentLayout->setSpacing(0);

    mainContentLayout->addWidget(m_leftSidePanel.get(), 0);
    mainContentLayout->addWidget(m_centralWidget.get(), 1);
    mainContentLayout->addWidget(m_rightSidePanel.get(), 0);

    // 4. Add the widgets to the grid layout at specific row/column positions.
    // The format is: addWidget(widget, row, column, rowSpan, columnSpan)
    if (hasToolBar)
    {
        mainGridLayout->addWidget(m_titleBar.get(), 0, 0, 1, 3); // Row 0, Col 0, spans 1 row, 3 columns
        mainGridLayout->addWidget(m_topPanel.get(), 1, 0, 1, 2); // Row 1, Col 1
        mainGridLayout->addWidget(middleContentContainer, 2, 0); // Row 2, Col 0
        mainGridLayout->addWidget(m_bottomPanel.get(), 3, 0); // Row 3, Col 0, spans 1 row, 3 columns
    }
    else
    {
        m_titleBar->setObjectName("secondaryTitleBar");
        mainGridLayout->addWidget(m_titleBar.get(), 0, 0, 1, 0); // Row 0, Col 0, spans 1 row, 3 columns
        mainGridLayout->addWidget(middleContentContainer, 1, 0); // Row 2, Col 0
        mainGridLayout->addWidget(m_bottomPanel.get(), 2, 0); // Row 3, Col 0, spans 1 row, 3 columns
    }

    // Set other rows/columns to have 0 stretch so they don't expand.
    mainGridLayout->setRowStretch(0, 0);
    mainGridLayout->setRowStretch(1, 1);
    mainGridLayout->setColumnStretch(2, 0);

    connect(m_closeButton.get(), &QPushButton::clicked, parent, &QWidget::close);
    connect(m_titlebarEvents.get(), &ToolBarEvent::dragWindow, this, &Frame::windowDrag);
}

Frame::~Frame()
{
    // smart pointers are cleaned up
    delete m_frameContainer;
}

void Frame::windowResizeSlot(const QSize& size) const
{
    qDebug() << "WindowResizeSlot " << size;
    m_frameContainer->setFixedSize(size);
}

void Frame::windowDrag(QMouseEvent* event)
{
    m_dragPosition = event->globalPosition().toPoint();
    move(m_dragPosition);
    event->accept();
}

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
#include <QLabel>

#include "ToolBar.h"

Frame::Frame(QWidget* parent, const bool hasToolBar, const QSize minSize)
    : QWidget(parent),
      m_dragPosition(QPoint(0, 0))
{
    // 1. Create a main container widget. This will be the single central widget.
    m_frameContainer = new QWidget(this);
    m_frameContainer->setObjectName("Frame");

    auto* frameLayout = new QVBoxLayout(this);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->addWidget(m_frameContainer);

    // 2. Create the grid layout that the container will use.
    QGridLayout* mainGridLayout = new QGridLayout(m_frameContainer);
    mainGridLayout->setContentsMargins(0, 0, 0, 0); // No margins for a seamless frame
    mainGridLayout->setSpacing(0); // No spacing between frame parts

    // titleBar
    {
        m_titleBar = new QWidget(m_frameContainer);
        m_titlebarEvents = new ToolBarEvent(m_titleBar);
        m_titleBar->setFixedHeight(35); // Set your desired title bar height
        m_titleBar->setObjectName("customTitleBar"); // For styling

        const auto titleBarLayout = new QHBoxLayout(m_titleBar);
        titleBarLayout->setContentsMargins(10, 0, 0, 0); // Left margin for the icon
        titleBarLayout->setSpacing(0);

        // --- Logo and Title ---
        const auto iconButton = new QPushButton(m_titleBar);
        iconButton->setObjectName("titleBarLogoButton");
        iconButton->setStyleSheet("padding: 0px; border: none; background: transparent;");
        if (parent && !parent->windowIcon().isNull()) {
            iconButton->setIcon(parent->windowIcon());
        } else {
            iconButton->setIcon(QIcon(":/icons/buraq.png"));
        }
        iconButton->setIconSize(QSize(24, 24));
        iconButton->setFixedSize(35, 35); 
        iconButton->setFlat(true);

        m_titleLabel = new QLabel(m_titleBar);
        m_titleLabel->setObjectName("titleText");
        m_titleLabel->setText("Buraq"); // Default title

        const auto version = new QLabel(m_titleBar);
        version->setObjectName("versionText");
        version->setText("v2.0.0");
        version->setStyleSheet("margin-left: 10px; color: #888; font-size: 9pt;");
        if (!hasToolBar) {
            version->hide();
            iconButton->hide();
            titleBarLayout->addWidget(iconButton);
            titleBarLayout->addWidget(m_titleLabel);
            titleBarLayout->addWidget(version);
        }

        // --- End Logo and Title ---

        titleBarLayout->addStretch();
        
        m_extraButtons = new QWidget(m_titleBar);
        m_extraButtonsLayout = new QHBoxLayout(m_extraButtons);
        m_extraButtonsLayout->setContentsMargins(0, 0, 0, 0);
        m_extraButtonsLayout->setSpacing(0);
        titleBarLayout->addWidget(m_extraButtons);

        m_closeButton = new QPushButton("✕", m_titleBar); // X for close
        m_closeButton->setObjectName("closeButton"); // For specific styling
        m_closeButton->setFixedSize(m_titleBar->height(), m_titleBar->height());
        titleBarLayout->addWidget(m_closeButton);
    }

    // Toolkit (topPanel)
    {
        m_topPanel = new QWidget(m_frameContainer);
        m_topPanel->setFixedHeight(35); // Set your desired title bar height
        const auto toolKitLayout = new QHBoxLayout(m_topPanel);
        toolKitLayout->setContentsMargins(0, 0, 0, 0);
        toolKitLayout->setSpacing(0);
        toolKitLayout->setObjectName("topPanel");
    }

    // left side panel
    {
        m_leftSidePanel = new QWidget(m_frameContainer);
        m_leftSidePanelLayout = new QVBoxLayout(m_leftSidePanel);
        m_leftSidePanelLayout->setContentsMargins(0, 0, 0, 0); // No margins for the main layout
        m_leftSidePanelLayout->setSpacing(0);

        m_leftSidePanel->setFixedWidth(35);
        m_leftSidePanel->setObjectName("leftPanel");
    }

    // Central area
    {
        m_centralWidget = new QWidget(m_frameContainer);
        m_mainLayout = new QVBoxLayout(m_centralWidget); // Apply layout to the central widget
        m_mainLayout->setContentsMargins(0, 0, 0, 0); // No margins for the main layout
        m_mainLayout->setSpacing(0);
        m_centralWidget->setObjectName("centralDiv");
    }

    // right side panel
    {
        m_rightSidePanel = new QWidget(m_frameContainer);
        m_rightSidePanelLayout = new QVBoxLayout(m_rightSidePanel);
        m_rightSidePanelLayout->setContentsMargins(0, 0, 0, 0); // No margins for the main layout
        m_rightSidePanelLayout->setSpacing(0);
        m_rightSidePanel->setFixedWidth(35);
        m_rightSidePanel->setObjectName("rightPanel");
    }

    // bottom panel
    {
        m_bottomPanel = new QWidget(m_frameContainer);
        m_bottomPanelLayout = new QHBoxLayout(m_bottomPanel);
        m_bottomPanelLayout->setContentsMargins(0, 0, 0, 0); // No margins for the main layout
        m_bottomPanelLayout->setSpacing(0);
        m_bottomPanel->setFixedHeight(25);
        m_bottomPanel->setObjectName("bottomPanel");
    }

    const auto middleContentContainer = new QWidget(m_frameContainer);
    QHBoxLayout* midContentLayout = new QHBoxLayout(middleContentContainer);
    midContentLayout->setContentsMargins(0, 0, 0, 0); // No margins for a seamless frame
    midContentLayout->setSpacing(0);

    midContentLayout->addWidget(m_leftSidePanel, 0);
    midContentLayout->addWidget(m_centralWidget, 1);
    midContentLayout->addWidget(m_rightSidePanel, 0);

    // 4. Add the widgets to the grid layout at specific row/column positions.
    // The format is: addWidget(widget, row, column, rowSpan, columnSpan)
    if (hasToolBar)
    {
        mainGridLayout->addWidget(m_titleBar, 0, 0, 1, 3); // Row 0, Col 0, spans 1 row, 3 columns
        mainGridLayout->addWidget(m_topPanel, 1, 0, 1, 2); // Row 1, Col 1
        mainGridLayout->addWidget(middleContentContainer, 2, 0); // Row 2, Col 0
        mainGridLayout->addWidget(m_bottomPanel, 3, 0); // Row 3, Col 0, spans 1 row, 3 columns
    }
    else
    {
        m_titleBar->setObjectName("secondaryTitleBar");
        mainGridLayout->addWidget(m_titleBar, 0, 0, 1, 1); // Row 0, Col 0
        mainGridLayout->addWidget(middleContentContainer, 1, 0, 1, 1); // Row 1, Col 0
        mainGridLayout->addWidget(m_bottomPanel, 2, 0, 1, 1); // Row 2, Col 0
    }

    // Set other rows/columns to have 0 stretch so they don't expand.
    mainGridLayout->setRowStretch(0, 0);
    mainGridLayout->setRowStretch(1, 1);
    mainGridLayout->setColumnStretch(2, 0);

    if (parent) {
        connect(m_closeButton, &QPushButton::clicked, parent, &QWidget::close);
    }
    connect(m_titlebarEvents, &ToolBarEvent::dragWindow, this, &Frame::windowDrag);
}

Frame::~Frame()
{
    // Qt handles cleanup of children
}

void Frame::windowResizeSlot(const QSize& size) const
{
    qDebug() << "WindowResizeSlot " << size;
    m_frameContainer->setFixedSize(size);
}

void Frame::windowDrag(QMouseEvent* event)
{
    m_dragPosition = event->globalPosition().toPoint();
    if (const auto parentWidget = dynamic_cast<QWidget*>(parent()))
    {
        parentWidget->move(m_dragPosition);
    }
    event->accept();
}

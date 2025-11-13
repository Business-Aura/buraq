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
#include <QToolBar>

#include "ToolBar.h"

Frame::Frame(QWidget* parent,  bool hasToolBar, const QSize minSize)
    : QWidget(parent),
      m_titleBar(std::make_unique<QWidget>(this)),
      m_topPanel(std::make_unique<QWidget>(this)),
      m_rightSidePanel(std::make_unique<QWidget>(this)),
      m_bottomPanel(std::make_unique<QWidget>(this)),
      m_centralWidget(std::make_unique<QWidget>(this)),
      m_dragPosition(QPoint(0, 0))
{
    // 1. Create a main container widget. This will be the single central widget.
    QWidget* frameContainer = new QWidget(this);
    frameContainer->setObjectName("Frame");
    frameContainer->setMinimumSize(minSize);

    // 2. Create the grid layout that the container will use.
    QGridLayout* mainGridLayout = new QGridLayout(frameContainer);
    mainGridLayout->setContentsMargins(0, 0, 0, 0); // No margins for a seamless frame
    mainGridLayout->setSpacing(0); // No spacing between frame parts

    // titleBar
    {
        const auto m_titleBar_ = m_titleBar.get();
        m_titlebarEvents = new ToolBarEvent(m_titleBar_);
        m_titleBar_->installEventFilter(m_titlebarEvents);
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
    mainGridLayout->addWidget(m_titleBar.get(), 0, 0, 1, 3); // Row 0, Col 0, spans 1 row, 3 columns
    mainGridLayout->addWidget(m_topPanel.get(), 1, 0, 1, 2); // Row 1, Col 1
    mainGridLayout->addWidget(middleContentContainer, 2, 0); // Row 2, Col 0
    mainGridLayout->addWidget(m_bottomPanel.get(), 3, 0); // Row 3, Col 0, spans 1 row, 3 columns

    // Set other rows/columns to have 0 stretch so they don't expand.
    mainGridLayout->setRowStretch(0, 0);
    mainGridLayout->setRowStretch(1, 1);
    mainGridLayout->setColumnStretch(2, 0);

    connect(m_closeButton.get(), &QPushButton::clicked, parent, &QWidget::close);
    connect(m_titlebarEvents, &ToolBarEvent::dragWindow, this, &Frame::windowDrag);
}

Frame::~Frame()
{
    // smart pointers are cleaned up
}

void Frame::windowDrag(QMouseEvent* event)
{
    m_dragPosition = event->globalPosition().toPoint();
    move(m_dragPosition);
    event->accept();
}

Qt::Edges Frame::calculateEdges(const QPoint& pos, const int margin) const
{
    Qt::Edges edges;
    if (pos.x() < margin) edges |= Qt::LeftEdge;
    if (pos.x() > width() - margin) edges |= Qt::RightEdge;
    if (pos.y() < margin) edges |= Qt::TopEdge;
    if (pos.y() > height() - margin) edges |= Qt::BottomEdge;
    return edges;
}

void Frame::updateCursorShape(const QPoint& pos)
{
    if (m_resizing || parent == nullptr)
        return;

    m_resizeEdges = calculateEdges(pos, m_resizeMargin);

    if (m_resizeEdges == (Qt::TopEdge | Qt::LeftEdge) || m_resizeEdges == (Qt::BottomEdge | Qt::RightEdge))
        parent->setCursor(Qt::SizeFDiagCursor);
    else if (m_resizeEdges == (Qt::TopEdge | Qt::RightEdge) || m_resizeEdges == (Qt::BottomEdge | Qt::LeftEdge))
        parent->setCursor(Qt::SizeBDiagCursor);
    else if (m_resizeEdges & (Qt::LeftEdge | Qt::RightEdge))
        parent->setCursor(Qt::SizeHorCursor);
    else if (m_resizeEdges & (Qt::TopEdge | Qt::BottomEdge))
        parent->setCursor(Qt::SizeVerCursor);
    else
        parent->setCursor(Qt::ArrowCursor);
}

void Frame::mousePressEvent(QMouseEvent* event)
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

void Frame::mouseMoveEvent(QMouseEvent* event)
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

void Frame::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = false;
        m_resizing = false;
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mouseReleaseEvent(event);
}

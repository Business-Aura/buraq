// TerminalPanel.cpp — Bottom panel implementation
#include "TerminalPanel.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTabBar>
#include <QDir>
#include <QDebug>
#include <QTimer>

#include "output_display/OutputDisplay.h"
#include "TerminalWidget.h"

TerminalPanel::TerminalPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ── Tab widget ──
    m_tabs = new QTabWidget(this);
    m_tabs->setObjectName("BottomPanel");
    m_tabs->setTabsClosable(false);   // We handle close buttons manually
    m_tabs->setMovable(true);
    m_tabs->setDocumentMode(true);    // Sleeker VSCode-style look

    // ── "New Terminal" (+) corner button ──
    auto* cornerContainer = new QWidget(this);
    auto* cornerLayout = new QHBoxLayout(cornerContainer);
    cornerLayout->setContentsMargins(0, 0, 10, 0); // Push button 10px off the right edge
    cornerLayout->setSpacing(0);

    auto* addBtn = new QPushButton("+", this);
    addBtn->setObjectName("NewTerminalButton");
    addBtn->setFixedSize(28, 28);
    addBtn->setToolTip("New Terminal");
    connect(addBtn, &QPushButton::clicked, this, &TerminalPanel::addTerminalTab);

    cornerLayout->addWidget(addBtn);
    cornerContainer->setLayout(cornerLayout);

    m_tabs->setCornerWidget(cornerContainer, Qt::TopRightCorner);

    // ── Output tab (always index 0, no close button) ──
    m_outputDisplay = new OutputDisplay(this);
    m_outputTabIndex = m_tabs->addTab(m_outputDisplay, "Output");

    // ── Close-tab signal ──
    connect(m_tabs, &QTabWidget::tabCloseRequested,
            this, &TerminalPanel::closeTab);

    layout->addWidget(m_tabs);

    // Start hidden — user opens via button or shortcut
    hide();
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void TerminalPanel::log(const QString& output, const QString& error) const
{
    if (m_outputDisplay)
        m_outputDisplay->log(output, error);
}

void TerminalPanel::toggle()
{
    if (isVisible())
    {
        hide();
    }
    else
    {
        show();
        if (m_tabs && m_tabs->count() == 1)
        {
            addTerminalTab();
        }
    }
}

void TerminalPanel::setProjectDirectory(const QString& dir)
{
    m_projectDir = dir;
}

void TerminalPanel::setShell(const QString& shellPath, const QString& shellArgs)
{
    m_shellPath = shellPath;
    m_shellArgs = shellArgs;
}

void TerminalPanel::showOutputTab()
{
    show();
    m_tabs->setCurrentIndex(m_outputTabIndex);
}

// ─────────────────────────────────────────────────────────────────────────────
// Terminal tab management
// ─────────────────────────────────────────────────────────────────────────────

TerminalWidget* TerminalPanel::createTerminalWidget()
{
    const QString workDir = m_projectDir.isEmpty()
        ? QDir::homePath()
        : m_projectDir;

    auto* tw = new TerminalWidget(m_shellPath, m_shellArgs, workDir, this);
    connect(tw, &TerminalWidget::titleChanged,
            this, &TerminalPanel::onTerminalTitleChanged);
    return tw;
}

void TerminalPanel::addTerminalTab()
{
    ++m_terminalCount;
    TerminalWidget* tw = createTerminalWidget();

    const QString label = QStringLiteral("Terminal %1").arg(m_terminalCount);
    int idx = m_tabs->addTab(tw, label);
    m_tabs->setCurrentIndex(idx);

    // Add a close button only on terminal tabs (not the Output tab)
    // We achieve this by setting tabsClosable only after adding
    m_tabs->tabBar()->setTabButton(idx, QTabBar::RightSide, makeCloseButton(idx));

    show();
}

// Dynamically create a per-tab close button
QPushButton* TerminalPanel::makeCloseButton(int tabIndex)
{
    // We capture tabIndex by value for closure, but the index can shift after
    // deletions — so we identify the tab by widget pointer at click time.
    auto* btn = new QPushButton("✕", this);
    btn->setObjectName("TerminalCloseButton");
    btn->setFixedSize(16, 16);
    btn->setFlat(true);

    QTabWidget* tabs = m_tabs; // capture for lambda
    QWidget* tabWidget = m_tabs->widget(tabIndex);
    connect(btn, &QPushButton::clicked, this, [this, tabs, tabWidget]() {
        // Defer deletion via QTimer to avoid deleting the sender button
        // in the middle of its own event handler (which causes a crash).
        QTimer::singleShot(0, this, [this, tabs, tabWidget]() {
            int idx = tabs->indexOf(tabWidget);
            if (idx > 0) closeTab(idx); // Never close index 0 (Output)
        });
    });

    return btn;
}

void TerminalPanel::closeTab(int index)
{
    if (index == m_outputTabIndex) return; // Output tab is permanent

    QWidget* w = m_tabs->widget(index);
    m_tabs->removeTab(index);
    if (w) w->deleteLater();
}

void TerminalPanel::onTerminalTitleChanged(const QString& title)
{
    // Find which tab sent this signal
    if (auto* tw = qobject_cast<TerminalWidget*>(sender()))
    {
        int idx = m_tabs->indexOf(tw);
        if (idx >= 0)
            m_tabs->setTabText(idx, title);
    }
}

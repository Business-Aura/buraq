// toolbar.cpp
#include <QMessageBox>
#include <QByteArray>
#include <QBuffer>
#include <QPixmap>
#include <QMenu>
#include <QPushButton> // Required for QPushButton
#include "toolbar.h"
#include "frameless_window/FramelessWindow.h"

// Constructor with title
ToolBar::ToolBar(const QString& title, QWidget* parent)
    : QToolBar(title, parent), m_window(parent)
{
    m_customAction = nullptr;
    m_fileMenu = nullptr;

    // --- CHANGE THESE LINES ---
    setMovable(false); // Disable dragging
    setFloatable(false); // Disable detaching and floating
    // --- END CHANGES ---

    setContextMenuPolicy(Qt::PreventContextMenu);
    setIconSize(QSize(24, 24));
    setObjectName("test");


    qDebug() << "Custom ToolBar created with title:" << title;
}

ToolBar::ToolBar(QWidget* parent)
    : ToolBar(QString(), parent)
{
}

// ... (rest of the code remains the same) ...

void ToolBar::addCustomAction(const QString& text, const QIcon& icon)
{
    // Use the provided icon, or a default empty icon if none is given
    QAction* action = new QAction(icon.isNull() ? QIcon() : icon, text, this);
    addAction(action);
    connect(action, &QAction::triggered, this, &ToolBar::onCustomActionTriggered);
    qDebug() << "Custom action added to toolbar: " << text;
}

void ToolBar::onCustomActionTriggered()
{
    qDebug() << "Custom action was triggered!";
    emit customActionTriggered();
}

void ToolBar::addFileMenu()
{
    if (m_fileMenu)
    {
        qDebug() << "File menu already exists, not adding again.";
        return;
    }

    m_fileMenu = new QMenu(this); // The QMenu itself still exists
    m_fileMenu->setObjectName("fileMenu");

    QAction* newAction = new QAction("New", this);
    QAction* openAction = new QAction("Open", this);
    QAction* saveAction = new QAction("Save", this);
    QAction* exitAction = new QAction("Exit", this);

    m_fileMenu->addAction(newAction);
    m_fileMenu->addAction(openAction);
    m_fileMenu->addAction(saveAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(exitAction);

    // --- START CHANGES FOR QPushButton ---
    // Create a QPushButton to act as the menu trigger
    m_fileMenuButton = new QPushButton("☰", this);
    m_fileMenuButton->setObjectName("fileMenuButton");
    m_fileMenuButton->setFixedSize(40, height());

    // Add the button to the toolbar. By default, it will be on the left if added first.
    addWidget(m_fileMenuButton);

    // Connect the button's clicked signal to a slot that shows the menu
    connect(m_fileMenuButton, &QPushButton::clicked, this, &ToolBar::onFileMenuButtonClicked);
    // --- END CHANGES FOR QPushButton ---

    connect(newAction, &QAction::triggered, this, &ToolBar::onNewFile);
    connect(openAction, &QAction::triggered, this, &ToolBar::onOpenFile);
    connect(saveAction, &QAction::triggered, this, &ToolBar::onSaveFile);
    if (const auto window = dynamic_cast<FramelessWindow*>(m_window); window != nullptr) // FIX ME
    {
        connect(exitAction, &QAction::triggered, window, &FramelessWindow::closeWindowSlot);
    }

    qDebug() << "File menu successfully added to toolbar.";
}

void ToolBar::onNewFile()
{
    qDebug() << "New File action triggered!";
    QMessageBox::information(this, "File Action", "New File selected!");
    emit newFileTriggered();
}

void ToolBar::onOpenFile()
{
    qDebug() << "Open File action triggered!";
    QMessageBox::information(this, "File Action", "Open File selected!");
    emit openFileTriggered();
}

void ToolBar::onSaveFile()
{
    qDebug() << "Save File action triggered!";
    QMessageBox::information(this, "File Action", "Save File selected!");
    emit saveFileTriggered();
}

void ToolBar::onExit()
{
    qDebug() << "Exit action triggered!";
    QMessageBox::information(this, "File Action", "Exit selected! (Application would typically quit)");
    emit exitTriggered();
}

// --- NEW SLOT IMPLEMENTATION FOR QPushButton ---
void ToolBar::onFileMenuButtonClicked()
{
    // Get the sender of the signal (which is the QPushButton in this case)
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button)
    {
        // Map the button's bottom-left corner to global screen coordinates
        // This makes the menu appear directly below the button.
        const QPoint pos = button->mapToGlobal(QPoint(0, button->height()));
        m_fileMenu->popup(pos); // Show the menu at the calculated position
    }
}

// --- END NEW SLOT IMPLEMENTATION ---

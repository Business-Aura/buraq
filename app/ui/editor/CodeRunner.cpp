// CodeRunner.cpp — Play/Run button implementation
#include "CodeRunner.h"

#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include "editor/Editor.h"
#include "frameless_window/MainWindow.h"
#include "terminal/TerminalPanel.h"
#include "../../database/db_conn.h"
#include "settings/UserSettings.h"
#include "extensions/ExtensionManager.h"

CodeRunner::CodeRunner(QWidget* parent)
    : QPushButton(parent), m_window(parent)
{
    setObjectName("CodeRunner");
    setIcon(QIcon(":/icons/ui/play.svg"));
    setIconSize(QSize(16, 16));
    setToolTip("Run Code (Selection / Script)");
    setupSignals();
}

CodeRunner::~CodeRunner()
{
    m_window = nullptr;
}

void CodeRunner::setupSignals()
{
    connect(this, &QPushButton::clicked, this, &CodeRunner::runCode);

    const auto window = dynamic_cast<MainWindow*>(m_window);
    if (window)
    {
        connect(this, &CodeRunner::statusUpdate, window, &MainWindow::processStatusSlot);
        connect(this, &CodeRunner::updateOutputResult, window, &MainWindow::processResultSlot);
    }
}

void CodeRunner::runCode()
{
    const auto window_ = dynamic_cast<MainWindow*>(m_window);
    if (window_ == nullptr || !window_->getEditor())
    {
        return;
    }

    // 1. Save the file first (automatically triggers Save Dialog if file has no path yet)
    window_->getEditor()->saveFile();

    QString filePath = window_->getEditor()->currentFile();
    if (filePath.isEmpty())
    {
        return; // User cancelled saving
    }

    QFileInfo fileInfo(filePath);
    QString ext = fileInfo.suffix().toLower();
    QString dir = QDir::toNativeSeparators(fileInfo.absolutePath());
    QString fileName = fileInfo.fileName();
    QString baseName = fileInfo.baseName();

    // Get the terminal panel
    TerminalPanel* termPanel = window_->terminalPanel();
    if (!termPanel)
    {
        return;
    }

    // Retrieve active shell configuration
    const UserSettings& prefs = window_->getUserPreferences();
    QString shell = prefs.shellPath.toLower();

    QString cmd;

    // Check if the user has highlighted selection to run (e.g. selection execution)
    QString selected = window_->getEditor()->selectedText();
    if (!selected.isEmpty())
    {
        cmd = selected.replace("\u2029", "\n");
    }
    else
    {
        // Query ExtensionManager for runner command
        cmd = ExtensionManager::instance().getRunCommand(filePath, shell);

        if (cmd.isEmpty())
        {
            const QString action = ExtensionManager::instance().getActionForFile(filePath);
            if (action == "build_menu")
            {
                emit statusUpdate("Selected file is not a script. Use Build menu to compile C/C++.");
                return;
            }

            emit statusUpdate("No extension runner installed for file type '." + ext + "'.");
            return;
        }
    }

    if (!cmd.isEmpty())
    {
        emit statusUpdate("Running code in terminal...");
        termPanel->executeCommand(cmd);
    }
}

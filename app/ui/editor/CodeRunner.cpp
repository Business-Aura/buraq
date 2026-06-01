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

CodeRunner::CodeRunner(QWidget* parent)
    : QPushButton("{ }", parent), m_window(parent)
{
    setObjectName("CodeRunner");
    setToolTip("Run Code (C++ / PowerShell / Selection)");
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
    else if (ext == "cpp" || ext == "cxx" || ext == "cc" || ext == "c")
    {
        emit statusUpdate("Selected file is not a script. Use Build menu to compile C/C++.");
        return;
    }
    else if (ext == "ps1")
    {
        // PowerShell script execution
        if (shell.contains("powershell") || shell.contains("pwsh"))
        {
            cmd = QString("& \"%1\"").arg(QDir::toNativeSeparators(filePath));
        }
        else
        {
            cmd = QString("pwsh -File \"%1\"").arg(QDir::toNativeSeparators(filePath));
        }
    }
    else
    {
        emit statusUpdate("File type not supported for script execution.");
        return;
    }

    if (!cmd.isEmpty())
    {
        emit statusUpdate("Running code in terminal...");
        termPanel->executeCommand(cmd);
    }
}

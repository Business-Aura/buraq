//
// Created by talik on 5/28/2025.
//

#include <QIcon>
#include "CodeRunner.h"
#include <QThread>
#include "CustomLabel.h"
#include "Editor.h"
#include "IconButton.h"
#include "app_ui/AppUi.h"
#include "frameless_window/FramelessWindow.h"

CodeRunner::CodeRunner(QWidget* parent)
    : QPushButton("{ }", parent), m_window(parent), m_workerThread(nullptr),
      m_minion(nullptr)
{
    setObjectName("CodeRunner");

    // set tooltip for the run buttons
    setToolTip(
        "Run code."
        " & "
        "Highlighted code.");

    setupSignals();
}

// Call this once in your CodeRunner's constructor.
void CodeRunner::setupWorker()
{
    // --- 1. Create and Connect Objects ---
    m_psClient = new PSClient(this);
    m_workerThread = new QThread(this);
    m_minion = new Minion();
    m_minion->moveToThread(m_workerThread);

    // --- 2. Connect Signals and Slots ---

    // Minion (worker thread) asks psClient (main thread) to execute a script.
    connect(m_minion, &Minion::runScriptRequested, m_psClient, &PSClient::runScript);

    // psClient (main thread) sends result back to CodeRunner (main thread).
    connect(m_psClient, &PSClient::scriptResultReceived, this, &CodeRunner::handleTaskResults);

    // Clean up the thread and worker when the thread's event loop finishes.
    connect(m_workerThread, &QThread::finished, m_minion, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished, m_workerThread, &QObject::deleteLater);

    m_workerThread->start();
}


// This function is now much simpler. It just gets the script and signals the worker.
void CodeRunner::runCode()
{
    if (!m_workerThread || !m_workerThread->isRunning())
    {
        // If the thread isn't running, set it up.
        setupWorker();
    }

    // --- Get the script text from the UI in the main thread ---
    const auto window_ = dynamic_cast<FramelessWindow*>(m_window);
    if (window_ == nullptr || !window_->getEditor())
    {
        return; // Safety check
    }

    QString script = window_->getEditor()->selectedText();
    if (script.isEmpty())
    {
        script = window_->getEditor()->toPlainText();
    }

    if (script.isEmpty())
    {
        return; // Nothing to run
    }

    const auto cleanedScript = script.replace("\u2029", "\n");

    emit statusUpdate("Running code..");

    // --- Safely trigger the task on the worker thread via a signal ---
    // The Minion's process slot should be connected to this signal.
    // We assume Minion has a signal like `startProcessing(QString)`.
    QMetaObject::invokeMethod(m_minion, "processScript", Qt::QueuedConnection, Q_ARG(QString, cleanedScript));
}

void CodeRunner::handleTaskResults(const QVariant& result)
{
    // if (result.isValid() && result.canConvert<QString>())
    if (const auto flag = result.canConvert<QString>(); flag && result.isValid())
    {
        const auto resultString = result.value<QString>();
        QString error = "";
        int statusCode = 0;
        if (!resultString.isEmpty() && resultString.contains("exception", Qt::CaseInsensitive))
        {
            error = resultString;
            // resultString.clear();
            statusCode = 1;
        }
        emit updateOutputResult(statusCode, resultString, error);
    }
    else
    {
        emit updateOutputResult(1, "", "Error failed to execute task.");
    }
}

void CodeRunner::handleProgress(int i)
{
    emit statusUpdate("Executing...");
}

void CodeRunner::handleWorkerFinished()
{
    emit statusUpdate("Ready..");

    m_workerThread = nullptr;
    m_minion = nullptr;
}

CodeRunner::~CodeRunner()
{
    // smart pointers are deleted automatically
    // editor pointer should be deleted elsewhere
    m_window = nullptr;

    if (m_workerThread && m_workerThread->isRunning())
    {
        m_workerThread->requestInterruption();
        m_workerThread->quit(); // Ask event loop to quit
        if (!m_workerThread->wait(5000))
        {
            // Wait for max 5 seconds
            m_workerThread->terminate(); // Force terminate (last resort)
            m_workerThread->wait(); // Wait for termination
        }
    }
}

void CodeRunner::setupSignals()
{
    // Signal to execute the code
    connect(this, &IconButton::clicked, this, &CodeRunner::runCode);

    const auto window = dynamic_cast<FramelessWindow*>(m_window);
    // Signal to update status bar in AppUI component for the running process
    connect(this, &CodeRunner::statusUpdate, window, &FramelessWindow::processStatusSlot);

    // Signal to update the out component in AppUI component for the completed process
    connect(this, &CodeRunner::updateOutputResult, window, &FramelessWindow::processResultSlot);
    connect(this, &CodeRunner::statusUpdate, m_psClient, &PSClient::connected);
}

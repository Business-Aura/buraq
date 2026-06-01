// TerminalWidget.cpp — Interactive terminal tab implementation
#include "TerminalWidget.h"

#include <QTextEdit>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QScrollBar>
#include <QFont>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QApplication>
#include <QClipboard>
#include <QDebug>

#include "TerminalSession.h"

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

TerminalWidget::TerminalWidget(
    const QString& shellPath,
    const QString& shellArgs,
    const QString& workingDirectory,
    QWidget*       parent
)
    : QWidget(parent)
{
    // ── Layout ──
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ── Display widget ──
    m_display = new QTextEdit(this);
    m_display->setObjectName("TerminalDisplay");
    m_display->setReadOnly(false);          // We manage read-only-ness manually
    m_display->setUndoRedoEnabled(false);
    m_display->setAcceptRichText(false);
    m_display->setLineWrapMode(QTextEdit::WidgetWidth);
    m_display->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_display->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Monospace font — essential for terminal feel
    QFont font("Cascadia Code");
    if (!font.exactMatch()) font.setFamily("Consolas");
    if (!font.exactMatch()) font.setFamily("Courier New");
    font.setPointSize(10);
    font.setFixedPitch(true);
    m_display->setFont(font);

    // Install event filter to intercept key presses before QTextEdit handles them
    m_display->installEventFilter(this);

    layout->addWidget(m_display);

    // ── Shell session ──
    m_session = new TerminalSession(shellPath, shellArgs, workingDirectory, this);
    connect(m_session, &TerminalSession::dataReceived,
            this, &TerminalWidget::onDataReceived);
    connect(m_session, &TerminalSession::sessionEnded,
            this, &TerminalWidget::onSessionEnded);
}

TerminalWidget::~TerminalWidget()
{
    if (m_session)
    {
        disconnect(m_session, nullptr, this, nullptr);
        m_session->terminate();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void TerminalWidget::setWorkingDirectory(const QString& path)
{
    if (m_session && m_session->isRunning())
    {
        // PowerShell and cmd both understand Set-Location / cd
        const QString shellName = m_session->shellPath().toLower();
        QString cmd;
        if (shellName.contains("powershell") || shellName.contains("pwsh"))
            cmd = QStringLiteral("Set-Location -Path \"%1\"").arg(path);
        else
            cmd = QStringLiteral("cd /d \"%1\"").arg(path);

        m_session->sendInput(cmd);
    }
}

bool TerminalWidget::sessionActive() const
{
    return m_session && m_session->isRunning();
}

// ─────────────────────────────────────────────────────────────────────────────
// ANSI / VT100 rendering
// ─────────────────────────────────────────────────────────────────────────────

void TerminalWidget::appendAnsiText(const QString& raw)
{
    QList<AnsiSegment> segments = m_parser.parse(raw);

    QTextCursor cursor(m_display->document());
    cursor.movePosition(QTextCursor::End);

    for (const AnsiSegment& seg : segments)
        appendSegment(cursor, seg);

    // After appending output, set a new input anchor
    m_inputAnchor = m_display->document()->characterCount() - 1;
    m_inputBuffer.clear();

    // Auto-scroll to bottom
    m_display->verticalScrollBar()->setValue(
        m_display->verticalScrollBar()->maximum()
    );
}

void TerminalWidget::appendSegment(QTextCursor& cursor, const AnsiSegment& seg)
{
    QTextCharFormat fmt;

    if (seg.foreground.isValid())
        fmt.setForeground(seg.foreground);
    else
        fmt.clearForeground();

    if (seg.background.isValid())
        fmt.setBackground(seg.background);
    else
        fmt.clearBackground();

    if (seg.bold)
        fmt.setFontWeight(QFont::Bold);
    else
        fmt.setFontWeight(QFont::Normal);

    fmt.setFontItalic(seg.italic);
    fmt.setFontUnderline(seg.underline);

    if (seg.dim)
        fmt.setFontStretch(90); // slight visual hint for dim

    cursor.insertText(seg.text, fmt);
}

// ─────────────────────────────────────────────────────────────────────────────
// Session callbacks
// ─────────────────────────────────────────────────────────────────────────────

void TerminalWidget::onDataReceived(const QString& data)
{
    appendAnsiText(data);
}

void TerminalWidget::onSessionEnded(int exitCode)
{
    m_inputActive = false;
    QTextCursor cursor(m_display->document());
    cursor.movePosition(QTextCursor::End);

    QTextCharFormat fmt;
    fmt.setForeground(QColor("#FF6B6B"));
    cursor.insertText(
        QStringLiteral("\n[Process exited with code %1]\n").arg(exitCode),
        fmt
    );

    emit titleChanged(QStringLiteral("Terminal (exited)"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Keyboard input handling
// ─────────────────────────────────────────────────────────────────────────────

bool TerminalWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_display && event->type() == QEvent::KeyPress)
    {
        handleKeyPress(static_cast<QKeyEvent*>(event));
        return true; // We always consume the key — we control everything
    }
    return QWidget::eventFilter(watched, event);
}

void TerminalWidget::handleKeyPress(QKeyEvent* event)
{
    if (!m_inputActive) return;

    // ── Copy / Paste ──
    if (event->matches(QKeySequence::Copy))
    {
        m_display->copy();
        return;
    }
    if (event->matches(QKeySequence::Paste))
    {
        const QString clip = QApplication::clipboard()->text();
        m_inputBuffer += clip;
        QTextCursor cursor = m_display->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(clip);
        m_display->setTextCursor(cursor);
        return;
    }

    // ── Enter — submit command ──
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        submitCommand();
        return;
    }

    // ── Backspace ──
    if (event->key() == Qt::Key_Backspace)
    {
        if (!m_inputBuffer.isEmpty())
        {
            m_inputBuffer.chop(1);
            QTextCursor cursor = m_display->textCursor();
            cursor.movePosition(QTextCursor::End);
            const int docLen = m_display->document()->characterCount() - 1;
            if (docLen > m_inputAnchor)
            {
                cursor.deletePreviousChar();
                m_display->setTextCursor(cursor);
            }
        }
        return;
    }

    // ── Ctrl+C — send interrupt ──
    if (event->key() == Qt::Key_C && (event->modifiers() & Qt::ControlModifier))
    {
        if (m_session && m_session->isRunning())
        {
            // On Windows, sending Ctrl+C via stdin doesn't always work for
            // piped processes. We clear the input buffer and show ^C visually.
            m_inputBuffer.clear();
            QTextCursor cursor(m_display->document());
            cursor.movePosition(QTextCursor::End);
            QTextCharFormat fmt;
            fmt.setForeground(QColor("#FF6B6B"));
            cursor.insertText(QStringLiteral("^C\n"), fmt);
            m_inputAnchor = m_display->document()->characterCount() - 1;
            m_display->setTextCursor(cursor);
        }
        return;
    }

    // ── Printable characters ──
    const QString text = event->text();
    if (!text.isEmpty() && text[0].isPrint())
    {
        m_inputBuffer += text;
        QTextCursor cursor = m_display->textCursor();
        cursor.movePosition(QTextCursor::End);

        // Render typed char in a distinct colour so it's visible over the prompt
        QTextCharFormat inputFmt;
        inputFmt.setForeground(QColor("#E0E0E0"));
        cursor.insertText(text, inputFmt);
        m_display->setTextCursor(cursor);
    }
}

void TerminalWidget::submitCommand()
{
    // Insert a newline visually
    QTextCursor cursor(m_display->document());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(QStringLiteral("\n"));
    m_display->setTextCursor(cursor);
    m_display->verticalScrollBar()->setValue(
        m_display->verticalScrollBar()->maximum()
    );

    // Send to shell
    if (m_session && m_session->isRunning())
        m_session->sendInput(m_inputBuffer);

    // Reset
    m_inputBuffer.clear();
    m_inputAnchor = m_display->document()->characterCount() - 1;
}

void TerminalWidget::sendCommand(const QString& command)
{
    if (m_session && m_session->isRunning())
    {
        QTextCursor cursor(m_display->document());
        cursor.movePosition(QTextCursor::End);

        QTextCharFormat fmt;
        fmt.setForeground(QColor("#E0E0E0"));
        cursor.insertText(command + "\n", fmt);

        m_display->setTextCursor(cursor);
        m_display->verticalScrollBar()->setValue(
            m_display->verticalScrollBar()->maximum()
        );

        m_inputBuffer.clear();
        m_inputAnchor = m_display->document()->characterCount() - 1;

        m_session->sendInput(command);
    }
}

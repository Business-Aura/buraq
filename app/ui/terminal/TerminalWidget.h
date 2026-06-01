// TerminalWidget.h — Interactive terminal tab with VT100 rendering
#pragma once

#include <QWidget>
#include <QTextCursor>
#include "AnsiParser.h"

class QTextEdit;
class QVBoxLayout;
class TerminalSession;

class TerminalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalWidget(
        const QString& shellPath,
        const QString& shellArgs,
        const QString& workingDirectory,
        QWidget*       parent = nullptr
    );
    ~TerminalWidget() override;

    // Change working directory after construction (sends cd command to shell)
    void setWorkingDirectory(const QString& path);

    // Run command directly in the active shell
    void sendCommand(const QString& command);

    [[nodiscard]] bool sessionActive() const;

signals:
    void titleChanged(const QString& title);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onDataReceived(const QString& data);
    void onSessionEnded(int exitCode);

private:
    void appendAnsiText(const QString& raw);
    void appendSegment(QTextCursor& cursor, const AnsiSegment& seg);
    void handleKeyPress(QKeyEvent* event);
    void submitCommand();

    QTextEdit*       m_display   = nullptr;
    TerminalSession* m_session   = nullptr;
    AnsiParser       m_parser;

    // The position in the document at which user input starts (after prompt)
    int  m_inputAnchor = 0;
    bool m_inputActive = true;

    // Accumulated user keystroke buffer (shown in display, sent on Enter)
    QString m_inputBuffer;
};

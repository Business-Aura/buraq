// TerminalSession.h — QProcess-based interactive shell session
#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

class TerminalSession : public QObject
{
    Q_OBJECT

public:
    explicit TerminalSession(
        const QString& shellPath,
        const QString& shellArgs,
        const QString& workingDirectory,
        QObject*       parent = nullptr
    );
    ~TerminalSession() override;

    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] QString shellPath() const { return m_shellPath; }

signals:
    void dataReceived(const QString& data);
    void sessionEnded(int exitCode);

public slots:
    // Send a line of input to the shell (newline appended automatically)
    void sendInput(const QString& text);
    void terminate();

private slots:
    void onReadyReadStdOut();
    void onReadyReadStdErr();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    QProcess* m_process = nullptr;
    QString   m_shellPath;
};

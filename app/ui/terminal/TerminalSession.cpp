// TerminalSession.cpp — QProcess shell session implementation
#include "TerminalSession.h"

#include <QtGlobal>
#include <QDebug>
#include <QProcessEnvironment>

TerminalSession::TerminalSession(
    const QString& shellPath,
    const QString& shellArgs,
    const QString& workingDirectory,
    QObject*       parent
)
    : QObject(parent),
      m_process(new QProcess(this)),
      m_shellPath(shellPath)
{
    m_process->setWorkingDirectory(workingDirectory);

    // Merge stderr into stdout so we get a single stream — easier for the widget
    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &TerminalSession::onReadyReadStdOut);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &TerminalSession::onReadyReadStdErr);
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TerminalSession::onProcessFinished);

    // Build arg list from space-separated string
    QStringList args = shellArgs.split(' ', Qt::SkipEmptyParts);

    // Set environment variables to force color output in tools (like git, npm, gemini, etc.)
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
#if defined(Q_OS_WIN)
    env.insert("TERM", "dumb");
    env.insert("VCPKG_MAX_CONCURRENCY", "1");
    env.insert("MAKEFLAGS", "-j1");
    // Disable ConPTY in MSYS2/Cygwin to prevent hangs when run under a non-console parent (QProcess)
    env.insert("MSYS", "disable_pcon");
    env.insert("CYGWIN", "disable_pcon");
#else
    env.insert("TERM", "xterm-256color");
#endif
    env.insert("COLORTERM", "truecolor");
    env.insert("CLICOLOR", "1");
    env.insert("CLICOLOR_FORCE", "1");
    env.insert("FORCE_COLOR", "1");
    env.insert("GIT_CONFIG_PARAMETERS", "'color.ui=always'");
    m_process->setProcessEnvironment(env);

    qDebug() << "[Terminal] Starting shell:" << shellPath << args
             << "in" << workingDirectory;

    m_process->start(shellPath, args);

    if (!m_process->waitForStarted(3000))
    {
        qDebug() << "[Terminal] Failed to start shell:" << m_process->errorString();
        emit sessionEnded(-1);
    }
}

TerminalSession::~TerminalSession()
{
    terminate();
}

bool TerminalSession::isRunning() const
{
    return m_process && m_process->state() == QProcess::Running;
}

void TerminalSession::sendInput(const QString& text)
{
    if (!isRunning()) return;
    QByteArray data = (text + "\n").toUtf8();
    m_process->write(data);
    m_process->waitForBytesWritten(500);
}

void TerminalSession::terminate()
{
    if (m_process && m_process->state() != QProcess::NotRunning)
    {
        m_process->terminate();
        if (!m_process->waitForFinished(2000))
            m_process->kill();
    }
}

void TerminalSession::onReadyReadStdOut()
{
    const QByteArray raw = m_process->readAllStandardOutput();
    if (!raw.isEmpty())
        emit dataReceived(QString::fromUtf8(raw));
}

void TerminalSession::onReadyReadStdErr()
{
    const QByteArray raw = m_process->readAllStandardError();
    if (!raw.isEmpty())
        emit dataReceived(QString::fromUtf8(raw));
}

void TerminalSession::onProcessFinished(int exitCode, QProcess::ExitStatus /*status*/)
{
    qDebug() << "[Terminal] Shell exited with code" << exitCode;
    emit sessionEnded(exitCode);
}

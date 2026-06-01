// TerminalPanel.h — Bottom panel: Output tab + one-or-more Terminal tabs
#pragma once

#include <QWidget>

class QTabWidget;
class QPushButton;
class OutputDisplay;
class TerminalWidget;

class TerminalPanel : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalPanel(QWidget* parent = nullptr);
    ~TerminalPanel() override = default;

    // Forward Output-display log calls
    void log(const QString& output, const QString& error) const;

    // Show/hide the whole panel (toggle like VSCode)
    void toggle();

    // Set the project directory — new terminals will cd there automatically
    void setProjectDirectory(const QString& dir);

    // Set the shell from settings (called after settings are saved)
    void setShell(const QString& shellPath, const QString& shellArgs);

    // Make the Output tab active (called when a run completes)
    void showOutputTab();

    // Run command in active terminal tab (creates tab if none/exited)
    void executeCommand(const QString& command);

    // Returns a pointer to the output display (used by MainWindow)
    OutputDisplay* outputDisplay() const { return m_outputDisplay; }

public slots:
    void addTerminalTab();

private slots:
    void closeTab(int index);
    void onTerminalTitleChanged(const QString& title);

private:
    QTabWidget*    m_tabs          = nullptr;
    OutputDisplay* m_outputDisplay = nullptr;
    QString        m_projectDir;
    QString        m_shellPath     = "powershell.exe";
    QString        m_shellArgs     = "-NoExit -NoLogo";

    int            m_outputTabIndex = 0;
    int            m_terminalCount  = 0;

    TerminalWidget* createTerminalWidget();
    QPushButton*    makeCloseButton(int tabIndex);
};

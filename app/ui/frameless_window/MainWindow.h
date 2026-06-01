//
// Created by talik on 11/12/2025.
//

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "FramelessWindow.h"

class CustomDrawer; // Forward declaration
class Editor;
class Frame;
class TerminalPanel;
class QPushButton;
class QSplitter;

class MainWindow final : public FramelessWindow
{
public slots:
    void processResultSlot(int exitCode, const QString& output, const QString& error) const;
    void processStatusSlot(const QString&, int timeout = 5000) const;
    void updateDrawer() const;
    void onNewFileTriggered();
    void onOpenFileTriggered();
    void onSaveFileTriggered();
    void onBuildProjectTriggered();
    void onRunProjectTriggered();

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    [[nodiscard]] Editor* getEditor() const;
    void onShowOutputButtonClicked() const;

    OutputDisplay* outputDisplay() const;
    [[nodiscard]] TerminalPanel* terminalPanel() const { return m_terminalPanel; }

protected:
    bool maybeSave() override;
    void onApplySettingChanges() override;

private:
    void buildOrRunProject(bool buildOnly);
    QString findCMakeExecutable(const QString& checkDir, const QString& buildDir, const QString& baseName) const;
    QString findMakefileExecutable(const QString& checkDir, const QString& baseName) const;
    QString detectVcpkgTriplet() const;

    TerminalPanel* m_terminalPanel;
    CustomDrawer* m_drawer;
    Editor* m_editor;

    // buttons
    QPushButton* m_folderButton;
    QPushButton* m_outputButton;

    // splitters
    QSplitter* rightSideSplitter;
    QSplitter* topAreaSplitter;
};


#endif //MAIN_WINDOW_H

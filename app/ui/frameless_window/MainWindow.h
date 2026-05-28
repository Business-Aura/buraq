//
// Created by talik on 11/12/2025.
//

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "FramelessWindow.h"

class CustomDrawer; // Forward declaration
class Editor;
class Frame;
class OutputDisplay;
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

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    [[nodiscard]] Editor* getEditor() const;
    void onShowOutputButtonClicked() const;

protected:
    bool maybeSave() override;

private:
    OutputDisplay* m_outPutArea;
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

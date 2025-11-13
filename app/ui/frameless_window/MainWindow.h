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

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    [[nodiscard]] Editor* getEditor() const;
    void onShowOutputButtonClicked() const;

private:
    std::unique_ptr<OutputDisplay> m_outPutArea;
    std::unique_ptr<CustomDrawer> m_drawer;
    std::unique_ptr<Editor> m_editor;

    // buttons
    std::unique_ptr<QPushButton> m_folderButton;
    std::unique_ptr<QPushButton> m_outputButton;

    // splitters
    std::unique_ptr<QSplitter> rightSideSplitter;
    std::unique_ptr<QSplitter> topAreaSplitter;
};


#endif //MAIN_WINDOW_H

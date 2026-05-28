//
// Created by talik on 11/12/2025.
//

#include <QStatusBar>
#include <QVBoxLayout>
#include <QSplitter>

#include <QMessageBox>
#include "editor/Editor.h"
#include "ToolBar.h"
#include "MainWindow.h"
#include "CustomDrawer.h"
#include "Frame/Frame.h"
#include "output_display/OutputDisplay.h"

MainWindow::MainWindow(QWidget* parent) : FramelessWindow(parent)
{
    m_outPutArea = new OutputDisplay(this);
    m_editor = new Editor(this);
    m_drawer = new CustomDrawer(m_editor);

    const auto main_content_widget = m_Frame->getMainContentWidget();
    const auto contentArea = new QWidget(main_content_widget);
    main_content_widget->layout()->addWidget(contentArea);

    // 1. MAIN LAYOUT: A horizontal layout to separate the left controls from the main content.
    QHBoxLayout* mainLayout = new QHBoxLayout(contentArea);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 2. LEFT CONTROL PANEL: A vertical layout for the top and bottom buttons.
    const auto leftPanelLayout = m_Frame->getLeftSidePanelLayout();

    m_folderButton = new QPushButton("🗀", contentArea);
    m_folderButton->setObjectName("folderButton");
    m_folderButton->setFixedSize(35, 35);

    connect(m_folderButton, &QPushButton::clicked, this, &MainWindow::updateDrawer);

    m_outputButton = new QPushButton("❯_", contentArea);
    m_outputButton->setObjectName("outputConsoleButton");
    m_outputButton->setFixedSize(35, 35);

    connect(m_outputButton, &QPushButton::clicked, this, &MainWindow::onShowOutputButtonClicked);

    leftPanelLayout->addWidget(m_folderButton); // Add button to the top
    leftPanelLayout->addStretch(); // Pushes buttons to top and bottom
    leftPanelLayout->addWidget(m_outputButton); // Add button to the bottom

    // 3a. Top Area (Editor + Drawer)
    QWidget* topAreaWidget = new QWidget();
    QHBoxLayout* topAreaLayout = new QHBoxLayout(topAreaWidget);
    topAreaLayout->setSpacing(0);
    topAreaLayout->setContentsMargins(0, 0, 0, 0);

    // 3. MAIN VERTICAL SPLITTER: Separates the top (editor/drawer) from the bottom (output).
    rightSideSplitter = new QSplitter(Qt::Vertical);

    // 4. HORIZONTAL SPLITTER: This will go in the top section of the vertical splitter.
    // It separates the drawer from the editor.
    topAreaSplitter = new QSplitter(Qt::Horizontal);

    // Add the drawer and editor to the HORIZONTAL splitter
    topAreaSplitter->addWidget(m_drawer);
    topAreaSplitter->addWidget(m_editor);
    topAreaSplitter->setSizes({250, 750}); // Initial widths for drawer and editor

    // 5. BOTTOM AREA (Output)

    // 6. ASSEMBLE THE VERTICAL SPLITTER:
    // Add the horizontal splitter (as the top widget) and the output area (as the bottom widget).
    rightSideSplitter->addWidget(topAreaSplitter);
    rightSideSplitter->addWidget(m_outPutArea);
    rightSideSplitter->setSizes({500, 200}); // Initial heights for top and bottom sections

    // 7. ASSEMBLE THE MAIN LAYOUT
    mainLayout->addWidget(rightSideSplitter, 1); // The '1' stretch factor allows it to expand

    // 8. Connect ToolBar signals
    if (m_toolBar)
    {
        connect(m_toolBar, &ToolBar::newFileTriggered, this, &MainWindow::onNewFileTriggered);
        connect(m_toolBar, &ToolBar::openFileTriggered, this, &MainWindow::onOpenFileTriggered);
        connect(m_toolBar, &ToolBar::saveFileTriggered, this, &MainWindow::onSaveFileTriggered);
    }
}

MainWindow::~MainWindow() = default;

bool MainWindow::maybeSave()
{
    if (!m_editor || !m_editor->isDirty()) return true;

    const QMessageBox::StandardButton ret = QMessageBox::warning(this, "Buraq Editor",
                                "The document has been modified.\nDo you want to save your changes?",
                                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save)
    {
        m_editor->saveFile();
        return true;
    }

    if (ret == QMessageBox::Cancel)
    {
        return false;
    }

    return true;
}

void MainWindow::onNewFileTriggered()
{
    if (m_editor)
    {
        m_editor->clear();
        processStatusSlot("New file created.");
    }
}

void MainWindow::onOpenFileTriggered()
{
    if (m_drawer)
    {
        m_drawer->onAddFolderButtonClicked();
    }
}

void MainWindow::onSaveFileTriggered()
{
    if (m_editor)
    {
        m_editor->saveFile();
    }
}

void MainWindow::processResultSlot(const int exitCode, const QString& output, const QString& error) const
{
    if (m_outPutArea == nullptr) return;

    if (m_outPutArea->show(); exitCode == 0)
    {
        m_outPutArea->log(output, error);

        processStatusSlot(error.isEmpty() ? "Completed!" : "Completed with errors.");
    }
    else
    {
        processStatusSlot("Process failed!");
        m_outPutArea->log("", error);
    }
}

void MainWindow::processStatusSlot(const QString& message, const int timeout) const
{
    if (m_statusBar)
    {
        m_statusBar->showMessage(message, timeout);
    }
}

void MainWindow::onShowOutputButtonClicked() const
{
    qDebug() << "Open or close Output";
    if (m_outPutArea == nullptr) return;

    if (m_outPutArea->isHidden())
    {
        m_outPutArea->show();
    }
    else
    {
        m_outPutArea->hide();
    }
}

Editor* MainWindow::getEditor() const
{
    return m_editor;
}

void MainWindow::updateDrawer() const
{
    qDebug() << "Open or close Drawer";
    if (m_drawer == nullptr) return;

    if (m_drawer->isHidden())
    {
        m_drawer->show();
    }
    else
    {
        m_drawer->hide();
    }
}

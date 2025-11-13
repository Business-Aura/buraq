//
// Created by talik on 11/12/2025.
//

#include <QStatusBar>
#include <QVBoxLayout>
#include <QSplitter>

#include "MainWindow.h"
#include "CustomDrawer.h"
#include "Frame/Frame.h"
#include "output_display/OutputDisplay.h"

MainWindow::MainWindow(QWidget* parent) : FramelessWindow(parent),
                                          m_outPutArea(std::make_unique<OutputDisplay>(this)),
                                          m_editor(std::make_unique<Editor>(this))
{
    m_drawer = std::make_unique<CustomDrawer>(m_editor.get());

    const auto main_content_widget = m_Frame->getMainContentWidget();
    const auto contentArea = new QWidget(main_content_widget);
    main_content_widget->layout()->addWidget(contentArea);

    // 1. MAIN LAYOUT: A horizontal layout to separate the left controls from the main content.
    QHBoxLayout* mainLayout = new QHBoxLayout(contentArea);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 2. LEFT CONTROL PANEL: A vertical layout for the top and bottom buttons.
    const auto leftPanelLayout = m_Frame->getLeftSidePanelLayout();

    m_folderButton = std::make_unique<QPushButton>("🗀", contentArea);
    m_folderButton->setObjectName("folderButton");
    m_folderButton->setFixedSize(35, 35);

    connect(m_folderButton.get(), &QPushButton::clicked, this, &MainWindow::updateDrawer);

    m_outputButton = std::make_unique<QPushButton>("❯_", contentArea);
    m_outputButton->setObjectName("outputConsoleButton");
    m_outputButton->setFixedSize(35, 35);

    connect(m_outputButton.get(), &QPushButton::clicked, this, &MainWindow::onShowOutputButtonClicked);

    leftPanelLayout->addWidget(m_folderButton.get()); // Add button to the top
    leftPanelLayout->addStretch(); // Pushes buttons to top and bottom
    leftPanelLayout->addWidget(m_outputButton.get()); // Add button to the bottom

    // 3a. Top Area (Editor + Drawer)
    QWidget* topAreaWidget = new QWidget();
    QHBoxLayout* topAreaLayout = new QHBoxLayout(topAreaWidget);
    topAreaLayout->setSpacing(0);
    topAreaLayout->setContentsMargins(0, 0, 0, 0);

    // 3. MAIN VERTICAL SPLITTER: Separates the top (editor/drawer) from the bottom (output).
    rightSideSplitter = std::make_unique<QSplitter>(Qt::Vertical);

    // 4. HORIZONTAL SPLITTER: This will go in the top section of the vertical splitter.
    // It separates the drawer from the editor.
    topAreaSplitter = std::make_unique<QSplitter>(Qt::Horizontal);

    // Add the drawer and editor to the HORIZONTAL splitter
    topAreaSplitter->addWidget(m_drawer.get());
    topAreaSplitter->addWidget(m_editor.get());
    topAreaSplitter->setSizes({250, 750}); // Initial widths for drawer and editor

    // 5. BOTTOM AREA (Output)

    // 6. ASSEMBLE THE VERTICAL SPLITTER:
    // Add the horizontal splitter (as the top widget) and the output area (as the bottom widget).
    rightSideSplitter->addWidget(topAreaSplitter.get());
    rightSideSplitter->addWidget(m_outPutArea.get());
    rightSideSplitter->setSizes({500, 200}); // Initial heights for top and bottom sections

    // 7. ASSEMBLE THE MAIN LAYOUT
    mainLayout->addWidget(rightSideSplitter.get(), 1); // The '1' stretch factor allows it to expand
}

MainWindow::~MainWindow() = default;

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
    return m_editor.get();
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

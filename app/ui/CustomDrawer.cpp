// MIT License
//
// Copyright (c)  "2025" Talik A. Kasozi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//
// Created by talik on 3/9/2024.
//

#include "CustomDrawer.h"
#include "BuraqFileIconProvider.h"
#include <QGridLayout>
#include <QLabel>
#include <QFileDialog>
#include <QPushButton>
#include <QHeaderView>
#include <QFileInfo>
#include "../database/db_conn.h"

CustomDrawer::CustomDrawer(Editor* editor) : QWidget(editor), editor(editor)
{
    setFixedWidth(DrawerMeasurements::width);
    // Consider if setMaximumHeight(500) is truly desired, or if content should dictate height.
    // If content can grow beyond 500, remove this line or make it a maximum.
    // setMaximumHeight(500);
    setFixedWidth(250);

    // 1. Set up the main vertical layout for the CustomDrawer.
    pLayout = new QVBoxLayout();
    setLayout(pLayout); // Transfer ownership to 'this' QWidget

    // Access the layout via layout() member function after transfer.
    QVBoxLayout* mainVLayout = qobject_cast<QVBoxLayout*>(layout());
    mainVLayout->setSpacing(6); // Spacing between widgets in the main layout
    mainVLayout->setContentsMargins(6, 8, 6, 4); // Margins around the main layout's content

    // 2. Create a container widget for the header section (label and add button).
    QWidget* headerPanel = new QWidget(this);

    // 3. Create a horizontal layout for the header section.
    QHBoxLayout* headerLayout = new QHBoxLayout(headerPanel);
    headerLayout->setContentsMargins(4, 0, 4, 0); // Clean margins within the header
    headerLayout->setSpacing(4); // Small spacing between label and buttons

    // 4. Create the "EXPLORER" label.
    workspaceLabel = new QLabel("EXPLORER", headerPanel);
    workspaceLabel->setObjectName("HeaderLabel");

    // 5. Create the "addFolder" button.
    addFolder = new QPushButton(this);
    addFolder->setObjectName("AddFolder");
    addFolder->setIcon(QIcon(":/icons/ui/add_folder.svg"));
    addFolder->setIconSize(QSize(14, 14));
    addFolder->setFixedSize(24, 24);
    addFolder->setToolTip("Open Workspace Folder...");
    connect(addFolder, &QPushButton::clicked, this, &CustomDrawer::onAddFolderButtonClicked);

    // 5b. Create the "closeWorkspace" button.
    closeWorkspace = new QPushButton(this);
    closeWorkspace->setObjectName("CloseWorkspace");
    closeWorkspace->setIcon(QIcon(":/icons/ui/close_folder.svg"));
    closeWorkspace->setIconSize(QSize(14, 14));
    closeWorkspace->setFixedSize(24, 24);
    closeWorkspace->setToolTip("Close Workspace");
    connect(closeWorkspace, &QPushButton::clicked, this, &CustomDrawer::onCloseWorkspaceButtonClicked);

    // 6. Add widgets to the header layout.
    headerLayout->addWidget(workspaceLabel);
    headerLayout->addStretch(1); // This stretch pushes the button to the far right.
    headerLayout->addWidget(addFolder);
    headerLayout->addWidget(closeWorkspace);

    // 7. Add the header panel to the main vertical layout of CustomDrawer.
    mainVLayout->addWidget(headerPanel);

    // 8. Add a sleek 1px hairline separator below the header.
    QFrame* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setFixedHeight(1);
    separator->setStyleSheet("background-color: #2b2b2b; border: none;");
    mainVLayout->addWidget(separator);

    // 9. Initialize the File System Model and Tree View
    m_fileSystemModel = new QFileSystemModel(this);
    m_iconProvider = std::make_unique<BuraqFileIconProvider>();
    m_fileSystemModel->setIconProvider(m_iconProvider.get());
    m_fileSystemModel->setRootPath(""); // Default to empty
    m_fileSystemModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs);

    m_treeView = new QTreeView(this);
    m_treeView->setHeaderHidden(true); // Hide the top header row
    m_treeView->setAnimated(true);
    m_treeView->setIndentation(16);
    m_treeView->setSortingEnabled(true);
    m_treeView->setFrameShape(QFrame::NoFrame);
    m_treeView->setMouseTracking(true);
    // Remove the frame to blend in with the dark theme
    m_treeView->setFrameShape(QFrame::NoFrame);
    
    connect(m_treeView, &QTreeView::clicked, this, &CustomDrawer::onTreeViewClicked);

    // Add the tree view to the main layout with a stretch factor of 1 so it fills the remaining space
    mainVLayout->addWidget(m_treeView, 1); 

    // Read from the database and load the active workspace folder
    QString savedWorkspace = database::getWorkspacePath();
    if (!savedWorkspace.isEmpty()) {
        setWorkspace(savedWorkspace, false);

        // Restore the last opened file if it exists
        QString lastFile = database::getLastOpenedFilePath();
        if (!lastFile.isEmpty() && QFileInfo::exists(lastFile) && QFileInfo(lastFile).isFile()) {
            if (editor) {
                editor->openAndParseFile(lastFile, QFile::OpenModeFlag::ReadWrite);
            }
        }
    } else {
        m_treeView->setModel(nullptr);
    }

    // drawer is collapsed by default.
    // show(); // Uncomment if you want it visible by default
}

CustomDrawer::~CustomDrawer() = default;

void CustomDrawer::toggle()
{
    if (isVisible())
    {
        hide();
    }
    else
    {
        show();
    }
}

void CustomDrawer::onAddFolderButtonClicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, "Select Workspace Folder",
                                                        "",
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dirPath.isEmpty())
    {
        setWorkspace(dirPath);
    }
}

void CustomDrawer::onCloseWorkspaceButtonClicked()
{
    database::clearWorkspacePath();
    
    // Clear the model of the tree view
    m_treeView->setModel(nullptr); 
    workspaceLabel->setText("EXPLORER");
    
    if (editor) {
        editor->clear();
    }
    emit workspaceChanged("");
}

void CustomDrawer::setWorkspace(const QString& dirPath, bool saveToDb)
{
    if (saveToDb) {
        database::setWorkspacePath(dirPath);
    }
    
    // Set model and hide columns
    m_treeView->setModel(m_fileSystemModel);
    for (int i = 1; i < m_fileSystemModel->columnCount(); ++i) {
        m_treeView->hideColumn(i);
    }
    
    // Update the model and view
    QModelIndex rootIndex = m_fileSystemModel->setRootPath(dirPath);
    m_treeView->setRootIndex(rootIndex);

    // Update label to show uppercase folder name
    QFileInfo dirInfo(dirPath);
    workspaceLabel->setText(dirInfo.fileName().toUpper());
    
    emit workspaceChanged(dirPath);
}

void CustomDrawer::onTreeViewClicked(const QModelIndex& index)
{
    // Only open the file in the editor if it's an actual file, not a directory
    if (!m_fileSystemModel->isDir(index))
    {
        QString filePath = m_fileSystemModel->filePath(index);
        if (editor)
        {
            editor->openAndParseFile(filePath, QFile::OpenModeFlag::ReadWrite);
        }
    }
}

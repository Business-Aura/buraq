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
    mainVLayout->setSpacing(8); // Spacing between widgets in the main layout
    mainVLayout->setContentsMargins(2, 4, 2, 4); // Margins around the main layout's content

    // 2. Create a container widget for the header section (label and add button).
    QWidget* headerPanel = new QWidget(this);

    // 3. Create a horizontal layout for the header section.
    QHBoxLayout* headerLayout = new QHBoxLayout(headerPanel);
    headerLayout->setContentsMargins(0, 0, 0, 0); // No extra margins within the header
    headerLayout->setSpacing(5); // Small spacing between label and button

    // 4. Create the "Workspace" label.
    workspaceLabel = new QLabel("Workspace", headerPanel);
    workspaceLabel->setObjectName("HeaderLabel");
    // Apply a bold font to make it stand out.
    workspaceLabel->setFont(QFont("Segoe UI", 10, QFont::Bold));

    // 5. Create the "addFolder" button.
    addFolder = new QPushButton("🗀", this); // Folder icon
    addFolder->setObjectName("AddFolder");
    addFolder->setFixedSize(25, headerPanel->height());
    connect(addFolder, &QPushButton::clicked, this, &CustomDrawer::onAddFolderButtonClicked);

    // 5b. Create the "closeWorkspace" button.
    closeWorkspace = new QPushButton("✕", this); // Close icon
    closeWorkspace->setObjectName("CloseWorkspace");
    closeWorkspace->setFixedSize(25, headerPanel->height());
    closeWorkspace->setFlat(true);
    closeWorkspace->setStyleSheet("QPushButton { color: #888; border: none; font-weight: bold; background: transparent; } "
                                  "QPushButton:hover { color: #ff5555; }");
    connect(closeWorkspace, &QPushButton::clicked, this, &CustomDrawer::onCloseWorkspaceButtonClicked);

    // 6. Add widgets to the header layout.
    headerLayout->addWidget(workspaceLabel);
    headerLayout->addStretch(1); // This stretch pushes the button to the far right.
    headerLayout->addWidget(addFolder);
    headerLayout->addWidget(closeWorkspace);

    // 7. Add the header panel to the main vertical layout of CustomDrawer.
    mainVLayout->addWidget(headerPanel);

    // 8. Add a separator line for visual distinction below the header.
    QFrame* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine); // Horizontal line
    separator->setFrameShadow(QFrame::Sunken); // Gives a sunken 3D effect
    mainVLayout->addWidget(separator);

    // 9. Initialize the File System Model and Tree View
    m_fileSystemModel = new QFileSystemModel(this);
    m_fileSystemModel->setRootPath(""); // Default to empty
    m_fileSystemModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs);

    m_treeView = new QTreeView(this);
    m_treeView->setHeaderHidden(true); // Hide the top header row
    
    m_treeView->setAnimated(false);
    m_treeView->setIndentation(20);
    m_treeView->setSortingEnabled(true);
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
    workspaceLabel->setText("Workspace");
    
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

    // Update label to show the folder name instead of just "Workspace"
    QFileInfo dirInfo(dirPath);
    workspaceLabel->setText(dirInfo.fileName());
    
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

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

#include <QGridLayout>
#include <QLabel>
#include <QFileDialog>
#include "CustomDrawer.h"

#include <QPushButton>

#include "IconButton.h"
#include "../database/db_conn.h"

CustomDrawer::CustomDrawer(Editor* editor) : QWidget(editor), editor(editor)
{
    setFixedWidth(DrawerMeasurements::width);
    // Consider if setMaximumHeight(500) is truly desired, or if content should dictate height.
    // If content can grow beyond 500, remove this line or make it a maximum.
    // setMaximumHeight(500);
    setFixedWidth(250);

    // 1. Set up the main vertical layout for the CustomDrawer.
    // The QWidget (this) takes ownership of the layout when setLayout() is called.
    // Therefore, we release the unique_ptr to transfer ownership.
    pLayout = std::make_unique<QVBoxLayout>();
    setLayout(pLayout.release()); // Transfer ownership to 'this' QWidget

    // Access the layout via layout() member function after transfer.
    QVBoxLayout* mainVLayout = qobject_cast<QVBoxLayout*>(layout());
    mainVLayout->setSpacing(8); // Spacing between widgets in the main layout
    mainVLayout->setContentsMargins(2, 4, 2, 4); // Margins around the main layout's content

    // 2. Create a container widget for the header section (label and add button).
    // This widget will be parented to CustomDrawer, so no unique_ptr needed here.
    QWidget* headerPanel = new QWidget(this);

    // 3. Create a horizontal layout for the header section.
    // headerPanel takes ownership of this layout.
    QHBoxLayout* headerLayout = new QHBoxLayout(headerPanel);
    headerLayout->setContentsMargins(0, 0, 0, 0); // No extra margins within the header
    headerLayout->setSpacing(5); // Small spacing between label and button

    // 4. Create the "Workspace" label.
    // Parented to headerPanel, so no unique_ptr needed.
    const auto workspaceLabel = new QLabel("Workspace", headerPanel);
    workspaceLabel->setObjectName("HeaderLabel");
    // Apply a bold font to make it stand out.
    workspaceLabel->setFont(QFont("Segoe UI", 10, QFont::Bold));

    // 5. Create the "addFile" button.
    // addFile is a unique_ptr member variable, so we initialize it.
    addFile = std::make_unique<QPushButton>("📄", this);
    addFile->setObjectName("AddFile");
    addFile->setFixedSize(25, headerPanel->height());

    // Connect its clicked signal to your slot.
    connect(addFile.get(), &IconButton::clicked, this, &CustomDrawer::onAddButtonClicked);

    // 6. Add widgets to the header layout.
    // The workspaceLabel will take up available space, pushing addFile to the right.
    headerLayout->addWidget(workspaceLabel);
    headerLayout->addStretch(1); // This stretch pushes the button to the far right.
    // Transfer ownership of the IconButton to the layout.
    headerLayout->addWidget(addFile.release());

    // 7. Add the header panel to the main vertical layout of CustomDrawer.
    mainVLayout->addWidget(headerPanel);

    // 8. Add a separator line for visual distinction below the header.
    QFrame* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine); // Horizontal line
    separator->setFrameShadow(QFrame::Sunken); // Gives a sunken 3D effect
    mainVLayout->addWidget(separator);

    // 9. Add a stretch to the main layout to push content to the top.
    // This ensures that if there's not enough content to fill the drawer,
    // the existing widgets stay at the top.
    mainVLayout->addStretch(1);

    // Read from the database and load all the files that were
    // previously opened. This function should add widgets to `mainVLayout`.
    showPreviouslyOpenedFiles();

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

void CustomDrawer::onAddButtonClicked()
{
    QFileDialog dialog(this);
    // Removes name filter to allow future support for other languages
    dialog.setViewMode(QFileDialog::Detail);

    if (dialog.exec())
    {
        if (const QStringList fileNames = dialog.selectedFiles(); !fileNames.empty())
        {
            const QString& filePath = fileNames.at(0);
            const std::string &fileName = file_utils::getFilename(filePath.toStdString());

            const QString qFileName = QString::fromStdString(fileName);
            if (const QVariant result = database::insertFile(filePath, qFileName); result.isValid())
            {
                createFileLabel(filePath, qFileName, true);
            }
        }
    }
}

void CustomDrawer::createFileLabel(
    const QString& filePath, const QString& fileName, bool shouldAutoOpenFile) const
{
    const auto label = new FilePathLabel(filePath, nullptr);

    connect(label, &FilePathLabel::clicked, this, &CustomDrawer::onFileLabelClick);

    const auto mainVLayout = qobject_cast<QVBoxLayout*>(layout());
    mainVLayout->addWidget(label);

    if (!filePath.isEmpty())
    {
        openFilePath(label, filePath, fileName);

        if (shouldAutoOpenFile)
        {
            // updated the side panel to show this as the newly active file
            emit label->clicked();
        }
    }
}

void CustomDrawer::openFilePath(FilePathLabel* label, const QString& filePath, const QString& fileName)
{
    if (!filePath.isEmpty())
    {
        label->setText(fileName);

        QFile file(filePath);
        file.close();
    }
}

void CustomDrawer::onFileLabelClick()
{
    QObject* senderObj = sender();
    const QObject* activeItem = state.activeFileLabel;
    if (activeItem == senderObj)
    {
        // do nothing.
        return;
    }

    if (activeItem != nullptr)
    {
        if (const auto prevActiveLabel = dynamic_cast<FilePathLabel*>(state.activeFileLabel); prevActiveLabel !=
            nullptr)
        {
            prevActiveLabel->reset();
        }
    }

    // update active label
    if (
        const auto label = dynamic_cast<FilePathLabel*>(senderObj);
        label != nullptr
    )
    {
        // sets active label css
        label->activeLabel();

        setActive(label);

        // Update the editor
        if (editor)
        {
            editor->openAndParseFile(label->getFilePath(), QFile::OpenModeFlag::ReadWrite);
        }
    }
}

void CustomDrawer::setActive(QWidget* pLabel)
{
    state.activeFileLabel = pLabel;
}

void CustomDrawer::showPreviouslyOpenedFiles() const
{
    auto previousOpenedFiles = database::findPreviouslyOpenedFiles();

    bool isFirstTime = true;
    for (const auto file : previousOpenedFiles)
    {
        try
        {
            createFileLabel(file->getFilePath(), file->getFileName(), isFirstTime);

            if (isFirstTime)
            {
                isFirstTime = false;
            }
        }
        catch (...)
        {
            database::deleteRow(file->getFilePath());
        }
    }
}

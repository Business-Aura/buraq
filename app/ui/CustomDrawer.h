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

#ifndef CUSTOM_DRAWER_H
#define CUSTOM_DRAWER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTreeView>
#include <QFileSystemModel>
#include "editor/Editor.h"

class QPushButton;
class QLabel;

class CustomDrawer : public QWidget {
    Q_OBJECT

public slots:
    void onAddFolderButtonClicked();
    void onCloseWorkspaceButtonClicked();

private slots:
    void onTreeViewClicked(const QModelIndex& index);

signals:
    void workspaceChanged(const QString& dirPath);

public:
    enum DrawerMeasurements {
        width = 256,
    };

    explicit CustomDrawer(Editor *editor);
    ~CustomDrawer() override = default;

    void toggle();

private:
    Editor *editor;
    QPushButton* addFolder;
    QPushButton* closeWorkspace;
    QLabel* workspaceLabel;
    QVBoxLayout* pLayout;

    QTreeView* m_treeView;
    QFileSystemModel* m_fileSystemModel;

    void setWorkspace(const QString& dirPath, bool saveToDb = true);
};

#endif //CUSTOM_DRAWER_H

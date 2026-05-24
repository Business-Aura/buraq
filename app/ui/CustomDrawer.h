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
#include <QGridLayout>
#include "editor/Editor.h"
#include "FilePathLabel.h"

class QPushButton;

struct drawerState {
	QWidget *activeFileLabel;
};

class CustomDrawer : public QWidget {
Q_OBJECT

	void setActive(QWidget *pLabel);

public slots:
    void onAddButtonClicked();

private slots:

	void onFileLabelClick();

public:
	enum DrawerMeasurements {
		width = 256,
	};

	explicit CustomDrawer(Editor *editor);

	void toggle();

	// smart pointer will be cleaned up.
	~CustomDrawer() override = default;

	void showPreviouslyOpenedFiles() const;

private:
	Editor *editor;
	std::unique_ptr<QPushButton> addFile;
	std::unique_ptr<QVBoxLayout> pLayout;

	struct drawerState state = {.activeFileLabel = nullptr};

	static void openFilePath(FilePathLabel *label, const QString &filePath, const QString &fileName);

	void createFileLabel(const QString &filePath, const QString &fileName, bool shouldAutoOpenFile) const;
};


#endif //CUSTOM_DRAWER_H

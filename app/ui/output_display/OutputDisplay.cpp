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
// Created by talik on 5/1/2024.
//
#include <QScrollArea>
#include <QDateTime>
#include "OutputDisplay.h"
#include "Utils.h"
#include "app_ui/AppUi.h"
#include <QScrollArea>
#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>

void init_main_out_area(QPlainTextEdit*, QVBoxLayout*, int);

OutputDisplay::OutputDisplay(QWidget* window) : QWidget(window), m_window(window)
{
    // TODO This will eventually become a tool bar
    const auto pMainLabel = new QLabel(this);
    pMainLabel->setFixedHeight(25);
    pMainLabel->setText("❯_");

    const auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(pMainLabel);

    main = std::make_unique<QPlainTextEdit>();
    init_main_out_area(main.get(), layout, 0);

    hide();
}

void init_main_out_area(QPlainTextEdit* main, QVBoxLayout* layout, int editorWidth = 0)
{
    // For displaying the output_display
    layout->addWidget(main);

    // Get the current palette
    QPalette palette = main->palette();

    // Set the color for the background of the selection
    palette.setColor(QPalette::Highlight, QColor(0, 120, 215)); // A common blue selection background
    palette.setColor(QPalette::Text, QColor(Qt::white)); // A common blue selection background
    main->setPalette(palette);

    main->setReadOnly(true);
    main->setLineWrapMode(QPlainTextEdit::LineWrapMode::WidgetWidth);

    main->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    if (editorWidth > 0)
    {
        main->setMinimumWidth(editorWidth);
    }

    // --- QSS for Custom Scrollbar ---
    // Apply the stylesheet directly to the textEdit widget
    // This ensures only this textEdit's scrollbars are affected (and its children if any)
    main->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    main->setMinimumHeight(50);
}

void OutputDisplay::toggle()
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

void OutputDisplay::log(const QString& output, const QString& errorOutput) const
{
    QStringList list = output.split("\\u2029");
    QStringList errorsList = errorOutput.split("\\u2029");

    QLabel* error = createLabel(errorOutput, "error");

    // Adds timestamp
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString formattedDateTime = currentDateTime.toString("yyyy-MM-dd hh:mm:ss"); // Custom format
    main->appendHtml("<h4 style=\"color: #FFFDD0;\">Executed: " +
        formattedDateTime + "</h4>");
    // End Adds timestamp

    main->appendHtml("<div>");
    for (const QString& qString : list)
    {
        main->appendHtml("<p  style=\"color: #FFFFFF;\">" + qString + "</p>");
    }
    for (const QString& qString : errorsList)
    {
        // error logs
        main->appendHtml("<p style=\"color: #FF6347;\">" + qString + "</p>");
    }
    main->appendHtml("</div>");

    if (error != nullptr)
    {
        const auto layout = new QVBoxLayout;
        layout->addWidget(error);
    }
}

QLabel* OutputDisplay::createLabel(const QString& text, QString state)
{
    if (text.isEmpty())
        return nullptr;

    auto label = new QLabel;
    label->setText(text);
    label->setFont(QFont("Arial", 32));

	//@formatter:off
	// label->setStyleSheet(
	// 	std::equal(state.begin(), state.end(), "error") ? "color: #FF6347;" : "color: #F9F6EE;");
    //@formatter:on

    label->setAlignment(Qt::AlignLeft);

    // Call adjustSize() after setting the text
    label->adjustSize();

    // Get the recommended size from sizeHint()
    QSize recommendedSize = label->sizeHint();
    recommendedSize.setWidth(recommendedSize.width() * 2); // Add some extra width
    // auto n = std::max(100, recommendedSize.height());
    recommendedSize.setHeight(recommendedSize.height() + 100); // Add some extra width

    // Set the label's size manually
    label->setFixedSize(recommendedSize);

    return label;
}

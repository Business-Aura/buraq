//
// Created by talik on 5/28/2025.
//
#include "EditorMargin.h"
#include "editor/Editor.h"
#include <QHBoxLayout> // Required for QHBoxLayout
#include <QPainter>
#include <QTextBlock>

EditorMargin::EditorMargin(QWidget* windowPtr, QWidget* parent) : CommonWidget(parent), windowPtr(windowPtr)
{
    setFixedWidth(82); // The EditorMargin itself has a fixed width

    const auto main_layout = new QHBoxLayout(this);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);

    // --- 1st Widget: Action Area (contains CodeRunner) - NOW ON THE LEFT ---
    const auto action_widget = new QWidget(this);
    action_widget->setFixedWidth(35);

    const auto action_layout = new QVBoxLayout(action_widget);
    action_layout->setSpacing(0);
    action_layout->setContentsMargins(0, 0, 0, 0);

    codeRunner = new CodeRunner(windowPtr);
    codeRunner->setFixedSize(35, 35);
    action_layout->addWidget(codeRunner);

    // Then, add a stretchable space after it
    action_layout->addStretch(1);

    // Add the action_widget to the main horizontal layout first
    main_layout->addWidget(action_widget);

    // --- 2nd Widget: Line Numbers Area - NOW ON THE RIGHT ---
    line_numbers_widget = new LineNumberAreaWidget(this);
    line_numbers_widget->setObjectName("EditorMargin");
    line_numbers_widget->setFixedWidth(47);

    // Add the line_numbers widget to the main horizontal layout second
    main_layout->addWidget(line_numbers_widget);

    EditorMargin::setupSignals();
}

// Call this method whenever the editor's state changes
// For example, when text is edited, or the view is scrolled.
void EditorMargin::updateState(const buraq::EditorState& newState) const
{
    Q_UNUSED(newState);
    if (line_numbers_widget)
    {
        line_numbers_widget->update();
    }
}

void EditorMargin::onEditorScrolled()
{
    if (line_numbers_widget)
    {
        line_numbers_widget->update();
    }
}

void EditorMargin::updateMarginWidth(const buraq::EditorState& state) const
{
    Q_UNUSED(state);
    if (line_numbers_widget)
    {
        line_numbers_widget->update();
    }
}

void EditorMargin::setupSignals() const
{
    // add impl
}

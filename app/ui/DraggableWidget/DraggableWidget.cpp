//
// Created by talik on 9/1/2025.
//

#include "DraggableWidget.h"

#include <QLabel>
#include <QVBoxLayout>

DraggableWidget::DraggableWidget(QWidget *parent)
    : QWidget(parent)
{
    // --- Give the widget some visual style so we can see it ---
    setFixedSize(150, 100);
    setStyleSheet("background-color: #2196F3; color: white; border-radius: 8px; font-weight: bold;");

    // Add a label to make it clear
    QLabel *label = new QLabel("Drag Me!", this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label, 0, Qt::AlignCenter);
    setLayout(layout);
}

/**
 * @brief Handles the mouse press event to start a drag operation.
 */
void DraggableWidget::mousePressEvent(QMouseEvent *event)
{
    // Check if the left mouse button was pressed
    if (event->button() == Qt::LeftButton) {
        // Store the click position relative to the widget's top-left corner
        m_dragPosition = event->pos();
    }
}

/**
 * @brief Handles the mouse move event to update the widget's position.
 */
void DraggableWidget::mouseMoveEvent(QMouseEvent *event)
{
    // Check if the left mouse button is being held down
    if (event->buttons() & Qt::LeftButton) {
        // Calculate the new top-left position of the widget.
        // The new position is the widget's current position plus the
        // mouse's movement since the last event.
        move(pos() + event->pos() - m_dragPosition);
    }
}

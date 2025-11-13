//
// Created by talik on 9/1/2025.
//

#ifndef DRAGGABLEWIDGET_H
#define DRAGGABLEWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPoint>

/**
 * @brief A simple QWidget that can be dragged around its parent.
 *
 * This widget overrides the mouse press and move events to implement
 * dragging functionality.
 */
class DraggableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DraggableWidget(QWidget *parent = nullptr);

protected:
    // Override the mouse events to handle dragging
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    // Stores the position of the initial mouse click relative to the widget's corner
    QPoint m_dragPosition;
};

#endif // DRAGGABLEWIDGET_H

//
// Created by talik on 2/7/2024.
//

#include <QEvent>
#include <QMouseEvent>

#include "ToolBarEvent.h"

#include "frameless_window/FramelessWindow.h"

bool ToolBarEvent::eventFilter(QObject* watched, QEvent* event)
{
    if (const auto toolBar = dynamic_cast<QWidget*>(watched); toolBar == nullptr || event == nullptr)
    {
        return false; // Pass event to the object's normal handlers
    }

    const auto _event = dynamic_cast<QMouseEvent*>(event);

    switch (event->type())
    {
    case QEvent::MouseButtonDblClick:
        emit showMaximizedOrRestore();
        return true;
    case QEvent::MouseButtonPress:

        if (_event->button() == Qt::LeftButton)
        {
            isMouseBtnPressed = true;
            this->dragStartPos = _event->pos();

            return true;
        }

        return false;
    case QEvent::MouseMove:

        if (isMouseBtnPressed)
        {
            emit dragWindow(_event);
            return true;
        }

        // return false; // Stop event propagation (optional)
    case QEvent::MouseButtonRelease:
        isMouseBtnPressed = false;
        return true;
    default:
        return QObject::eventFilter(watched, event);
    }
}

ToolBarEvent::ToolBarEvent(QObject* parent)
    : QObject(parent),
      dragStartPos(0, 0),
      isMouseBtnPressed(false)
{
}

ToolBarEvent::~ToolBarEvent() = default;

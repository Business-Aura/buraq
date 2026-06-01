//
// Created by talik on 9/1/2025.
//

#ifndef BURAQ_FRAME_H
#define BURAQ_FRAME_H

#include <QWidget>

#include "settings/UserSettings.h"
#include "settings/SettingManager/SettingsManager.h"

class ToolBarEvent;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;
class QToolBar;
class QSize;
class QLabel;

class Frame final : public QWidget
{
    Q_OBJECT

public slots:
    void windowResizeSlot(const QSize &size) const;

private slots:
    void windowDrag(QMouseEvent* event);

public:
    explicit Frame(QWidget* parent, bool hasToolBar = false, QSize minSize = QSize(1200, 720));
    ~Frame() override;

    QWidget* getTitleBar() const
    {
        return m_titleBar;
    }

    QLabel* getTitleLabel() const
    {
        return m_titleLabel;
    }

    QWidget* getToolKitBar() const
    {
        return m_topPanel;
    }

    QWidget* getMainContentWidget() const
    {
        return m_centralWidget;
    }

    QWidget* getBottomPanelWidget() const
    {
        return m_bottomPanel;
    }

    QVBoxLayout* getMainLayout() const
    {
        return m_mainLayout;
    }

    QHBoxLayout* getExtraButtonsLayout() const
    {
        return m_extraButtonsLayout;
    }

    QHBoxLayout* getBottomPanelLayout() const
    {
        return m_bottomPanelLayout;
    }

    QVBoxLayout* getLeftSidePanelLayout() const
    {
        return m_leftSidePanelLayout;
    }

    QVBoxLayout* getRightSidePanelLayout() const
    {
        return m_rightSidePanelLayout;
    }

    QPushButton* getCloseButton() const
    {
        return m_closeButton;
    }

signals:
    void closeWindow();

private:
    QWidget* m_extraButtons;
    QWidget* m_leftSidePanel;
    QWidget* m_titleBar;
    QLabel* m_titleLabel;
    QWidget* m_topPanel;
    QWidget* m_rightSidePanel;
    QWidget* m_bottomPanel;
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QVBoxLayout* m_leftSidePanelLayout;
    QVBoxLayout* m_rightSidePanelLayout;
    QHBoxLayout* m_extraButtonsLayout;
    QHBoxLayout* m_bottomPanelLayout;
    QPushButton* m_closeButton;
    std::unique_ptr<SettingsManager> settingsManager;
    ToolBarEvent* m_titlebarEvents;
    UserSettings m_userPreference;
    QWidget* m_frameContainer;

    Qt::Edges m_resizeEdges;
    int m_resizeMargin = 5; // The pixel margin to detect resizing
    bool m_resizing = false;
    bool m_dragging = false;
    QPoint m_dragPosition; // To store the offset of the mouse click from the m_window's top-left
};

#endif //BURAQ_FRAME_H

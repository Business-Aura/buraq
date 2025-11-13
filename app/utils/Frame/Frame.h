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

class Frame final : public QWidget
{
    Q_OBJECT

private slots:
    void windowDrag(QMouseEvent* event);

public:
    explicit Frame(QWidget* parent, bool hasToolBar = false, QSize minSize = QSize(1200, 720));
    ~Frame() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    QWidget* getTitleBar() const
    {
        return m_titleBar.get();
    }

    QWidget* getToolKitBar() const
    {
        return m_topPanel.get();
    }

    QWidget* getMainContentWidget() const
    {
        return m_centralWidget.get();
    }

    QWidget* getBottomPanelWidget() const
    {
        return m_bottomPanel.get();
    }

    QVBoxLayout* getMainLayout() const
    {
        return m_mainLayout.get();
    }

    QHBoxLayout* getExtraButtonsLayout() const
    {
        return m_extraButtonsLayout.get();
    }

    QHBoxLayout* getBottomPanelLayout() const
    {
        return m_bottomPanelLayout.get();
    }

    QVBoxLayout* getLeftSidePanelLayout() const
    {
        return m_leftSidePanelLayout.get();
    }

    QVBoxLayout* getRightSidePanelLayout() const
    {
        return m_rightSidePanelLayout.get();
    }

signals:
    void closeWindow();

private:
    // Helper function to update the cursor shape based on position
    void updateCursorShape(const QPoint &pos);

    // Helper function to calculate which edges the mouse is on
    Qt::Edges calculateEdges(const QPoint &pos, int margin) const;

    std::unique_ptr<QWidget> m_extraButtons;
    std::unique_ptr<QWidget> m_leftSidePanel;
    std::unique_ptr<QWidget> m_titleBar;
    std::unique_ptr<QWidget> m_topPanel;
    std::unique_ptr<QWidget> m_rightSidePanel;
    std::unique_ptr<QWidget> m_bottomPanel;
    std::unique_ptr<QWidget> m_centralWidget;
    std::unique_ptr<QVBoxLayout> m_mainLayout;
    std::unique_ptr<QVBoxLayout> m_leftSidePanelLayout;
    std::unique_ptr<QVBoxLayout> m_rightSidePanelLayout;
    std::unique_ptr<QHBoxLayout> m_extraButtonsLayout;
    std::unique_ptr<QHBoxLayout> m_bottomPanelLayout;
    std::unique_ptr<QPushButton> m_closeButton;
    std::unique_ptr<SettingsManager> settingsManager;
    std::unique_ptr<Frame> m_Frame;

    QWidget* parent{};
    ToolBarEvent* m_titlebarEvents;
    UserSettings userPreference;

    Qt::Edges m_resizeEdges;
    int m_resizeMargin = 5; // The pixel margin to detect resizing
    bool m_resizing = false;
    bool m_dragging = false;
    QPoint m_dragPosition; // To store the offset of the mouse click from the m_window's top-left
};

#endif //BURAQ_FRAME_H

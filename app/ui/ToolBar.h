// toolbar.h
#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>

struct IWindow;
class QWidget;
class QMenu;
class QIcon;
class QAction;
class QPushButton;

class ToolBar final : public QToolBar
{
	Q_OBJECT

public:
	explicit ToolBar(const QString &title, QWidget *parent = nullptr);
	explicit ToolBar(QWidget *parent = nullptr);

	void addCustomAction(const QString &text, const QIcon &icon = QIcon());
	void addFileMenu();
	void addBuildMenu();
	void addBuildConfigMenu(const QString &initialConfig);
	QString activeBuildConfig() const;
	QWidget* m_window;

	signals:
		void customActionTriggered();
	void newFileTriggered();
	void openFileTriggered();
	void saveFileTriggered();
	void exitTriggered();
	void buildProjectTriggered();
	void runProjectTriggered();
	void buildConfigChanged(const QString &config);

private slots:
	void onCustomActionTriggered();
	void onNewFile();
	void onOpenFile();
	void onSaveFile();
	void onExit();
	void onFileMenuButtonClicked(); // Add this new slot declaration
	void onBuildMenuButtonClicked();
	void onBuildProject();
	void onRunProject();
	void onBuildConfigMenuButtonClicked();
	void onDebugConfigSelected();
	void onReleaseConfigSelected();

private:
	QAction *m_customAction;
	QMenu *m_fileMenu;
	QPushButton* m_fileMenuButton;
	QMenu *m_buildMenu;
	QPushButton* m_buildMenuButton;
	QMenu *m_configMenu;
	QPushButton* m_configMenuButton;
	QString m_activeConfig;
};

#endif // TOOLBAR_H

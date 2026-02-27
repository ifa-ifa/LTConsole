#pragma once
#include <QMainWindow>
#include <QDockWidget>
#include <QCloseEvent>
#include <QMenuBar>
#include <QTimer>
#include <QMenu>

class MainWindow : public QMainWindow {
	Q_OBJECT

public:

	MainWindow();
private slots:
	void saveUiState();
private:

	void closeEvent(QCloseEvent* e);

	QDockWidget* terminalDock;
	QDockWidget* toolboxDock;
	QDockWidget* atlasDock;
	QDockWidget* entityViewerDock;
	QDockWidget* infoWidgetDock;
	QDockWidget* inspectorDock;
	QDockWidget* cutscenePlayerDock;

	QTimer* m_saveStateTimer;

};
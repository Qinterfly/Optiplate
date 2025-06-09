/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Declaration of the MainWindow class
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QThread>

#include "project.h"
#include "propertieseditor.h"

namespace ads
{
class CDockWidget;
class CDockManager;
}

namespace Frontend
{

class Logger;
class ConvergencePlot;
class GeometryPlot;
class OptionsEditor;
class PanelEditor;
class PropertiesViewer;
class SolutionBrowser;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* pParent = nullptr);
    virtual ~MainWindow();
    static Logger* pLogger;

    QSettings& settings();

    // File interaction
    void newProject();
    bool openProject(QString const& pathFile);
    void saveProject();
    void saveAsProject(QString const& pathFile);

    // Solver interaction
    void startSolver();
    void stopSolver();

signals:
    void stopSolverRequested();

private:
    void initializeWindow();
    void closeEvent(QCloseEvent* pEvent) override;

    // Content
    void createContent();
    void createFileActions();
    void createWindowActions();
    void createSolverActions();
    void createHelpActions();
    void createDockManager();
    ads::CDockWidget* createPanelEditor();
    ads::CDockWidget* createPropertiesEditor(QString const& name, PropertyType type, Backend::Properties& properties);
    ads::CDockWidget* createOptionsEditor();
    ads::CDockWidget* createPropertiesViewer();
    ads::CDockWidget* createSolutionBrowser();
    ads::CDockWidget* createConvergencePlot();
    ads::CDockWidget* createGeometryPlot();
    ads::CDockWidget* createLogger();
    void createConnections();

    // State
    void setProjectTitle();
    void setModified(bool flag);
    void setTheme();
    void processProjectChange();
    void updateSolverActions();

    // Recent
    void retrieveRecentProjects();
    void addToRecentProjects();
    void openRecentProject();

    // Settings
    void saveSettings();
    void restoreSettings();

    // Dialogs
    void newProjectDialog();
    void openProjectDialog();
    void saveAsProjectDialog();
    bool saveProjectChangesDialog();
    bool stopSolverDialog();
    void clearResultsDialog();
    void about();

private:
    QSettings mSettings;
    QList<QString> mPathRecentProjects;

    // UI
    ads::CDockManager* mpDockManager;
    QMenu* mpRecentMenu;
    QMenu* mpWindowMenu;
    PanelEditor* mpPanelEditor;
    QMap<PropertyType, PropertiesEditor*> mPropertiesEditors;
    PropertiesViewer* mpPropertiesViewer;
    OptionsEditor* mpOptionsEditor;
    ConvergencePlot* mpConvergencePlot;
    GeometryPlot* mpGeometryPlot;
    SolutionBrowser* mpSolutionBrowser;
    bool mIsSolverRunning;
    QAction* mpStartSolverAction;
    QAction* mpStopSolverAction;

    // Project
    Backend::Project mProject;
};

class SolveThread : public QThread
{
    Q_OBJECT

public:
    SolveThread(Backend::Project project, QObject* pParent = nullptr);
    void run() override;

signals:
    void iterationFinished(Backend::Optimizer::Solution solution);
    void resultReady(QList<Backend::Optimizer::Solution> solutions);
    void log(QString message);

private:
    Backend::Project mProject;
};

void logMessage(QtMsgType type, QMessageLogContext const& /*context*/, QString const& message);
}

#endif // MAINWINDOW_H

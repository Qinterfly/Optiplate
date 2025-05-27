/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the MainWindow class
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

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
class OptionsEditor;
class PanelEditor;
class PropertiesViewer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* pParent = nullptr);
    ~MainWindow();
    static Logger* pLogger;

    QSettings& settings();

    // File interaction
    void newProject();
    bool openProject(QString const& pathFile);
    void saveProject();
    void saveAsProject(QString const& pathFile);

private:
    void initializeWindow();
    void closeEvent(QCloseEvent* pEvent) override;

    // Content
    void createContent();
    void createFileActions();
    void createWindowActions();
    void createDockManager();
    ads::CDockWidget* createPanelEditor();
    ads::CDockWidget* createPropertiesEditor(QString const& name, PropertyType type, Backend::Properties& properties);
    ads::CDockWidget* createOptionsEditor();
    ads::CDockWidget* createPropertiesViewer();
    ads::CDockWidget* createConvergencePlot();
    ads::CDockWidget* createLogger();
    void createConnections();

    // State
    void setProjectTitle();
    void setModified(bool flag);
    void setTheme();
    void processProjectChange();

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

    // Project
    Backend::Project mProject;
};

void logMessage(QtMsgType type, const QMessageLogContext& /*context*/, const QString& message);
}

#endif // MAINWINDOW_H

/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the MainWindow class
 */

#include <DockAreaWidget.h>
#include <DockManager.h>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMenuBar>
#include <QMessageBox>
#include <QThread>
#include <QToolBar>

#include "config.h"
#include "convergenceplot.h"
#include "geometryplot.h"
#include "logger.h"
#include "mainwindow.h"
#include "optionseditor.h"
#include "paneleditor.h"
#include "propertiesviewer.h"
#include "solutionbrowser.h"
#include "uiconstants.h"
#include "uiutility.h"

using namespace ads;
using namespace Frontend;

Logger* MainWindow::pLogger = nullptr;
QString MainWindow::language = "en";

void switchTranslator(QTranslator& translator, QString const& fileName, QString const& language, bool isResource);

MainWindow::MainWindow(QWidget* pParent)
    : QMainWindow(pParent)
    , mSettings(Constants::Settings::skFileName, QSettings::IniFormat)
    , mIsSolverRunning(false)
{
    initializeWindow();
    createContent();
    createConnections();
    restoreSettings();
    newProject();
}

MainWindow::~MainWindow()
{

}

//! Set a state and geometry of the main window
void MainWindow::initializeWindow()
{
    setWindowState(Qt::WindowMaximized);
    setWindowTitle(QString(APP_NAME) + "[*]");
    setTheme();
    setLanguage();
    qInstallMessageHandler(Frontend::logMessage);

    // Since OpenGL widgets tend to change the window geometry, we set the maximized state manually
    Utility::fullScreenResize(this);
}

//! Save project and settings before exit
void MainWindow::closeEvent(QCloseEvent* pEvent)
{
    bool isClose = stopSolverDialog() && saveProjectChangesDialog();
    if (isClose)
    {
        saveSettings();
        pEvent->accept();
    }
    else
    {
        pEvent->ignore();
    }
}

//! Create all the widgets and corresponding actions
void MainWindow::createContent()
{
    // Top widgets
    createFileActions();
    createWindowActions();
    createSolverActions();
    createHelpActions();

    // Manager to place dockable widgets
    createDockManager();

    // Panel editor
    ads::CDockWidget* pWidget = createPanelEditor();
    mpDockManager->addDockWidget(ads::TopDockWidgetArea, pWidget);
    mpWindowMenu->addSeparator();

    // Properties editors
    pWidget = createPropertiesEditor(tr("Target"), PropertyType::kTarget, mProject.configuration().target);
    ads::CDockAreaWidget* pArea = mpDockManager->addDockWidget(ads::BottomDockWidgetArea, pWidget);
    pWidget = createPropertiesEditor(tr("Weight"), PropertyType::kWeight, mProject.configuration().weight);
    mpDockManager->addDockWidget(ads::CenterDockWidgetArea, pWidget, pArea);

    // Options editor
    pWidget = createOptionsEditor();
    mpDockManager->addDockWidget(ads::CenterDockWidgetArea, pWidget, pArea);
    pArea->setCurrentIndex(0);
    mpWindowMenu->addSeparator();

    // Properties viewer
    pWidget = createPropertiesViewer();
    mpDockManager->addDockWidget(ads::BottomDockWidgetArea, pWidget);

    // Solution browser
    pWidget = createSolutionBrowser();
    mpDockManager->addDockWidget(ads::RightDockWidgetArea, pWidget);

    // Convergence plot
    pWidget = createConvergencePlot();
    pArea = mpDockManager->addDockWidget(ads::RightDockWidgetArea, pWidget);

    // Geometry plot
    pWidget = createGeometryPlot();
    pArea = mpDockManager->addDockWidget(ads::CenterDockWidgetArea, pWidget, pArea);
    pArea->setCurrentIndex(0);

    // Logger
    pWidget = createLogger();
    mpDockManager->addDockWidget(ads::BottomDockWidgetArea, pWidget, pArea);
}

//! Create the actions to interact with files
void MainWindow::createFileActions()
{
    // Create the actions
    QAction* pNewAction = new QAction(tr("&New Project"), this);
    QAction* pOpenProjectAction = new QAction(tr("&Open Project..."), this);
    QAction* pSaveAction = new QAction(tr("&Save"), this);
    QAction* pSaveAsAction = new QAction(tr("&Save As..."), this);
    QAction* pExitAction = new QAction(tr("E&xit"), this);

    // Set the icons
    pNewAction->setIcon(QIcon(":/icons/document-new.svg"));
    pOpenProjectAction->setIcon(QIcon(":/icons/document-open.svg"));
    pSaveAction->setIcon(QIcon(":/icons/document-save.svg"));
    pSaveAsAction->setIcon(QIcon(":/icons/document-save-as.svg"));

    // Set the shortcuts
    pNewAction->setShortcut(QKeySequence::New);
    pOpenProjectAction->setShortcut(QKeySequence::Open);
    pSaveAction->setShortcut(QKeySequence::Save);
    pSaveAsAction->setShortcut(QKeySequence::SaveAs);
    pExitAction->setShortcut(QKeySequence::Quit);

    // Create the menu
    QMenu* pFileMenu = new QMenu(tr("&File"), this);
    mpRecentMenu = new QMenu(tr("Recent P&rojects"), this);
    pFileMenu->setFont(font());

    // Connect the actions
    connect(pNewAction, &QAction::triggered, this, &MainWindow::newProjectDialog);
    connect(pOpenProjectAction, &QAction::triggered, this, &MainWindow::openProjectDialog);
    connect(pSaveAction, &QAction::triggered, this, &MainWindow::saveProject);
    connect(pSaveAsAction, &QAction::triggered, this, &MainWindow::saveAsProjectDialog);
    connect(pExitAction, &QAction::triggered, qApp, &QApplication::quit);

    // Fill up the menu
    pFileMenu->addAction(pNewAction);
    pFileMenu->addAction(pOpenProjectAction);
    pFileMenu->addMenu(mpRecentMenu);
    pFileMenu->addSeparator();
    pFileMenu->addAction(pSaveAction);
    pFileMenu->addAction(pSaveAsAction);
    pFileMenu->addSeparator();
    pFileMenu->addAction(pExitAction);
    menuBar()->addMenu(pFileMenu);

    // Create the toolbar
    QToolBar* pFileToolBar = new QToolBar;
    pFileToolBar->setIconSize(Constants::Size::skToolBarIcon);
    pFileToolBar->addAction(pNewAction);
    pFileToolBar->addAction(pOpenProjectAction);
    pFileToolBar->addSeparator();
    pFileToolBar->addAction(pSaveAction);
    pFileToolBar->addAction(pSaveAsAction);
    Utility::setShortcutHints(pFileToolBar);
    addToolBar(pFileToolBar);
}

//! Create the actions to customize windows
void MainWindow::createWindowActions()
{
    mpWindowMenu = new QMenu(tr("&Window"), this);
    mpWindowMenu->setFont(font());
    menuBar()->addMenu(mpWindowMenu);
}

//! Create the actions to run the solver
void MainWindow::createSolverActions()
{
    // Create the actions
    mpStartSolverAction = new QAction(tr("&Start the solver"), this);
    mpStopSolverAction = new QAction(tr("&Stop the solver"), this);
    QAction* pClearAction = new QAction(tr("&Clear results"), this);

    // Set the shortcuts
    mpStartSolverAction->setShortcut(Qt::Key_F5);
    mpStopSolverAction->setShortcut(Qt::SHIFT | Qt::Key_F5);

    // Set the icons
    mpStartSolverAction->setIcon(QIcon(":/icons/process-start.svg"));
    mpStopSolverAction->setIcon(QIcon(":/icons/process-stop.svg"));
    pClearAction->setIcon(QIcon(":/icons/edit-delete.svg"));

    // Set the connections
    connect(mpStartSolverAction, &QAction::triggered, this, &MainWindow::startSolver);
    connect(mpStopSolverAction, &QAction::triggered, this, &MainWindow::stopSolver);
    connect(pClearAction, &QAction::triggered, this, &MainWindow::clearResultsDialog);

    // Create the menu
    QMenu* pMenu = new QMenu(tr("&Solver"), this);
    pMenu->setFont(font());
    pMenu->addAction(mpStartSolverAction);
    pMenu->addAction(mpStopSolverAction);
    pMenu->addAction(pClearAction);
    menuBar()->addMenu(pMenu);

    // Create the toolbar
    QToolBar* pToolBar = new QToolBar;
    pToolBar->setIconSize(Constants::Size::skToolBarIcon);
    pToolBar->addAction(mpStartSolverAction);
    pToolBar->addAction(mpStopSolverAction);
    pToolBar->addAction(pClearAction);
    Utility::setShortcutHints(pToolBar);
    addToolBar(pToolBar);
    updateSolverActions();
}

//! Create the actions to show the program info
void MainWindow::createHelpActions()
{
    // Create the actions
    QAction* pAboutAction = new QAction(tr("&About"), this);
    QAction* pAboutQtAction = new QAction(tr("&About Qt"), this);

    // Connect the actions
    connect(pAboutAction, &QAction::triggered, this, &MainWindow::about);
    connect(pAboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);

    // Fill up the menu
    QMenu* pHelpMenu = new QMenu(tr("&Help"), this);
    pHelpMenu->setFont(font());
    pHelpMenu->addAction(pAboutAction);
    pHelpMenu->addAction(pAboutQtAction);
    menuBar()->addMenu(pHelpMenu);
}

//! Create the dock manager and specify its configuration
void MainWindow::createDockManager()
{
    ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
    ads::CDockManager::setAutoHideConfigFlags({ads::CDockManager::DefaultAutoHideConfig});
    mpDockManager = new ads::CDockManager(this);
}

//! Create a widget to represent a panel data
ads::CDockWidget* MainWindow::createPanelEditor()
{
    // Create the widget to edit panel data
    mpPanelEditor = new PanelEditor(mProject.panel());

    // Construct the dock widget
    ads::CDockWidget* pDockWidget = new CDockWidget(mpDockManager, tr("Panel"));
    pDockWidget->setWidget(mpPanelEditor);
    mpWindowMenu->addAction(pDockWidget->toggleViewAction());
    return pDockWidget;
}

//! Create a widget to represent inertia properties
ads::CDockWidget* MainWindow::createPropertiesEditor(QString const& name, PropertyType type, Backend::Properties& properties)
{
    // Create the widget to edit properties
    mPropertiesEditors[type] = new PropertiesEditor(type, properties);

    // Construct the dock widget
    ads::CDockWidget* pDockWidget = new CDockWidget(mpDockManager, name);
    pDockWidget->setWidget(mPropertiesEditors[type]);
    mpWindowMenu->addAction(pDockWidget->toggleViewAction());
    return pDockWidget;
}

//! Create a widget to represent optimization options
ads::CDockWidget* MainWindow::createOptionsEditor()
{
    // Create the widget to browse options
    mpOptionsEditor = new OptionsEditor(mProject.configuration().options);

    // Construct the dock widget
    ads::CDockWidget* pDockWidget = new CDockWidget(mpDockManager, tr("Options"));
    pDockWidget->setWidget(mpOptionsEditor);
    mpWindowMenu->addAction(pDockWidget->toggleViewAction());
    return pDockWidget;
}

//! Create a widget to represent current properties
ads::CDockWidget* MainWindow::createPropertiesViewer()
{
    // Create the widget to view and compare properties
    mpPropertiesViewer = new PropertiesViewer;

    // Construct the dock widget
    ads::CDockWidget* pDockWidget = new CDockWidget(mpDockManager, tr("Inertia Properties"));
    pDockWidget->setWidget(mpPropertiesViewer);
    mpWindowMenu->addAction(pDockWidget->toggleViewAction());
    return pDockWidget;
}

//! Create a widget to represnt solutions
ads::CDockWidget* MainWindow::createSolutionBrowser()
{
    // Create the widget to view the solutions
    mpSolutionBrowser = new SolutionBrowser;

    // Construct the dock widget
    ads::CDockWidget* pDockWidget = new CDockWidget(mpDockManager, tr("Solution"));
    pDockWidget->setWidget(mpSolutionBrowser);
    mpWindowMenu->addAction(pDockWidget->toggleViewAction());
    return pDockWidget;
}

//! Create a widget to represent convergence
ads::CDockWidget* MainWindow::createConvergencePlot()
{
    // Create the widget to view and compare properties
    mpConvergencePlot = new ConvergencePlot;

    // Construct the dock widget
    ads::CDockWidget* pDockWidget = new CDockWidget(mpDockManager, tr("Convergence"));
    pDockWidget->setWidget(mpConvergencePlot);
    mpWindowMenu->addAction(pDockWidget->toggleViewAction());
    return pDockWidget;
}

//! Create a widget to represent panel geometry
ads::CDockWidget* MainWindow::createGeometryPlot()
{
    // Create the widget to view panel geometry
    mpGeometryPlot = new GeometryPlot;

    // Construct the dock widget
    ads::CDockWidget* pDockWidget = new CDockWidget(mpDockManager, tr("Geometry"));
    pDockWidget->setWidget(mpGeometryPlot);
    mpWindowMenu->addAction(pDockWidget->toggleViewAction());
    return pDockWidget;
}

//! Create a widget to log all events
ads::CDockWidget* MainWindow::createLogger()
{
    // Create the logger
    if (!pLogger)
        pLogger = new Logger;

    // Create a dock widget
    ads::CDockWidget* pDockWidget = new ads::CDockWidget(mpDockManager, tr("Log"));
    pDockWidget->setWidget(pLogger);
    mpWindowMenu->addAction(pDockWidget->toggleViewAction());
    return pDockWidget;
}

//! Connect the widgets between each other
void MainWindow::createConnections()
{
    // Slot functions
    auto updateWidgets = [this](Backend::Optimizer::Solution solution = Backend::Optimizer::Solution())
    {
        if (solution.isValid())
        {
            mpPropertiesViewer->update(solution.properties, mProject.configuration());
            mpGeometryPlot->plot(mProject.panel(), solution);
        }
        else
        {
            mpPropertiesViewer->update(mProject.panel(), mProject.configuration());
            mpGeometryPlot->plot(mProject.panel(), solution);
        }
    };
    auto viewPanel = [this](Backend::Optimizer::Solution solution)
    {
        Backend::Panel panel = solution.panel;
        PanelEditor* pEditor = new PanelEditor(panel);
        ads::CDockWidget* pDockWidget = new CDockWidget(mpDockManager, tr("Panel (Iteration: %1)").arg(QString::number(solution.iteration)));
        pDockWidget->setWidget(pEditor);
        mpDockManager->addDockWidgetFloating(pDockWidget);
    };
    auto setPanel = [this](Backend::Optimizer::Solution solution)
    {
        mProject.panel() = solution.panel;
        mpPanelEditor->update();
        mpPropertiesViewer->update(mProject.panel(), mProject.configuration());
        mpGeometryPlot->plot(mProject.panel());
        qInfo() << tr("The panel data has been substituted");
    };

    // Panel
    connect(mpPanelEditor, &PanelEditor::dataChanged, this, &MainWindow::processProjectChange);
    connect(mpPanelEditor, &PanelEditor::dataChanged, this, updateWidgets);

    // Properties
    for (auto [key, value] : mPropertiesEditors.asKeyValueRange())
    {
        connect(value, &PropertiesEditor::dataChanged, this, &MainWindow::processProjectChange);
        connect(value, &PropertiesEditor::dataChanged, this, updateWidgets);
    }

    // Solutions
    connect(mpSolutionBrowser, &SolutionBrowser::solutionSelected, this, updateWidgets);
    connect(mpSolutionBrowser, &SolutionBrowser::viewPanelRequested, this, viewPanel);
    connect(mpSolutionBrowser, &SolutionBrowser::setPanelRequested, this, setPanel);
}

//! Close the current project and create a new one
void MainWindow::newProject()
{
    mProject = Backend::Project(tr("New"));
    qInfo() << tr("New project was created");
    setModified(false);
    mpPanelEditor->update();
    for (auto [key, value] : mPropertiesEditors.asKeyValueRange())
        value->update();
    mpOptionsEditor->update();
    mpPropertiesViewer->clear();
    mpConvergencePlot->clear();
}

//! Read the project located at the specified path
bool MainWindow::openProject(QString const& pathFile)
{
    if (mProject.read(pathFile))
    {
        qInfo() << tr("Project %1 was successfully opened").arg(pathFile);
        setModified(false);
        addToRecentProjects();
        mpPanelEditor->update();
        for (auto [key, value] : mPropertiesEditors.asKeyValueRange())
            value->update();
        mpOptionsEditor->update();
        mpPropertiesViewer->update(mProject.panel(), mProject.configuration());
        mpSolutionBrowser->update(mProject.solutions(), mProject.configuration().options);
        mpConvergencePlot->plot(mProject.solutions(), mProject.configuration().options);
        return true;
    }
    return false;
}

//! Save the project at the last used path
void MainWindow::saveProject()
{
    QString const& lastPath = mProject.pathFile();
    if (lastPath.isEmpty())
    {
        saveAsProjectDialog();
        return;
    }
    if (mProject.write(lastPath))
    {
        qInfo() << tr("The project was saved using the previous location: %1").arg(lastPath);
        setModified(false);
    }
}

//! Save the project using the path specified
void MainWindow::saveAsProject(QString const& pathFile)
{
    if (mProject.write(pathFile))
    {
        qInfo() << tr("The project was saved as the following file %1").arg(pathFile);
        addToRecentProjects();
        setModified(false);
    }
}

//! Open the project which was selected from the Recent Projects menu
void MainWindow::openRecentProject()
{
    if (!saveProjectChangesDialog())
        return;
    QAction* pAction = (QAction*) sender();
    if (pAction)
        openProject(pAction->text());
}

//! Represent the project name
void MainWindow::setProjectTitle()
{
    QString const& pathFile = mProject.pathFile();
    if (pathFile.isEmpty())
        setWindowTitle(QString("%1[*]").arg(APP_NAME));
    else
        setWindowTitle(QString("%1: %2[*]").arg(APP_NAME, pathFile));
}

//! Specify the application language based on its locale
void MainWindow::setLanguage()
{
    // Check if the requested language can be provided
    if (language != "en" && language != "ru")
        language = "ru";

    // Force the application locale
    setLocale(QLocale::C);
    QLocale::setDefault(locale());

    // Assign the translators
    switchTranslator(mTranslatorApplication, "application", language, true);
    switchTranslator(mTranslatorQt, "qt", language, false);
}

//! Whenever a project has been modified
void MainWindow::setModified(bool flag)
{
    setWindowModified(flag);
    setProjectTitle();
}

//! Set the application font, icon and stylesheet
void MainWindow::setTheme()
{
    // Font
    QFontDatabase::addApplicationFont(":/fonts/Roboto.ttf");
    QFontDatabase::addApplicationFont(":/fonts/RobotoMono.ttf");
    uint fontSize = 12;
#ifdef Q_OS_WIN
    fontSize = 10;
#endif
    QFont font("Roboto", fontSize);
    setFont(font);
    qApp->setFont(font);
    menuBar()->setFont(font);

    // Icon
    qApp->setWindowIcon(QIcon(":/icons/application.svg"));

    // Style
    qApp->setStyle("Fusion");
    QFile file(":/styles/modern.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);
}

//! Process change in project data
void MainWindow::processProjectChange()
{
    setModified(true);
}

//! Run the optimizer
void MainWindow::startSolver()
{
    // Check if the solver is alreay started
    if (mIsSolverRunning)
    {
        qWarning() << tr("The solver is already running");
        return;
    }

    // Update the state
    mIsSolverRunning = true;
    mProject.clearSolutions();
    updateSolverActions();
    mpPanelEditor->setEnabled(false);

    // Clear the tread
    SolveThread* pThread = new SolveThread(mProject, this);

    // Set the  connections
    connect(pThread, &SolveThread::iterationFinished, this,
            [this](Backend::Optimizer::Solution solution)
            {
                mProject.addSolution(solution);
                mpConvergencePlot->plot(mProject.solutions(), mProject.configuration().options);
                mpSolutionBrowser->update(mProject.solutions(), mProject.configuration().options);
                setModified(true);
            });
    connect(pThread, &SolveThread::resultReady, this,
            [this](QList<Backend::Optimizer::Solution> solutions)
            {
                mIsSolverRunning = false;
                mProject.setSolutions(solutions);
                updateSolverActions();
                mpSolutionBrowser->update(mProject.solutions(), mProject.configuration().options);
                mpPanelEditor->setEnabled(true);
                setModified(true);
                qInfo() << tr("Opimization process finished");
            });
    connect(pThread, &SolveThread::finished, pThread,
            [this, pThread]()
            {
                mIsSolverRunning = false;
                pThread->deleteLater();
                updateSolverActions();
                mpSolutionBrowser->update(mProject.solutions(), mProject.configuration().options);
                mpPanelEditor->setEnabled(true);
                setModified(true);
            });
    connect(this, &MainWindow::stopSolverRequested, pThread,
            [this, pThread]()
            {
                mIsSolverRunning = false;
                qWarning() << tr("Stop solver requested");
                pThread->requestInterruption();
                updateSolverActions();
                mpSolutionBrowser->update(mProject.solutions(), mProject.configuration().options);
                mpPanelEditor->setEnabled(true);
            });

    // Start the solver
    qInfo() << tr("Starting the optimization process...");
    pThread->start();
}

//! Stop the optimizer
void MainWindow::stopSolver()
{
    if (!mIsSolverRunning)
    {
        qWarning() << tr("The solver is not running");
        return;
    }
    emit stopSolverRequested();
}

void MainWindow::updateSolverActions()
{
    mpStartSolverAction->setVisible(!mIsSolverRunning);
    mpStopSolverAction->setVisible(mIsSolverRunning);
}

//! Retrieve recent projects from the settings file
void MainWindow::retrieveRecentProjects()
{
    QList<QVariant> listSettingsProjects = mSettings.value(Constants::Settings::skRecent).toList();
    mPathRecentProjects.clear();
    mpRecentMenu->clear();
    QString pathProject;
    QList<QVariant> updatedPaths;
    int numRecentProjects = listSettingsProjects.size();
    for (int i = 0; i != numRecentProjects; ++i)
    {
        QVariant const& varPath = listSettingsProjects[i];
        pathProject = varPath.toString();
        if (QFileInfo::exists(pathProject))
        {
            updatedPaths.push_back(pathProject);
            QAction* pAction = mpRecentMenu->addAction(pathProject);
            connect(pAction, &QAction::triggered, this, &MainWindow::openRecentProject);
            mPathRecentProjects.push_back(pathProject);
        }
    }
    mSettings.setValue(Constants::Settings::skRecent, updatedPaths);
}

//! Add the current project to the recent ones
void MainWindow::addToRecentProjects()
{
    QString const& pathFile = mProject.pathFile();
    if (!pathFile.isEmpty())
    {
        if (!mPathRecentProjects.contains(pathFile))
            mPathRecentProjects.push_back(pathFile);
        while (mPathRecentProjects.count() > Constants::Size::skMaxRecentProjects)
            mPathRecentProjects.pop_front();
        mpRecentMenu->clear();
        QList<QVariant> listSettingsProjects;
        int numRecentProjects = mPathRecentProjects.size();
        for (int i = 0; i != numRecentProjects; ++i)
        {
            QString const& path = mPathRecentProjects[i];
            listSettingsProjects.push_back(path);
            QAction* pAction = mpRecentMenu->addAction(path);
            connect(pAction, &QAction::triggered, this, &MainWindow::openRecentProject);
        }
        mSettings.setValue(Constants::Settings::skRecent, listSettingsProjects);
    }
}

//! Save window settings to a file
void MainWindow::saveSettings()
{
    mSettings.beginGroup(Constants::Settings::skMainWindow);
    mSettings.setValue(Constants::Settings::skLanguage, MainWindow::language);
    mSettings.setValue(Constants::Settings::skGeometry, saveGeometry());
    mSettings.setValue(Constants::Settings::skState, saveState());
    mSettings.setValue(Constants::Settings::skDockingState, mpDockManager->saveState());
    mSettings.endGroup();
    if (mSettings.status() == QSettings::NoError)
        qInfo() << tr("Settings were written to the file %1").arg(Constants::Settings::skFileName);
}

//! Restore window settings from a file
void MainWindow::restoreSettings()
{
    if (mSettings.allKeys().empty())
        return;
    mSettings.beginGroup(Constants::Settings::skMainWindow);
    QString lang = mSettings.value(Constants::Settings::skLanguage).toString();
    if (lang == language)
    {
        bool isOk = restoreGeometry(mSettings.value(Constants::Settings::skGeometry).toByteArray())
                    && restoreState(mSettings.value(Constants::Settings::skState).toByteArray())
                    && mpDockManager->restoreState(mSettings.value(Constants::Settings::skDockingState).toByteArray());
        if (isOk)
            qInfo() << tr("Settings were restored from the file %1").arg(Constants::Settings::skFileName);
    }
    mSettings.endGroup();
    retrieveRecentProjects();
}

//! Create the new project using the dialog, if necessary
void MainWindow::newProjectDialog()
{
    if (!saveProjectChangesDialog())
        return;
    newProject();
}

//! Read the project using the file dialog
void MainWindow::openProjectDialog()
{
    static QString const kExpectedSuffix = Backend::Project::fileSuffix();

    if (!saveProjectChangesDialog())
        return;

    // Create the file dialog
    QString pathFile = QFileDialog::getOpenFileName(this, tr("Open Project"), mProject.pathFile(),
                                                    tr("Project file format (*%1)").arg(kExpectedSuffix));
    if (pathFile.isEmpty())
        return;
    openProject(pathFile);
}

//! Save the project using the file dialog
void MainWindow::saveAsProjectDialog()
{
    static QString const kExpectedSuffix = Backend::Project::fileSuffix();

    QString pathFile = QFileDialog::getSaveFileName(this, tr("Save Project"), mProject.pathFile(),
                                                    tr("Project file format (*%1)").arg(kExpectedSuffix));
    if (pathFile.isEmpty())
        return;

    // Modify the suffix, if necessary
    Utility::modifyFileSuffix(pathFile, kExpectedSuffix);

    // Save the project
    saveAsProject(pathFile);
}

//! Save project changes through dialog
bool MainWindow::saveProjectChangesDialog()
{
    QString const title = tr("Save project changes");
    QString const message = tr("The project containes unsaved changes.\n"
                               "Would you like to save it?");
    if (isWindowModified())
    {
        int iResult = Utility::showSaveDialog(this, title, message);
        if (iResult < 0)
            return false;
        if (iResult == 1)
            saveProject();
    }
    return true;
}

//! Stop the solver through dialog
bool MainWindow::stopSolverDialog()
{
    QString const title = tr("Stop the solver");
    QString const message = tr("The solver is still running.\n"
                               "Would you like to close the application anyway?");
    if (mIsSolverRunning)
    {
        QMessageBox* pMessageBox = new QMessageBox(QMessageBox::Question, title, message, QMessageBox::NoButton, this);
        pMessageBox->setFont(font());
        auto result = pMessageBox->exec();
        if (result == QMessageBox::Yes)
            stopSolver();
        else
            return false;
    }
    return true;
}

//! Clear the project results
void MainWindow::clearResultsDialog()
{
    QString const title = tr("Clear the results");
    QString const message = tr("Would you like to clear the results?");

    // Check if the solver is still running
    if (mIsSolverRunning)
    {
        qWarning() << tr("The solver is still running. Wait till the results obtained");
        return;
    }

    // Check if there are any results to remove
    if (mProject.solutions().empty())
    {
        qWarning() << tr("There are no results to clear");
        return;
    }

    // Create the dialog window
    QMessageBox* pMessageBox = new QMessageBox(QMessageBox::Question, title, message, QMessageBox::Yes | QMessageBox::No, this);
    pMessageBox->setFont(font());
    auto result = pMessageBox->exec();
    if (result == QMessageBox::Yes)
    {
        mProject.clearSolutions();
        mpSolutionBrowser->update(mProject.solutions(), mProject.configuration().options);
        mpConvergencePlot->clear();
        qInfo() << tr("The results have been cleared");
    }
}

//! Show information about the program
void MainWindow::about()
{
    QString const build = QStringLiteral(__DATE__) + QStringLiteral(" ") + QStringLiteral(__TIME__);
    QString const author = tr("Pavel Lakiza");
    QString const message = tr("%1 is a program which optimizes mechanical and geometrical properties of panels\n\n"
                               "Built on %3\n\n"
                               "Copyright %1 (%2)")
                                .arg(APP_NAME, author, build);
    QString const title = tr("About %1 v%2").arg(APP_NAME, VERSION_FULL);
    QMessageBox::about(this, title, message);
}

SolveThread::SolveThread(Backend::Project project, QObject* pParent)
    : QThread(pParent)
    , mProject(project)
{
}

//! Run the optimizer
void SolveThread::run()
{
    // Configure the solver
    Backend::Configuration const& config = mProject.configuration();
    Backend::Optimizer optimizer(config.state, config.target, config.weight, config.options);

    // Set up the connections
    connect(&optimizer, &Backend::Optimizer::iterationFinished, this, &SolveThread::iterationFinished);
    connect(&optimizer, &Backend::Optimizer::log, this, [](QString message) { logMessage(QtMsgType::QtInfoMsg, QMessageLogContext(), message); });

    // Run the solver
    auto solutions = optimizer.solve(mProject.panel());
    emit resultReady(solutions);
}

//! Helper function to log all the messages
void Frontend::logMessage(QtMsgType type, QMessageLogContext const& /*context*/, QString const& message)
{
    if (Frontend::MainWindow::pLogger)
        Frontend::MainWindow::pLogger->log(type, message);
}

//! Helper function to switch translators
void switchTranslator(QTranslator& translator, QString const& fileName, QString const& language, bool isResource)
{
    // Remove the old translator
    qApp->removeTranslator(&translator);

    // Load the new translator
    QString baseDir = isResource ? ":" : QApplication::applicationDirPath();
    QString dir = baseDir + "/translations";
    QString pathFile = QString("%1/%2_%3.qm").arg(dir, fileName, language);
    if (translator.load(pathFile))
        qApp->installTranslator(&translator);
}

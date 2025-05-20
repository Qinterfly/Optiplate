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

namespace ads
{
class CDockWidget;
class CDockManager;
}

namespace Frontend
{

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* pParent = nullptr);
    ~MainWindow();

    QSettings& settings();

private:
    void initializeWindow();

private:
    QSettings mSettings;

    // Project
    Backend::Project mProject;
};

}

#endif // MAINWINDOW_H

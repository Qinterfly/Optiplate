/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the MainWindow class
 */

#include <DockManager.h>

#include "mainwindow.h"

using namespace Frontend;

QString const skMainWindow = "mainWindow";

MainWindow::MainWindow(QWidget* pParent)
    : QMainWindow(pParent)
    , mSettings(skMainWindow, QSettings::IniFormat)
{
    initializeWindow();
}

MainWindow::~MainWindow()
{

}

void MainWindow::initializeWindow()
{
    // TODO
}

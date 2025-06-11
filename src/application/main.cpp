/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief The startup function
 */

#include <QApplication>
#include <config.h>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    // Force the light theme
#ifdef Q_OS_WIN
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");
#endif

    // Create the application
    QApplication application(argc, argv);

    // Set up the language
    QString lang = QLocale::system().name();
    lang.truncate(lang.lastIndexOf('_'));
    Frontend::MainWindow::language = lang;

    // Create the graphical window
    Frontend::MainWindow window;
    window.show();

    return application.exec();
}

/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the TestFrontend class
 */

#ifndef TESTFRONTEND_H
#define TESTFRONTEND_H

#include <QTest>

#include "mainwindow.h"

namespace Frontend
{
class MainWindow;
}

namespace Tests
{

class TestFrontend : public QObject
{
    Q_OBJECT

public:
    TestFrontend();
    virtual ~TestFrontend();

private slots:
    void testOpenProject();
    void testSolveProject();

private:
    Frontend::MainWindow* mpMainWindow;

};

}

#endif // TESTFRONTEND_H

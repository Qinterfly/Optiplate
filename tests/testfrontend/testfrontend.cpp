/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the TestFrontend class
 */

#include <config.h>

#include "fileutility.h"
#include "testfrontend.h"

using namespace Tests;
using namespace Frontend;
using namespace Backend;

TestFrontend::TestFrontend()
{
    mpMainWindow = new MainWindow;
}

//! Open a project
void TestFrontend::testOpenProject()
{
    QString fileName = QString("real.%1").arg(Project::fileSuffix());
    QString pathFile = Utility::combineFilePath(EXAMPLES_DIR, fileName);
    QVERIFY(mpMainWindow->openProject(pathFile));
    mpMainWindow->show();
}

//! Solve the project
void TestFrontend::testSolveProject()
{
    // mpMainWindow->startSolver();
}

TestFrontend::~TestFrontend()
{
    QTest::qWait(30000);
    delete mpMainWindow;
}

QTEST_MAIN(TestFrontend)

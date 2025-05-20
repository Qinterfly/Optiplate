/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the TestFrontend class
 */

#include <config.h>

#include "testfrontend.h"

using namespace Tests;
using namespace Frontend;

TestFrontend::TestFrontend()
{
    mpMainWindow = new MainWindow();
}

//! Open a project consisted of several examples
void TestFrontend::testOpenProject()
{
    // TODO
}

TestFrontend::~TestFrontend()
{
    QTest::qWait(30000);
    delete mpMainWindow;
}

QTEST_MAIN(TestFrontend)

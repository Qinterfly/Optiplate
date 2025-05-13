/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the TestBackend class
 */

#include <QFileInfo>
#include <config.h>

#include "testbackend.h"

#include "optimizer.h"
#include "panel.h"

using namespace Tests;
using namespace Backend;
using namespace KCL;

static int const skNumDirections = 3;

TestBackend::TestBackend()
{

}

//! Build a panel associated with real data
void TestBackend::testCreateRealPanel()
{
    double const precision = 1e-4;

    // Create a panel
    mRealPanel.setThickness(0.0);
    mRealPanel.setXCoords({-0.73, -0.64, 0.43, 0.35});
    mRealPanel.setZCoords({0, 0.98, 0.98, 0});
    mRealPanel.setDepths({0.201, 0.16, 0.153});
    mRealPanel.setDensity(0.057855);

    // Define analytical properties
    Vec3 centerGravity = {-0.152764, 0, 0.47034};

    // Evaluate inertia properties
    auto props = mRealPanel.massProperties();
    QVERIFY(isEqual(props.mass, 0.01079, precision));
    for (int i = 0; i != skNumDirections; ++i)
        QVERIFY(isEqual(props.centerGravity[i], centerGravity[i], precision));
}

//! Update the real panel
void TestBackend::testUpdateRealPanel()
{
    // Set the optimization state
    Optimizer::State state;

    // Set the targets
    Optimizer::Target target;
    target.mass = 0.01079;
    target.centerGravity = {-0.155, 0, 0.519};
    target.inertiaMoments = {0.00077, 0.00176, 0.00107};
    target.inertiaProducts = {0, 0, 0.00001};

    // Assign the weights
    Optimizer::Weight weight;
    // weight.inertiaMoments.fill(0.0);
    // weight.inertiaProducts.fill(0.0);
    // weight.inertiaProducts[2] = 1.0;

    // Select the options
    Optimizer::Options options;
    options.autoScale = false;
    options.numIterations = 200;
    options.numThreads = 1;

    // Run the solver
    Optimizer optimizer(state, target, weight, options);
    optimizer.run(mRealPanel);
}

//! Create a rectangular panel and verify its inertia properties
void TestBackend::testCreateBasePanel()
{
    double const precision = 1e-4;

    // Set panel properties
    double width = 2;
    double height = 1;
    double depth = 1e-3;
    double density = 0.5;
    mBasePanel.setThickness(0.0);
    mBasePanel.setXCoords({0, 0, width, width});
    mBasePanel.setZCoords({0, height, height, 0});
    mBasePanel.setDepths({depth, depth, depth});
    mBasePanel.setDensity(density);

    // Compute analytical properties
    double mass = density * width * height * depth;
    Vec3 centerGravity = {width / 2, 0, height / 2};
    double inertiaMomentX = mass / 12 * (qPow(height, 2.0) + qPow(depth, 2.0));
    double inertiaMomentY = mass / 12 * (qPow(width, 2.0) + qPow(height, 2.0));
    double inertiaMomentZ = mass / 12 * (qPow(width, 2.0) + qPow(depth, 2.0));
    Vec3 inertiaMoments = {inertiaMomentX, inertiaMomentY, inertiaMomentZ};

    // Evaluate inertia properties
    auto props = mBasePanel.massProperties();
    QVERIFY(isEqual(props.mass, mass, precision));
    for (int i = 0; i != skNumDirections; ++i)
    {
        QVERIFY(isEqual(props.centerGravity[i], centerGravity[i], precision));
        QVERIFY(isEqual(props.inertiaMoments[i], inertiaMoments[i], precision));
        QVERIFY(isEqual(props.inertiaProducts[i], 0.0, precision));
    }
}

//! Update the base panel
void TestBackend::testUpdateBasePanel()
{
    // Panel initPanel = mBasePanel;

    // // Set up the target panel
    // Panel targetPanel;
    // double width = 2.01;
    // double height = 1;
    // double depth = 1e-3;
    // double density = 0.5;
    // targetPanel.setThickness(0.0);
    // targetPanel.setXCoords({0, 0, width, width});
    // targetPanel.setZCoords({0, height, height, 0});
    // targetPanel.setDepths({depth, depth, depth});
    // targetPanel.setDensity(density);
    // auto targetProps = targetPanel.massProperties();

    // // Set the optimization state
    // Optimizer::State state;

    // // Set the targets
    // Optimizer::Target target;
    // target.mass = targetProps.mass;
    // target.centerGravity = targetProps.centerGravity;
    // target.inertiaMoments = targetProps.inertiaMoments;
    // target.inertiaProducts = targetProps.inertiaProducts;

    // // Assign the weights
    // Optimizer::Weight weight;

    // // Select the options
    // Optimizer::Options options;
    // options.autoScale = false;
    // options.numIterations = 100;

    // // Run the solver
    // Optimizer optimizer(state, target, weight, options);
    // optimizer.run(initPanel);
}

//! Check if two double values are equal within the specified precision
bool TestBackend::isEqual(double firstValue, double secondValue, double precision)
{
    return qAbs(firstValue - secondValue) <= precision;
}

QTEST_MAIN(TestBackend)

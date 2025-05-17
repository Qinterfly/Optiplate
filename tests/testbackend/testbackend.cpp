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

//! Create a rectangular panel and verify its inertia properties
void TestBackend::testCreateBasePanel()
{
    double const precision = 1e-4;

    Panel panel;

    // Set panel properties
    double width = 2;
    double height = 1;
    double depth = 1e-3;
    double density = 0.5;
    panel.setThickness(0.0);
    panel.setXCoords({0, 0, width, width});
    panel.setZCoords({0, height, height, 0});
    panel.setDepths({depth, depth, depth});
    panel.setDensity(density);

    // Compute analytical properties
    double mass = density * width * height * depth;
    Vec3 centerGravity = {width / 2, 0, height / 2};
    double inertiaMomentX = mass / 12 * (qPow(height, 2.0) + qPow(depth, 2.0));
    double inertiaMomentY = mass / 12 * (qPow(width, 2.0) + qPow(height, 2.0));
    double inertiaMomentZ = mass / 12 * (qPow(width, 2.0) + qPow(depth, 2.0));
    Vec3 inertiaMoments = {inertiaMomentX, inertiaMomentY, inertiaMomentZ};

    // Evaluate inertia properties
    auto props = panel.massProperties();
    QVERIFY(isEqual(props.mass, mass, precision));
    for (int i = 0; i != skNumDirections; ++i)
    {
        QVERIFY(isEqual(props.centerGravity[i], centerGravity[i], precision));
        QVERIFY(isEqual(props.inertiaMoments[i], inertiaMoments[i], precision));
        QVERIFY(isEqual(props.inertiaProducts[i], 0.0, precision));
    }

    // Add panel to the project
    mBaseProject.setPanel(panel);
}

//! Update the base panel
void TestBackend::testUpdateBasePanel()
{
    Configuration& config = mBaseProject.configuration();
    Panel const& initPanel = mBaseProject.panel();

    // Set up the target panel
    Panel targetPanel;
    double width = 2.1;
    double height = 1;
    double depth = 1e-3;
    double density = 0.5;
    targetPanel.setThickness(0.0);
    targetPanel.setXCoords({0, 0, width, width});
    targetPanel.setZCoords({0, height, height, 0});
    targetPanel.setDepths({depth, depth, depth});
    targetPanel.setDensity(density);
    auto targetProps = targetPanel.massProperties();

    // Set the optimization state
    Optimizer::State& state = config.state;

    // Set the targets
    Properties& target = config.target;
    target.mass = targetProps.mass;
    target.centerGravity = targetProps.centerGravity;
    target.inertiaMoments = targetProps.inertiaMoments;
    target.inertiaProducts = targetProps.inertiaProducts;

    // Assign the weights
    Properties& weight = config.weight;

    // Select the options
    Optimizer::Options& options = config.options;
    options.maxRelativeError = 1e-3;
    options.logging = true;
    options.autoScale = true;
    options.diffStepSize = 1e-5;

    // Run the solver
    Optimizer optimizer(state, target, weight, options);
    auto solutions = optimizer.solve(initPanel);

    // Assign the solutions
    mBaseProject.setSolutions(solutions);
}

//! Build a panel associated with real data
void TestBackend::testCreateRealPanel()
{
    double const precision = 1e-4;

    // Create a panel
    Panel panel;
    panel.setThickness(0.0);
    panel.setXCoords({-0.73, -0.64, 0.43, 0.35});
    panel.setZCoords({0, 0.98, 0.98, 0});
    panel.setDepths({0.201, 0.16, 0.153});
    panel.setDensity(0.057855);

    // Define analytical properties
    Vec3 centerGravity = {-0.152764, 0, 0.47034};

    // Evaluate inertia properties
    auto props = panel.massProperties();
    QVERIFY(isEqual(props.mass, 0.01079, precision));
    for (int i = 0; i != skNumDirections; ++i)
        QVERIFY(isEqual(props.centerGravity[i], centerGravity[i], precision));

    // Add panel to the project
    mRealProject.setPanel(panel);
}

//! Update the real panel
void TestBackend::testUpdateRealPanel()
{
    Configuration& config = mRealProject.configuration();
    Panel const& initPanel = mRealProject.panel();

    // Set the optimization state
    Optimizer::State& state = config.state;

    // Set the targets
    Properties& target = config.target;
    target.mass = 0.01079;
    target.centerGravity = {-0.155, 0, 0.519};
    target.inertiaMoments = {0.00077, 0.00176, 0.00107};
    target.inertiaProducts = {0, 0, 0.00001};

    // Assign the weights
    Properties& weight = config.weight;

    // Select the options
    Optimizer::Options& options = config.options;
    options.autoScale = true;
    options.diffStepSize = 1e-5;

    // Run the solver
    Optimizer optimizer(state, target, weight, options);
    auto solutions = optimizer.solve(initPanel);

    // Assign the solutions
    mRealProject.setSolutions(solutions);
}

//! Check if two double values are equal within the specified precision
bool TestBackend::isEqual(double firstValue, double secondValue, double precision)
{
    return qAbs(firstValue - secondValue) <= precision;
}

QTEST_MAIN(TestBackend)

/*!
 * \file
 * \author Pavel Lakiza
 * \date July 2025
 * \brief Implementation of the TestBackend class
 */

#include <config.h>
#include <QFileInfo>
#include <QRandomGenerator>

#include "testbackend.h"

#include "fileutility.h"
#include "optimizer.h"
#include "panel.h"

using namespace Tests;
using namespace Backend;
using namespace KCL;

static int const skNumDirections = 3;

TestBackend::TestBackend()
    : mBaseProject("base")
    , mRealProject("real")
{

}

//! Create a series of panels with random parameters and estimate their properties
void TestBackend::testCreateRandomPanel()
{
    // Parameters
    int numSamples = 10;
    QPair<double, double> limitsThickness = {0, 1};
    QPair<double, double> limitsCoords = {-100, 100};
    QPair<double, double> limitsDepths = {0, 10};
    QPair<double, double> limitsDensity = {0, 10000};
    double timeout = 1.0;

    // Assign the path to write generated projects
    QString fileName = QString("random.%1").arg(Project::fileSuffix());
    QString pathFile = Utility::combineFilePath(TEMP_DIR, fileName);

    // Generate the series of panels
    KCL::Vec4 xCoords, zCoords;
    KCL::Vec3 depths;
    int numCoords = xCoords.size();
    while (numSamples-- > 0)
    {
        // Generate random parameters
        double thickness = generateDouble(limitsThickness);
        for (int i = 0; i != numCoords; ++i)
        {
            xCoords[i] = generateDouble(limitsCoords);
            zCoords[i] = generateDouble(limitsCoords);
        }
        for (double& value : depths)
            value = generateDouble(limitsDepths);
        double density = generateDouble(limitsDensity);

        // Set panel data
        Project project;
        Panel& panel = project.panel();
        panel.setThickness(thickness);
        panel.setXCoords(xCoords);
        panel.setZCoords(zCoords);
        panel.setDepths(depths);
        panel.setDensity(density);

        // Write the project
        project.write(pathFile);

        // Estimate mass properties
        panel.massProperties(timeout);
    }
}

//! Create a rectangular panel and verify its inertia properties
void TestBackend::testCreateBasePanel()
{
    double const precision = 1e-4;

    Panel& panel = mBaseProject.panel();

    // Set panel data
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
}

//! Build a panel associated with real data
void TestBackend::testCreateRealPanel()
{
    double const precision = 1e-4;

    Panel& panel = mRealProject.panel();

    // Set panel data
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
}

//! Update the base panel
void TestBackend::testUpdateBasePanel()
{
    Configuration& config = mBaseProject.configuration();
    Panel const& initPanel = mBaseProject.panel();

    // Set up the target panel
    Panel targetPanel;
    double width = 2.1;
    double height = 0.9;
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
    options.autoScale = false;
    options.maxRelativeError = 1e-3;
    options.numThreads = 1;
    options.diffStepSize = 1e-3;

    // Run the solver
    Optimizer optimizer(state, target, weight, options);
    connect(&optimizer, &Optimizer::log, this, &TestBackend::log);
    auto solutions = optimizer.solve(initPanel);
    QVERIFY(!solutions.empty());
    QVERIFY(solutions.back().isSuccess);

    // Assign the solutions
    mBaseProject.setSolutions(solutions);
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
    options.autoScale = false;
    options.diffStepSize = 1e-3;
    options.numThreads = 1;

    // Run the solver
    Optimizer optimizer(state, target, weight, options);
    connect(&optimizer, &Optimizer::log, this, &TestBackend::log);
    auto solutions = optimizer.solve(initPanel);
    QVERIFY(!solutions.empty());
    QVERIFY(solutions.back().isSuccess);

    // Assign the solutions
    mRealProject.setSolutions(solutions);
}

//! Write the base and real projects to files
void TestBackend::testWriteProject()
{
    std::vector<Backend::Project> projects({mBaseProject, mRealProject});
    for (auto& project : projects)
    {
        // Write the project to a file
        QString fileName = project.configuration().name + '.' + Project::fileSuffix();
        QString pathFile = Utility::combineFilePath(EXAMPLES_DIR, fileName);
        QVERIFY(project.write(pathFile));

        // Read the project
        Project checkProject;
        QVERIFY(checkProject.read(pathFile));

        // Check if the project are equal
        QVERIFY(checkProject.configuration().name == project.configuration().name);
        QVERIFY(checkProject.panel().density() == project.panel().density());
        QVERIFY(checkProject.solutions().size() == project.solutions().size());
    }
}

//! Check if two double values are equal within the specified precision
bool TestBackend::isEqual(double firstValue, double secondValue, double precision)
{
    return qAbs(firstValue - secondValue) <= precision;
}

//! Output message for logging
void TestBackend::log(QString message)
{
    std::cout << message.toStdString() << std::endl;
}

//! Generate a bounded double value
double TestBackend::generateDouble(QPair<double, double> const& limits)
{
    double value = QRandomGenerator::global()->generateDouble();
    return limits.first + value * (limits.second - limits.first);
}

QTEST_MAIN(TestBackend)

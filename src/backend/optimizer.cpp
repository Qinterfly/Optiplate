/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the Optimizer class
 */

#define MAX_LOG_LEVEL -1

#include <functional>
#include <QTextStream>
#include <QThread>
#include <QXmlStreamWriter>

#include "optimizer.h"

using namespace Backend;
using namespace KCL;

using Solutions = QList<Optimizer::Solution>;

static const int skNumDirections = 3;

ObjectiveFunctor::ObjectiveFunctor(Properties const& target, Properties const& weight, Optimizer::Options const& options, UnwrapFun unwrapFun,
                                   SolverFun solverFun)
    : mTarget(target)
    , mWeight(weight)
    , mUnwrapFun(unwrapFun)
    , mSolverFun(solverFun)
{
}

//! Compute the residuals
bool ObjectiveFunctor::operator()(double const* const* parameters, double* residuals) const
{
    Panel panel = mUnwrapFun(*parameters);

    // Check if the panel is valid
    if (!panel.isValid())
        return false;

    // Obtain the solution
    Properties props = mSolverFun(panel);
    if (props.mass == 0.0)
        return false;

    // Compure the errors in properties
    auto errorProps = props.compare(mTarget, mWeight);

    // Set the residuals
    auto errors = errorProps.validValues();
    std::copy_n(errors.begin(), errors.size(), residuals);

    return true;
}

OptimizerCallback::OptimizerCallback(std::vector<double> const& parameters, Properties const& target, Properties const& weight,
                                     Optimizer::Options const& options, UnwrapFun unwrapFun, SolverFun solverFun)
    : mParameters(parameters)
    , mTarget(target)
    , mWeight(weight)
    , mOptions(options)
    , mUnwrapFun(unwrapFun)
    , mSolverFun(solverFun)
{
}

ceres::CallbackReturnType OptimizerCallback::operator()(ceres::IterationSummary const& summary)
{
    // Check if the user requested to stop the solver
    if (QThread::currentThread()->isInterruptionRequested())
        return ceres::SOLVER_ABORT;

    // Obtain the solution
    Panel panel = mUnwrapFun(mParameters.data());
    Properties props = mSolverFun(panel);

    // Compare the properties with the target ones
    Properties errorProps = props.compare(mTarget, mWeight);
    double maxError = 0.0;
    auto errors = errorProps.validValues();
    for (auto e : errors)
        maxError = std::max(maxError, std::abs(e));

    // Create the function for printing
    int iLastName = 0;
    QString message;
    QTextStream stream(&message);
    auto logFun = [&stream, &iLastName](std::vector<double> const& current, std::vector<double> const& target, std::vector<double> const& errors)
    {
        std::vector<std::string> const kNames = {"M", "Cx", "Cy", "Cz", "Jx", "Jy", "Jz", "Jxy", "Jyz", "Jxz"};
        auto constexpr kFormat = "{:^7} {:10.4g} {:10.4g} {:10.4f}";
        int numValues = current.size();
        for (int i = 0; i != numValues; ++i)
        {
            if (!std::isnan(errors[i]))
                stream << std::format(kFormat, kNames[iLastName], current[i], target[i], errors[i] * 100).c_str() << Qt::endl;
            ++iLastName;
        }
    };
    auto vecFun = [](KCL::Vec3 vec) { return std::vector<double>(vec.begin(), vec.end()); };

    // Print the current info
    if (summary.iteration == 0)
        stream << std::format("{:^8} {:>6} {:>11} {:>10}", "Iter", "Fun", "Diff", "Grad").c_str() << Qt::endl;
    auto constexpr kFormat = "{:^7d} {:10.3e} {:10.3e} {:10.3e}";
    stream << std::format(kFormat, summary.iteration, summary.cost, summary.cost_change, summary.gradient_max_norm).c_str() << Qt::endl;
    stream << Qt::endl;

    // Print the properties
    logFun({props.mass}, {mTarget.mass}, {errorProps.mass});
    logFun(vecFun(props.centerGravity), vecFun(mTarget.centerGravity), vecFun(errorProps.centerGravity));
    logFun(vecFun(props.inertiaMoments), vecFun(mTarget.inertiaMoments), vecFun(errorProps.inertiaMoments));
    logFun(vecFun(props.inertiaProducts), vecFun(mTarget.inertiaProducts), vecFun(errorProps.inertiaProducts));

    // Indicate that the iteration is finished
    Optimizer::Solution solution;
    solution.iteration = summary.iteration;
    solution.isSuccess = summary.step_is_successful;
    solution.duration = summary.iteration_time_in_seconds;
    solution.cost = summary.cost;
    solution.panel = panel;
    solution.properties = props;
    solution.errorProperties = errorProps;
    emit iterationFinished(solution);
    emit log(message);

    // Check if the termination criterion is achieved
    if (maxError < mOptions.maxRelativeError)
        return ceres::SOLVER_TERMINATE_SUCCESSFULLY;
    return ceres::SOLVER_CONTINUE;
}

Optimizer::Optimizer()
{
}

Optimizer::Optimizer(State const& state, Properties const& target, Properties const& weight, Options const& options)
    : mState(state)
    , mTarget(target)
    , mWeight(weight)
    , mOptions(options)
{
}

//! Run the optimization process
QList<Optimizer::Solution> Optimizer::solve(Panel const& initPanel)
{
    // Get the parameters
    std::vector<double> parameters = wrap(initPanel);
    int numParameters = parameters.size();

    // Create the auxiliary functions
    UnwrapFun unwrapFun = [this, &initPanel, &numParameters](const double* const x)
    {
        std::vector<double> params(x, x + numParameters);
        return unwrap(initPanel, params);
    };
    auto solverFun = [this](Panel const& panel) { return panel.massProperties(mOptions.timeoutIteration); };

    // Obtain the initial solution
    Properties initProps = solverFun(initPanel);
    Properties initErrors = initProps.compare(mTarget, mWeight);

    // Estimate the number of residuals
    int numResiduals = initErrors.numValidValues();

    // Assign options to compute Jacobian
    ceres::NumericDiffOptions diffOpts;
    diffOpts.relative_step_size = mOptions.diffStepSize;

    // Create the cost function
    ObjectiveFunctor functor(mTarget, mWeight, mOptions, unwrapFun, solverFun);
    auto* costFunction = new ceres::DynamicNumericDiffCostFunction<ObjectiveFunctor, ceres::FORWARD>(&functor, ceres::DO_NOT_TAKE_OWNERSHIP,
                                                                                                     diffOpts);
    costFunction->AddParameterBlock(numParameters);
    costFunction->SetNumResiduals(numResiduals);

    // Set the problem
    ceres::Problem problem;
    problem.AddResidualBlock(costFunction, nullptr, parameters.data());

    // Intialize the resulting set
    Solutions solutions;
    solutions.reserve(mOptions.maxNumIterations);

    // Assign the solver settings
    ceres::Solver::Options solverOpts;
    solverOpts.max_num_iterations = mOptions.maxNumIterations;
    solverOpts.num_threads = mOptions.numThreads;
    solverOpts.minimizer_type = ceres::TRUST_REGION;
    solverOpts.linear_solver_type = ceres::DENSE_QR;
    solverOpts.use_nonmonotonic_steps = true;
    solverOpts.logging_type = ceres::SILENT;
    solverOpts.minimizer_progress_to_stdout = false;

    // Set the callback functions
    solverOpts.update_state_every_iteration = true;
    OptimizerCallback callback(parameters, mTarget, mWeight, mOptions, unwrapFun, solverFun);
    connect(&callback, &OptimizerCallback::iterationFinished, this,
            [this, &solutions](Solution solution)
            {
                solutions.push_back(solution);
                emit iterationFinished(solution);
            });
    connect(&callback, &OptimizerCallback::log, this, &Optimizer::log);
    solverOpts.callbacks.push_back(&callback);

    // Solve the problem
    ceres::Solver::Summary summary;
    ceres::Solve(solverOpts, &problem, &summary);
    if (!solutions.empty())
    {
        auto lastSolution = solutions.back();
        lastSolution.isSuccess = summary.IsSolutionUsable();
        lastSolution.message = summary.message.c_str();
    }

    // Log the report
    printReport(summary);

    return solutions;
}

//! Vectorize parameters for optimization
std::vector<double> Optimizer::wrap(Panel const& panel)
{
    std::vector<double> parameters;
    double scale;

    // Coordinates of points
    if (mState.vertices)
    {
        int numValues = panel.xCoords().size();
        if (mOptions.autoScale)
        {
            scale = 0.0;
            for (int i = 0; i != numValues; ++i)
            {
                scale = std::max(scale, std::abs(panel.xCoords()[i]));
                scale = std::max(scale, std::abs(panel.zCoords()[i]));
            }
            mScale.vertices = scale;
        }
        for (int i = 0; i != numValues; ++i)
        {
            parameters.push_back(panel.xCoords()[i] / mScale.vertices);
            parameters.push_back(panel.zCoords()[i] / mScale.vertices);
        }
    }

    // Depths
    if (mState.depths)
    {
        int numValues = panel.depths().size();
        if (mOptions.autoScale)
        {
            scale = 0.0;
            for (int i = 0; i != numValues; ++i)
                scale = std::max(scale, std::abs(panel.depths()[i]));
            mScale.depths = scale;
        }
        for (int i = 0; i != numValues; ++i)
            parameters.push_back(panel.depths()[i] / mScale.depths);
    }

    // Density
    if (mState.density)
    {
        if (mOptions.autoScale)
            mScale.density = std::abs(panel.density());
        parameters.push_back(panel.density() / mScale.density);
    }

    return parameters;
}

//! Modify model properties by a vector of parameters
Panel Optimizer::unwrap(Panel const& basePanel, std::vector<double> const& parameters)
{
    Panel panel = basePanel;
    int iLast = 0;

    // Coordinates of points
    if (mState.vertices)
    {
        int numValues = panel.xCoords().size();
        KCL::Vec4 xCoords, zCoords;
        for (int i = 0; i != numValues; ++i)
        {
            xCoords[i] = parameters[iLast++] * mScale.vertices;
            zCoords[i] = parameters[iLast++] * mScale.vertices;
        }
        panel.setXCoords(xCoords);
        panel.setZCoords(zCoords);
    }

    // Depths
    if (mState.depths)
    {
        int numValues = panel.depths().size();
        KCL::Vec3 depths;
        for (int i = 0; i != numValues; ++i)
            depths[i] = parameters[iLast++] * mScale.depths;
        panel.setDepths(depths);
    }

    // Density
    if (mState.density)
        panel.setDensity(parameters[iLast++] * mScale.density);

    // Renumerate points, if necessary
    panel.renumerate();

    return panel;
}

//! Output the report to log
void Optimizer::printReport(ceres::Solver::Summary const& summary)
{
    std::string message = "Ceres Solver Report\n";
    message += std::format("* Iterations:   {:d}\n", summary.iterations.size());
    message += std::format("* Initial cost: {:.3e}\n", summary.initial_cost);
    message += std::format("* Final cost:   {:.3e}\n", summary.final_cost);
    message += std::format("* Duration:     {:.3f} s\n", summary.total_time_in_seconds);
    message += std::format("* Termination:  {:}", ceres::TerminationTypeToString(summary.termination_type));
    emit log(message.c_str());
}

Optimizer::Solution::Solution()
    : iteration(-1)
    , isSuccess(false)
    , duration(0.0)
    , cost(std::numeric_limits<double>::infinity())
    , properties(0.0)
    , errorProperties(std::numeric_limits<double>::infinity())
{
}

//! Read a solution from a XML stream
void Optimizer::Solution::read(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "iteration")
            iteration = stream.readElementText().toInt();
        else if (stream.name() == "isSuccess")
            isSuccess = stream.readElementText().toInt();
        else if (stream.name() == "duration")
            duration = stream.readElementText().toDouble();
        else if (stream.name() == "cost")
            cost = stream.readElementText().toDouble();
        else if (stream.name() == "panel")
            panel.read(stream);
        else if (stream.name() == "properties")
            properties.read(stream);
        else if (stream.name() == "errorProperties")
            errorProperties.read(stream);
        else if (stream.name() == "message")
            message = stream.readElementText();
        else
            stream.skipCurrentElement();
    }
}

//! Output the solution to a XML stream
void Optimizer::Solution::write(QXmlStreamWriter& stream) const
{
    stream.writeStartElement("solution");
    stream.writeTextElement("iteration", QString::number(iteration));
    stream.writeTextElement("isSuccess", QString::number(isSuccess));
    stream.writeTextElement("duration", QString::number(duration));
    stream.writeTextElement("cost", QString::number(cost));
    panel.write(stream);
    properties.write("properties", stream);
    errorProperties.write("errorProperties", stream);
    stream.writeTextElement("message", message);
    stream.writeEndElement();
}

//! Check if the solution is valid
bool Optimizer::Solution::isValid() const
{
    return iteration >= 0;
}

//! Enabled parameters for optimization
Optimizer::State::State()
    : vertices(true)
    , depths(true)
    , density(true)
{
}

//! Read an optimizer state from a XML stream
void Optimizer::State::read(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "vertices")
            vertices = stream.readElementText().toInt();
        else if (stream.name() == "depths")
            depths = stream.readElementText().toInt();
        else if (stream.name() == "density")
            density = stream.readElementText().toInt();
        else
            stream.skipCurrentElement();
    }
}

//! Output the optimizer state to a XML stream
void Optimizer::State::write(QXmlStreamWriter& stream) const
{
    stream.writeStartElement("state");
    stream.writeTextElement("vertices", QString::number(vertices));
    stream.writeTextElement("depths", QString::number(vertices));
    stream.writeTextElement("density", QString::number(vertices));
    stream.writeEndElement();
}

//! Optimization options
Optimizer::Options::Options()
    : autoScale(false)
    , maxNumIterations(256)
    , timeoutIteration(1.0)
    , numThreads(1)
    , maxRelativeError(1e-3)
    , diffStepSize(1e-3)
{
}

//! Read optimizer options from a XML stream
void Optimizer::Options::read(QXmlStreamReader& stream)
{
    while (stream.readNextStartElement())
    {
        if (stream.name() == "autoScale")
            autoScale = stream.readElementText().toInt();
        else if (stream.name() == "maxNumIterations")
            maxNumIterations = stream.readElementText().toInt();
        else if (stream.name() == "timeoutIteration")
            timeoutIteration = stream.readElementText().toDouble();
        else if (stream.name() == "numThreads")
            numThreads = stream.readElementText().toInt();
        else if (stream.name() == "maxRelativeError")
            maxRelativeError = stream.readElementText().toDouble();
        else if (stream.name() == "diffStepSize")
            diffStepSize = stream.readElementText().toDouble();
        else
            stream.skipCurrentElement();
    }
}

//! Output the optimizer options to a XML stream
void Optimizer::Options::write(QXmlStreamWriter& stream) const
{
    stream.writeStartElement("options");
    stream.writeTextElement("autoScale", QString::number(autoScale));
    stream.writeTextElement("maxNumIterations", QString::number(maxNumIterations));
    stream.writeTextElement("timeoutIteration", QString::number(timeoutIteration));
    stream.writeTextElement("numThreads", QString::number(numThreads));
    stream.writeTextElement("maxRelativeSize", QString::number(maxRelativeError));
    stream.writeTextElement("diffStepSize", QString::number(diffStepSize));
    stream.writeEndElement();
}

Optimizer::Scale::Scale()
    : vertices(1.0)
    , depths(1.0)
    , density(1.0)
{
}

Optimizer::State const& Optimizer::state() const
{
    return mState;
}

Properties const& Optimizer::target() const
{
    return mTarget;
}

Properties const& Optimizer::weight() const
{
    return mWeight;
}

Optimizer::Options const& Optimizer::options() const
{
    return mOptions;
}

void Optimizer::setState(State const& state)
{
    mState = state;
}

void Optimizer::setTarget(Properties const& target)
{
    mTarget = target;
}

void Optimizer::setWeight(Properties const& weight)
{
    mWeight = weight;
}

void Optimizer::setOptions(Options const& options)
{
    mOptions = options;
}

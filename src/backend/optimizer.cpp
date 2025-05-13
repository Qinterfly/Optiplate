/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the Optimizer class
 */

#include <ceres/ceres.h>
#include <future>
#include <QDebug>

#include "optimizer.h"
#include "utility.h"

using namespace std::chrono_literals;
using namespace Backend;
using namespace KCL;

using UnwrapFun = std::function<Panel(const double* const)>;
using SolverFun = std::function<KCL::MassProperties(Panel const&)>;

static const int skNumDirections = 3;

//! Functor which are used by optimization solver
class ObjectiveFunctor
{
public:
    ObjectiveFunctor(Optimizer::Target const& target, Optimizer::Weight const& weight, Optimizer::Options const& options, UnwrapFun unwrapFun,
                     SolverFun solverFun)
        : mTarget(target)
        , mWeight(weight)
        , mOptions(options)
        , mUnwrapFun(unwrapFun)
        , mSolverFun(solverFun)
    {
    }

    //! Compute the residuals
    bool operator()(double const* const* parameters, double* residuals) const
    {
        Panel panel = mUnwrapFun(*parameters);

        // Check if the panel is valid
        if (!panel.isValid())
            return false;

        // Obtain the solution
        MassProperties props = mSolverFun(panel);
        if (props.mass == 0.0)
            return false;

        // Compute residuals
        int iLast = 0;
        if (mWeight.mass > mOptions.weightThreshold)
            residuals[iLast++] = Utility::relativeError(props.mass, mTarget.mass) * mWeight.mass;
        for (int i = 0; i != skNumDirections; ++i)
        {
            if (mWeight.centerGravity[i] > mOptions.weightThreshold)
                residuals[iLast++] = Utility::relativeError(props.centerGravity[i], mTarget.centerGravity[i]) * mWeight.centerGravity[i];
            if (mWeight.inertiaMoments[i] > mOptions.weightThreshold)
                residuals[iLast++] = Utility::relativeError(props.inertiaMoments[i], mTarget.inertiaMoments[i]) * mWeight.inertiaMoments[i];
            if (mWeight.inertiaProducts[i] > mOptions.weightThreshold)
                residuals[iLast++] = Utility::relativeError(props.inertiaProducts[i], mTarget.inertiaProducts[i]) * mWeight.inertiaProducts[i];
        }
        return true;
    }

private:
    Optimizer::Target mTarget;
    Optimizer::Weight mWeight;
    Optimizer::Options mOptions;
    UnwrapFun mUnwrapFun;
    SolverFun mSolverFun;
};

//! Functor which is called after every optimization iteration
class OptimizerCallback : public ceres::IterationCallback
{
public:
    OptimizerCallback(std::vector<double> const& parameters, Optimizer::Target const& target, Optimizer::Weight const& weight,
                      Optimizer::Options const& options, UnwrapFun unwrapFun, SolverFun solverFun, bool logging)
        : mParameters(parameters)
        , mTarget(target)
        , mWeight(weight)
        , mOptions(options)
        , mUnwrapFun(unwrapFun)
        , mSolverFun(solverFun)
        , mLogging(logging)
    {
    }

    ~OptimizerCallback() = default;

    ceres::CallbackReturnType operator()(ceres::IterationSummary const& summary)
    {
        // Obtain the solution
        Panel panel = mUnwrapFun(mParameters.data());
        MassProperties props = mSolverFun(panel);

        // Create the working function
        double error;
        double maxError = 0.0;
        int iLastName = 0;
        auto processFun = [this, &error, &maxError, &iLastName](std::vector<double> const& values, std::vector<double> const& targetValues,
                                                                std::vector<double> const& weights)
        {
            static std::vector<std::string> const kNames = {"M", "Cx", "Cy", "Cz", "Jx", "Jy", "Jz", "Jxy", "Jyz", "Jxz"};
            static auto constexpr kFormat = "{:^7} {:10.4g} {:10.4g} {:10.4f}\n";
            int numValues = values.size();
            for (int i = 0; i != numValues; ++i)
            {
                if (weights[i] > mOptions.weightThreshold)
                {
                    error = Utility::relativeError(values[i], targetValues[i]);
                    if (mLogging)
                        std::cout << std::format(kFormat, kNames[iLastName], values[i], targetValues[i], error * 100);
                    maxError = std::max(maxError, std::abs(error));
                }
                ++iLastName;
            }
        };
        auto vecFun = [](KCL::Vec3 vec) { return std::vector<double>(vec.begin(), vec.end()); };

        // Process the properties
        if (mLogging)
            std::cout << std::endl;
        processFun({props.mass}, {mTarget.mass}, {mWeight.mass});
        processFun(vecFun(props.centerGravity), vecFun(mTarget.centerGravity), vecFun(mWeight.centerGravity));
        processFun(vecFun(props.inertiaMoments), vecFun(mTarget.inertiaMoments), vecFun(mWeight.inertiaMoments));
        processFun(vecFun(props.inertiaProducts), vecFun(mTarget.inertiaProducts), vecFun(mWeight.inertiaProducts));
        if (mLogging)
            std::cout << std::endl;

        // Check if the termination criterion is achieved
        if (maxError < mOptions.maxRelativeError)
            return ceres::SOLVER_TERMINATE_SUCCESSFULLY;
        return ceres::SOLVER_CONTINUE;
    }

private:
    std::vector<double> const& mParameters;
    Optimizer::Target mTarget;
    Optimizer::Weight mWeight;
    Optimizer::Options mOptions;
    UnwrapFun mUnwrapFun;
    SolverFun mSolverFun;
    bool mLogging;
};

Optimizer::Optimizer(State const& state, Target const& target, Weight const& weight,
                     Options const& options)
    : mState(state)
    , mTarget(target)
    , mWeight(weight)
    , mOptions(options)
{
}

//! Run the optimization process
void Optimizer::run(Panel const& initPanel)
{
    // Get the parameters
    std::vector<double> parameters = wrap(initPanel);
    int numParameters = parameters.size();
    auto initParameters = parameters;

    // Create the auxiliary functions
    UnwrapFun unwrapFun = [this, &initPanel, &numParameters](const double* const x)
    {
        std::vector<double> params(x, x + numParameters);
        return unwrap(initPanel, params);
    };
    auto solverFun = [this](Panel const& panel)
    {
        auto fun = [&panel]() { return panel.massProperties(); };
        std::future<MassProperties> future = std::async(fun);
        std::future_status status = future.wait_for(mOptions.timeoutIteration);
        if (status != std::future_status::ready)
            return MassProperties();
        return future.get();
    };

    // Estimate the number of residuals
    int numResiduals = 0;
    if (mWeight.mass > mOptions.weightThreshold)
        ++numResiduals;
    for (int i = 0; i != skNumDirections; ++i)
    {
        if (mWeight.centerGravity[i] > mOptions.weightThreshold)
            ++numResiduals;
        if (mWeight.inertiaMoments[i] > mOptions.weightThreshold)
            ++numResiduals;
        if (mWeight.inertiaProducts[i] > mOptions.weightThreshold)
            ++numResiduals;
    }

    // Assign diff options
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

    // Assign the solver settings
    ceres::Solver::Options solverOpts;
    solverOpts.max_num_iterations = mOptions.numIterations;
    solverOpts.num_threads = mOptions.numThreads;
    solverOpts.minimizer_type = ceres::TRUST_REGION;
    solverOpts.linear_solver_type = ceres::DENSE_QR;
    solverOpts.use_nonmonotonic_steps = true;

    // Set the callback functions
    solverOpts.update_state_every_iteration = true;
    solverOpts.minimizer_progress_to_stdout = mOptions.logging;
    OptimizerCallback callback(parameters, mTarget, mWeight, mOptions, unwrapFun, solverFun, mOptions.logging);
    solverOpts.callbacks.push_back(&callback);

    // Solve the problem
    ceres::Solver::Summary summary;
    ceres::Solve(solverOpts, &problem, &summary);
    if (mOptions.logging)
        std::cout << summary.FullReport();

    // Process the result
    Panel optPanel = unwrapFun(parameters.data());
    MassProperties optProps = solverFun(optPanel);
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

//! Enabled parameters for optimization
Optimizer::State::State()
    : vertices(true)
    , depths(true)
    , density(true)
{
}

//! Optimization target values
Optimizer::Target::Target()
    : mass(0.0)
{
    centerGravity.fill(0.0);
    inertiaMoments.fill(0.0);
    inertiaProducts.fill(0.0);
}

//! Weights by which residuals get multiplied
Optimizer::Weight::Weight()
    : mass(1.0)
{
    centerGravity.fill(1.0);
    inertiaMoments.fill(1.0);
    inertiaProducts.fill(1.0);
    coefficients = {1.0, 0.0};
}

//! Optimization options
Optimizer::Options::Options()
    : logging(true)
    , autoScale(false)
    , numIterations(500)
    , timeoutIteration(1s)
    , numThreads(1)
    , weightThreshold(0.0)
    , maxRelativeError(1e-3)
    , diffStepSize(1e-5)
{
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

Optimizer::Target const& Optimizer::target() const
{
    return mTarget;
}

Optimizer::Weight const& Optimizer::weight() const
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

void Optimizer::setTarget(Target const& target)
{
    mTarget = target;
}

void Optimizer::setWeight(Weight const& weight)
{
    mWeight = weight;
}

void Optimizer::setOptions(Options const& options)
{
    mOptions = options;
}

/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the Optimizer class
 */

#include <functional>
#include <future>

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
    ObjectiveFunctor(Properties const& target, Properties const& weight, Optimizer::Options const& options, UnwrapFun unwrapFun,
                     SolverFun solverFun)
        : mTarget(target)
        , mWeight(weight)
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

private:
    Properties mTarget;
    Properties mWeight;
    UnwrapFun mUnwrapFun;
    SolverFun mSolverFun;
};

//! Functor which is called after every optimization iteration
class OptimizerCallback : public ceres::IterationCallback
{
public:
    OptimizerCallback(std::vector<double> const& parameters, Properties const& target, Properties const& weight,
                      Optimizer::Options const& options, UnwrapFun unwrapFun, SolverFun solverFun, bool logging)
        : mParameters(parameters)
        , mTarget(target)
        , mWeight(weight)
        , mOptions(options)
        , mUnwrapFun(unwrapFun)
        , mSolverFun(solverFun)
    {
    }

    ~OptimizerCallback() = default;

    ceres::CallbackReturnType operator()(ceres::IterationSummary const& summary)
    {
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
        auto printFun = [&iLastName](std::vector<double> const& current, std::vector<double> const& target, std::vector<double> const& errors)
        {
            static std::vector<std::string> const kNames = {"M", "Cx", "Cy", "Cz", "Jx", "Jy", "Jz", "Jxy", "Jyz", "Jxz"};
            static auto constexpr kFormat = "{:^7} {:10.4g} {:10.4g} {:10.4f}\n";
            int numValues = current.size();
            for (int i = 0; i != numValues; ++i)
            {
                if (!std::isnan(errors[i]))
                    std::cout << std::format(kFormat, kNames[iLastName], current[i], target[i], errors[i] * 100);
                ++iLastName;
            }
        };
        auto vecFun = [](KCL::Vec3 vec) { return std::vector<double>(vec.begin(), vec.end()); };

        // Print the properties
        if (mOptions.logging)
        {
            std::cout << std::endl;
            printFun({props.mass}, {mTarget.mass}, {errorProps.mass});
            printFun(vecFun(props.centerGravity), vecFun(mTarget.centerGravity), vecFun(errorProps.centerGravity));
            printFun(vecFun(props.inertiaMoments), vecFun(mTarget.inertiaMoments), vecFun(errorProps.inertiaMoments));
            printFun(vecFun(props.inertiaProducts), vecFun(mTarget.inertiaProducts), vecFun(errorProps.inertiaProducts));
            std::cout << std::endl;
        }

        // Check if the termination criterion is achieved
        if (maxError < mOptions.maxRelativeError)
            return ceres::SOLVER_TERMINATE_SUCCESSFULLY;
        return ceres::SOLVER_CONTINUE;
    }

private:
    std::vector<double> const& mParameters;
    Properties mTarget;
    Properties mWeight;
    Optimizer::Options mOptions;
    UnwrapFun mUnwrapFun;
    SolverFun mSolverFun;
};

Optimizer::Optimizer(State const& state, Properties const& target, Properties const& weight, Options const& options)
    : mState(state)
    , mTarget(target)
    , mWeight(weight)
    , mOptions(options)
{
}

//! Run the optimization process
Optimizer::Solution Optimizer::solve(Panel const& initPanel)
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
        std::future<Properties> future = std::async(fun);
        std::future_status status = future.wait_for(mOptions.timeoutIteration);
        if (status != std::future_status::ready)
            return Properties();
        return future.get();
    };

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
    return Solution(summary, optPanel, optProps);
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

Optimizer::Solution::Solution()
{
}

Optimizer::Solution::Solution(ceres::Solver::Summary const& summary, Panel const& rPanel, Properties const& rProperties)
    : iteration(summary.iterations.size())
    , cost(summary.final_cost)
    , panel(rPanel)
    , properties(rProperties)
{
}

//! Enabled parameters for optimization
Optimizer::State::State()
    : vertices(true)
    , depths(true)
    , density(true)
{
}

//! Optimization options
Optimizer::Options::Options()
    : logging(true)
    , autoScale(false)
    , numIterations(500)
    , timeoutIteration(1s)
    , numThreads(1)
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

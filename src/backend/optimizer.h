/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the Optimizer class
 */

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <ceres/ceres.h>
#include <chrono>
#include <QObject>

#include "panel.h"

namespace Backend
{

using UnwrapFun = std::function<Panel(const double* const)>;
using SolverFun = std::function<KCL::MassProperties(Panel const&)>;

//! Class to perform optimization of a panel model
class Optimizer : public QObject
{
    Q_OBJECT

public:
    //! Solution of the optimization problem
    struct Solution
    {
        Solution();

        bool isSuccess;
        double duration;
        double cost;
        Panel panel;
        Properties properties;
        Properties errorProperties;
    };

    //! Enabled parameters for optimization
    struct State
    {
        State();

        bool vertices;
        bool depths;
        bool density;
    };

    //! Optimization settings
    struct Options
    {
        Options();

        bool logging;
        bool autoScale;
        int maxNumIterations;
        std::chrono::seconds timeoutIteration;
        int numThreads;
        double maxRelativeError;
        double diffStepSize;
    };

    Optimizer(State const& state, Properties const& target, Properties const& weight, Options const& options);
    ~Optimizer() = default;

    QList<Solution> solve(Panel const& panel);

    State const& state() const;
    Properties const& target() const;
    Properties const& weight() const;
    Options const& options() const;

    void setState(State const& state);
    void setTarget(Properties const& target);
    void setWeight(Properties const& weight);
    void setOptions(Options const& options);

private:
    //! Optimization scale
    struct Scale
    {
        Scale();

        double vertices;
        double depths;
        double density;
    };

    std::vector<double> wrap(Panel const& panel);
    Panel unwrap(Panel const& basePanel, std::vector<double> const& parameters);

    State mState;
    Properties mTarget;
    Properties mWeight;
    Options mOptions;
    Scale mScale;
};

//! Functor to compute residuals
class ObjectiveFunctor
{
public:
    ObjectiveFunctor(Properties const& target, Properties const& weight, Optimizer::Options const& options, UnwrapFun unwrapFun,
                     SolverFun solverFun);
    bool operator()(double const* const* parameters, double* residuals) const;

private:
    Properties mTarget;
    Properties mWeight;
    UnwrapFun mUnwrapFun;
    SolverFun mSolverFun;
};

//! Functor to be called after every optimization iteration
class OptimizerCallback : public QObject, public ceres::IterationCallback
{
    Q_OBJECT

public:
    OptimizerCallback(std::vector<double> const& parameters, Properties const& target, Properties const& weight,
                      Optimizer::Options const& options, UnwrapFun unwrapFun, SolverFun solverFun, bool logging);
    ceres::CallbackReturnType operator()(ceres::IterationSummary const& summary);

signals:
    void iterationFinished(Backend::Optimizer::Solution solution);

private:
    std::vector<double> const& mParameters;
    Properties const& mTarget;
    Properties const& mWeight;
    Optimizer::Options const& mOptions;
    UnwrapFun mUnwrapFun;
    SolverFun mSolverFun;
};

} // namespace Backend

#endif // OPTIMIZER_H

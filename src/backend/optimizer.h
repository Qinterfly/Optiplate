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

#include "panel.h"

namespace Backend
{

//! Class to perform optimization of a panel model
class Optimizer
{
public:
    //! Solution of the optimization problem
    struct Solution
    {
        Solution();
        Solution(ceres::Solver::Summary const& summary, Panel const& rPanel, Properties const& rProperties);

        int iteration;
        double cost;
        Panel panel;
        Properties properties;
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

        //! Log results to std::out
        bool logging;

        //! Scale parameters during optimization
        bool autoScale;

        //! Maximum number of iterations
        int numIterations;

        //! Maximum duration of inertia properties computation per iteration
        std::chrono::seconds timeoutIteration;

        //! Number of threads used for optimization
        int numThreads;

        //! Threshold relative error of inertia properties
        double maxRelativeError;

        //! Relative step size to compute Jacobian
        double diffStepSize;
    };

    Optimizer(State const& state, Properties const& target, Properties const& weight, Options const& options);
    ~Optimizer() = default;

    Solution solve(Panel const& panel);

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

} // namespace Backend

#endif // OPTIMIZER_H

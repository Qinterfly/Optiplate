/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the Optimizer class
 */

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <chrono>

#include "panel.h"

namespace Backend
{

//! Class to perform optimization of a panel model
class Optimizer
{
public:
    //! Enabled parameters for optimization
    struct State
    {
        State();

        bool vertices;
        bool depths;
        bool density;
    };

    //! Target values for optimization
    struct Target
    {
        Target();

        double mass;
        KCL::Vec3 centerGravity;
        KCL::Vec3 inertiaMoments;
        KCL::Vec3 inertiaProducts;
    };

    //! Weights which used in residuals in the objective function
    struct Weight
    {
        Weight();

        double mass;
        KCL::Vec3 centerGravity;
        KCL::Vec3 inertiaMoments;
        KCL::Vec3 inertiaProducts;
        KCL::Vec2 coefficients;
    };

    //! Optimization settings
    struct Options
    {
        Options();

        bool autoScale;
        int numIterations;
        std::chrono::seconds timeoutIteration;
        int numThreads;
        double weightThreshold;
    };

    Optimizer(State const& state, Target const& target, Weight const& weight,
              Options const& options);
    ~Optimizer() = default;

    void run(Panel const& panel);

    State const& state() const;
    Target const& target() const;
    Weight const& weight() const;
    Options const& options() const;

    void setState(State const& state);
    void setTarget(Target const& target);
    void setWeight(Weight const& weight);
    void setOptions(Options const& options);

private:
    /// Optimization scale
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
    Target mTarget;
    Weight mWeight;
    Options mOptions;
    Scale mScale;
};

} // namespace Backend

#endif // OPTIMIZER_H

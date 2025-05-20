/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the Optimizer class
 */

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <ceres/ceres.h>

#include "panel.h"

QT_FORWARD_DECLARE_CLASS(QXmlStreamReader)
QT_FORWARD_DECLARE_CLASS(QXmlStreamWriter)

namespace Backend
{

using UnwrapFun = std::function<Panel(const double* const)>;
using SolverFun = std::function<KCL::MassProperties(Panel const&)>;

//! Class to perform optimization of a panel model
class Optimizer
{
public:
    //! Solution of the optimization problem
    struct Solution
    {
        Solution();

        void read(QXmlStreamReader& stream);
        void write(QXmlStreamWriter& stream);

        int iteration;
        bool isSuccess;
        double duration;
        double cost;
        Panel panel;
        Properties properties;
        Properties errorProperties;
        QString message;
    };

    //! Enabled parameters for optimization
    struct State
    {
        State();

        void read(QXmlStreamReader& stream);
        void write(QXmlStreamWriter& stream);

        bool vertices;
        bool depths;
        bool density;
    };

    //! Optimization options
    struct Options
    {
        Options();

        void read(QXmlStreamReader& stream);
        void write(QXmlStreamWriter& stream);

        bool logging;
        bool autoScale;
        int maxNumIterations;
        int timeoutIteration;
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

} // namespace Backend

#endif // OPTIMIZER_H

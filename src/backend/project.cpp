/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the Project class
 */

#include "project.h"

using namespace Backend;

Configuration::Configuration()
    : target(0.0)
    , weight(1.0)
{
}

Project::Project()
{
}

Configuration& Project::configuration()
{
    return mConfiguration;
}

Panel const& Project::panel() const
{
    return mPanel;
}

QList<Optimizer::Solution> Project::solutions() const
{
    return mSolutions;
}

void Project::setPanel(Panel const& panel)
{
    mPanel = panel;
}

void Project::setSolutions(QList<Optimizer::Solution> const& solutions)
{
    mSolutions = solutions;
}

/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the Project class
 */

#ifndef PROJECT_H
#define PROJECT_H

#include <QDateTime>
#include <QString>

#include "optimizer.h"
#include "panel.h"

namespace Backend
{

struct Configuration
{
    Configuration();

    QString name;
    Optimizer::State state;
    Properties target;
    Properties weight;
    Optimizer::Options options;
};

class Project
{
public:
    Project();
    ~Project() = default;

    Configuration& configuration();
    Panel const& panel() const;
    QList<Optimizer::Solution> solutions() const;

    void setPanel(Panel const& panel);
    void setSolutions(QList<Optimizer::Solution> const& solutions);

private:
    Configuration mConfiguration;
    Panel mPanel;
    QList<Optimizer::Solution> mSolutions;
};

} // namespace Backend

#endif // PROJECT_H

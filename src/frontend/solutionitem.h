/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Declaration of the SolutionModel class
 */

#ifndef SOLUTIONITEM_H
#define SOLUTIONITEM_H

#include <QStandardItem>

#include "optimizer.h"

namespace Frontend
{

class SolutionItem : public QStandardItem
{
public:
    SolutionItem(Backend::Optimizer::Solution const& solution);
    virtual ~SolutionItem();

    Backend::Optimizer::Solution const& solution() const;

private:
    Backend::Optimizer::Solution const& mSolution;
};

}

#endif // SOLUTIONITEM_H

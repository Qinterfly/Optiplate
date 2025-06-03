/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Declaration of the SolutionModel class
 */

#ifndef SOLUTIONMODEL_H
#define SOLUTIONMODEL_H

#include <QStandardItemModel>

#include "optimizer.h"

namespace Frontend
{

class SolutionModel : public QStandardItemModel
{
public:
    SolutionModel(QList<Backend::Optimizer::Solution> const& solutions, QObject* pParent = nullptr);
    virtual ~SolutionModel();

private:
    QList<Backend::Optimizer::Solution> const& mSolutions;
};

}

#endif // SOLUTIONMODEL_H

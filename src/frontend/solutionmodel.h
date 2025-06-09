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
    SolutionModel(QList<Backend::Optimizer::Solution> const& solutions, Backend::Optimizer::Options const& options, QObject* pParent = nullptr);
    virtual ~SolutionModel();

    QStringList mimeTypes() const override;
    QMimeData* mimeData(QModelIndexList const& indices) const override;

private:
    QList<Backend::Optimizer::Solution> const& mSolutions;
    Backend::Optimizer::Options const& mOptions;
};

}

#endif // SOLUTIONMODEL_H

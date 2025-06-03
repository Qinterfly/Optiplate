/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the SolutionModel class
 */

#include "solutionmodel.h"
#include "uiutility.h"

using namespace Frontend;

SolutionModel::SolutionModel(QList<Backend::Optimizer::Solution> const& solutions, QObject* pParent)
    : QStandardItemModel(pParent)
    , mSolutions(solutions)
{
    QStandardItem* pRootItem = invisibleRootItem();
    for (auto const& solution : solutions)
    {
        double error = solution.errorProperties.maxAbsValidValue() * 100;
        QString name = tr("Iteration %1 → %2 %").arg(QString::number(solution.iteration), QString::number(error, 'f', 3));
        QIcon icon(QString(":/icons/flag-%1.svg").arg(Utility::errorColorName(error)));
        QStandardItem* pItem = new QStandardItem(icon, name);
        pRootItem->appendRow(pItem);
    }
}

SolutionModel::~SolutionModel()
{
}

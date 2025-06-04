/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the SolutionModel class
 */

#include "solutionitem.h"
#include "uiutility.h"

using namespace Frontend;

SolutionItem::SolutionItem(Backend::Optimizer::Solution const& solution)
    : mSolution(solution)
{
    double error = mSolution.errorProperties.maxAbsValidValue() * 100;
    QString name = QObject::tr("Iteration %1 → %2 %").arg(QString::number(mSolution.iteration), QString::number(error, 'f', 3));
    QIcon icon(QString(":/icons/flag-%1.svg").arg(Utility::errorColorName(error)));
    setText(name);
    setIcon(icon);
}

SolutionItem::~SolutionItem()
{
}

Backend::Optimizer::Solution const& SolutionItem::solution() const
{
    return mSolution;
}

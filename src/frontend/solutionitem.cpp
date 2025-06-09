/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the SolutionModel class
 */

#include "solutionitem.h"
#include "uiutility.h"

using namespace Frontend;

SolutionItem::SolutionItem(Backend::Optimizer::Solution const& solution, Backend::Optimizer::Options const& options)
    : mSolution(solution)
    , mOptions(options)
{
    double error = mSolution.errorProperties.maxAbsValidValue();
    QIcon icon(QString(":/icons/flag-%1.svg").arg(Utility::errorColorName(error, mOptions.acceptThreshold, mOptions.criticalThreshold)));
    QString name = QObject::tr("Iteration %1 → %2 %").arg(QString::number(mSolution.iteration), QString::number(error * 100, 'f', 3));
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

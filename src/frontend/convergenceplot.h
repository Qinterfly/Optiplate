/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the ConvergencePlot class
 */

#ifndef CONVERGENCEPLOT_H
#define CONVERGENCEPLOT_H

#include <QWidget>

#include "optimizer.h"

namespace Frontend
{

class CustomPlot;

class ConvergencePlot : public QWidget
{
public:
    ConvergencePlot(QWidget* pParent = nullptr);
    virtual ~ConvergencePlot();

    QSize sizeHint() const override;

    void plot(QList<Backend::Optimizer::Solution> const& solutions);
    void clear();

private:
    void createContent();

private:
    CustomPlot* mpPlot;
};

}

#endif // CONVERGENCEPLOT_H

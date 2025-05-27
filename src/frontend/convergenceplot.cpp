/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the ConvergencePlot class
 */

#include "convergenceplot.h"
#include "customplot.h"

using namespace Frontend;

ConvergencePlot::ConvergencePlot(QWidget* pParent)
    : QWidget(pParent)
{
    createContent();
}

ConvergencePlot::~ConvergencePlot()
{
}

QSize ConvergencePlot::sizeHint() const
{
    return QSize(800, 600);
}

//! Create all the widgets associated with the viewer
void ConvergencePlot::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;

    // Create and configure the plot
    mpPlot = new CustomPlot;

    // Combine all the widgets
    pLayout->addWidget(mpPlot);

    // Set the layout
    setLayout(pLayout);
}

//! Represent solution history
void ConvergencePlot::plot(QList<Backend::Optimizer::Solution> const& solutions)
{
    int numSolutions = solutions.size();
    // TODO

    // Set the axis labels
    mpPlot->xAxis->setLabel(tr("Iteration"));
    mpPlot->yAxis->setLabel(tr("Error, %"));
}

//! Rempve all the content
void ConvergencePlot::clear()
{
    mpPlot->clear();
}

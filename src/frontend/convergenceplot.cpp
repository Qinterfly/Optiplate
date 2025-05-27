/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the ConvergencePlot class
 */

#include "convergenceplot.h"
#include "customplot.h"

using namespace Backend;
using namespace Frontend;

QList<double> sliceMassError(QList<Optimizer::Solution> const& solutions, double factor);
QList<double> sliceVectorError(QList<Optimizer::Solution> const& solutions, KCL::Vec3 Properties::* type, double factor);

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
void ConvergencePlot::plot(QList<Optimizer::Solution> const& solutions, Backend::Optimizer::Options const& options)
{
    double const kFactor = 100.0;

    // Set up the plot
    clear();

    // Slice iteration indices
    int numSolutions = solutions.size();
    QList<double> xIter(numSolutions);
    for (int i = 0; i != numSolutions; ++i)
        xIter[i] = solutions[i].iteration;

    // Slice errors of properties
    auto yMass = sliceMassError(solutions, kFactor);
    auto yGravity = sliceVectorError(solutions, &Properties::centerGravity, kFactor);
    auto yMoments = sliceVectorError(solutions, &Properties::inertiaMoments, kFactor);
    auto yProducts = sliceVectorError(solutions, &Properties::inertiaProducts, kFactor);

    // Add the graphs
    addGraph(xIter, yMass, tr("Mass"), Qt::red);
    addGraph(xIter, yGravity, tr("Center of gravity"), Qt::green);
    addGraph(xIter, yMoments, tr("Inertia moments"), Qt::blue);
    addGraph(xIter, yProducts, tr("Inertia products"), Qt::cyan);

    // Create the criterion line
    addCriterionLine(xIter.first(), xIter.last(), options.maxRelativeError * kFactor);

    // Display the legend
    mpPlot->legend->setVisible(true);

    // Set the axis labels
    mpPlot->xAxis->setLabel(tr("Iteration"));
    mpPlot->yAxis->setLabel(tr("Relative error, %"));

    // Set the ticks
    QSharedPointer<QCPAxisTickerFixed> pXTicker(new QCPAxisTickerFixed);
    pXTicker->setTickStep(1.0);
    mpPlot->xAxis->setTicker(pXTicker);
    QSharedPointer<QCPAxisTickerLog> pYTicker(new QCPAxisTickerLog);
    mpPlot->yAxis->setTicker(pYTicker);

    // Set the scale of axes
    mpPlot->yAxis->grid()->setSubGridVisible(true);
    mpPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    mpPlot->yAxis->setNumberFormat("eb");
    mpPlot->yAxis->setNumberPrecision(0);

    // Draw the result
    mpPlot->replot();
}

//! Remove all the content
void ConvergencePlot::clear()
{
    mpPlot->clear();
}

//! Create the graph and visualize it
void ConvergencePlot::addGraph(QList<double> const& xData, QList<double> const& yData, QString const& name, QColor const& color)
{
    int const kMarkerSize = 8;

    // Create a new graph
    QCPGraph* pGraph = mpPlot->addGraph();
    pGraph->setData(xData, yData, false);
    pGraph->setName(name);
    pGraph->setSelectable(QCP::SelectionType::stSingleData);
    pGraph->setAdaptiveSampling(true);
    pGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, kMarkerSize));

    // Set the graph style
    pGraph->setPen(QPen(color));

    // Rescale the axes, so that all the graphs fit
    bool isEnlarge = mpPlot->graphCount() > 1;
    pGraph->rescaleAxes(isEnlarge);
}

//! Create the line to display convergence criterion
void ConvergencePlot::addCriterionLine(double xMin, double xMax, double value)
{
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(1.0);
    pen.setStyle(Qt::DashLine);
    QCPCurve* pCurve = new QCPCurve(mpPlot->xAxis, mpPlot->yAxis);
    pCurve->addData(xMin, value);
    pCurve->addData(xMax, value);
    pCurve->setPen(pen);
    pCurve->setName(tr("Criterion"));
}

//! Helper function to slice errors in masses associated with iterations
QList<double> sliceMassError(QList<Optimizer::Solution> const& solutions, double factor)
{
    int numSolutions = solutions.size();
    QList<double> result(numSolutions);
    for (int i = 0; i != numSolutions; ++i)
        result[i] = qAbs(solutions[i].errorProperties.mass) * factor;
    return result;
}

//! Helper function to slice errors in vector properties associated with iterations
QList<double> sliceVectorError(QList<Optimizer::Solution> const& solutions, KCL::Vec3 Properties::* type, double factor)
{
    int numSolutions = solutions.size();
    QList<double> result(numSolutions);
    for (int i = 0; i != numSolutions; ++i)
    {
        auto const& data = solutions[i].errorProperties.*type;
        double maxValue = -std::numeric_limits<double>::infinity();
        int numData = data.size();
        for (int j = 0; j != numData; ++j)
            maxValue = qMax(maxValue, qAbs(data[j]));
        result[i] = maxValue * factor;
    }
    return result;
}

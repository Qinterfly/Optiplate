/*!
 * \file
 * \author Pavel Lakiza
 * \date November 2023
 * \brief Implementation of the DataTip class
 */

#include "datatip.h"
#include "customplot.h"

using namespace Frontend;

QString format(double value)
{
    return QString::number(value, 'g', 3);
}

DataTip::DataTip(double xData, double yData, CustomPlot* pPlot)
    : QCPItemText(pPlot)
    , mXData(xData)
    , mYData(yData)
    , mIsDrag(false)
    , mpPlot(pPlot)
{
    // Constants
    QColor const kBrushColor = QColor(255, 255, 255, 200);
    QColor const kPenColor = Qt::black;
    QColor const kMarkerColor = QColor(0, 115, 200);
    uint const kMarkerSize = 8;
    uint const kFontSize = 14;
    QMargins const kPadding(4, 0, 4, 0);

    // Set position
    position->setType(QCPItemPosition::ptPlotCoords);
    position->setCoords(xData, yData);
    setPositionAlignment(Qt::AlignLeft | Qt::AlignBottom);

    // Set style
    setPadding(kPadding);
    setBrush(QBrush(kBrushColor));
    QPen pen(kPenColor);
    pen.setJoinStyle(Qt::RoundJoin);
    setPen(pen);

    // Set text
    setTextAlignment(Qt::AlignLeft);
    setFont(QFont(font().family(), kFontSize));
    QString text = QString("X: %1\nY: %2").arg(format(xData), format(yData));
    setText(text);

    // Add the tracer point
    mpCurve = new QCPCurve(mpPlot->xAxis, mpPlot->yAxis);
    mpCurve->addData(xData, yData);
    mpCurve->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, kMarkerColor, kMarkerSize));
    mpCurve->removeFromLegend();
}

DataTip::~DataTip()
{
    mpPlot->removePlottable(mpCurve);
}

bool DataTip::isDrag() const
{
    return mIsDrag;
}

bool DataTip::isSelect(QPointF const& position, int tolerance)
{
    double distance = selectTest(position, false);
    return distance > 0 && distance <= 0.99 * tolerance;
}

void DataTip::startDrag(QPointF const& origin)
{
    mDragOrigin = origin;
    mIsDrag = true;
}

void DataTip::processDrag(QPointF const& point)
{
    if (!mIsDrag)
        return;
    float shiftX = point.x() - mDragOrigin.x();
    float shiftY = point.y() - mDragOrigin.y();
    Qt::AlignmentFlag horizontalFlag = positionAlignment().testFlag(Qt::AlignLeft) ? Qt::AlignLeft : Qt::AlignRight;
    Qt::AlignmentFlag verticalFlag = positionAlignment().testFlag(Qt::AlignTop) ? Qt::AlignTop : Qt::AlignBottom;
    if (std::abs(shiftX) > std::abs(shiftY))
        horizontalFlag = shiftX > 0 ? Qt::AlignLeft : Qt::AlignRight;
    else
        verticalFlag = shiftY > 0 ? Qt::AlignTop : Qt::AlignBottom;
    setPositionAlignment(horizontalFlag | verticalFlag);
}

void DataTip::finishDrag()
{
    mIsDrag = false;
}

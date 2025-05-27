/*!
 * \file
 * \author Pavel Lakiza
 * \date November 2023
 * \brief Declaration of the DataTip class
 */

#ifndef DATATIP_H
#define DATATIP_H

#include <qcustomplot.h>

namespace Frontend
{

class CustomPlot;

class DataTip : public QCPItemText
{
public:
    DataTip(double xData, double yData, CustomPlot* pPlot);
    virtual ~DataTip();

    void startDrag(QPointF const& origin);
    void processDrag(QPointF const& point);
    void finishDrag();
    bool isDrag() const;
    bool isSelect(QPointF const& position, int tolerance);

private:
    double mXData;
    double mYData;
    bool mIsDrag;
    QPointF mDragOrigin;
    CustomPlot* mpPlot;
    QCPCurve* mpCurve;
};

}

#endif // DATATIP_H

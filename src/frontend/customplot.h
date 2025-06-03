/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Declaration of the CustomPlot class
 */

#ifndef CUSTOMPLOT_H
#define CUSTOMPLOT_H

#include <qcustomplot.h>


namespace Frontend
{

class DataTip;

class CustomPlot : public QCustomPlot
{
    Q_OBJECT

public:
    CustomPlot(QWidget* pParent = nullptr);
    virtual ~CustomPlot();

    void clear();
    void refresh();
    void fit();

    QString title() const;
    void setTitle(QString const& title);
    QColor getAvailableColor() const;

signals:
    void dataDropped(QList<QStandardItem*> items);

protected:
    void keyPressEvent(QKeyEvent* pEvent) override;

private:
    // Create content and connections
    void initializePlot();
    void createActions();
    void specifyConnections();

    // State
    QList<QCPAbstractPlottable*> selectedPlottables();

    // Modifying entities
    void removeSelectedPlottables();
    void removeAllDataTips();
    void removeDataTip(uint index);
    void changeSelectedPlottable();
    void renameAxis(QCPAxis* pAxis, QCPAxis::SelectablePart part);
    void renamePlottable(QCPLegend* pLegend, QCPAbstractLegendItem* pItem);
    void renameTitle(QMouseEvent* pEvent);

    // Slots handling
    void processSelection();
    void processMousePress(QMouseEvent* pEvent);
    void processMouseWheel(QWheelEvent* pEvent);
    void processMouseMove(QMouseEvent* pEvent);
    void processMouseRelease(QMouseEvent* pEvent);
    void processBeforeReplot();
    void showContextMenu(QPoint position);
    void showDataTip(double xData, double yData);
    void copyPixmapToClipboard();
    void savePixmap();
    void viewPlotData();
    QPointF mapToLocal(QPointF const& position) const;

    // Legend
    bool mIsDragLegend;
    QPointF mDragLegendOrigin;

    // Items
    QList<DataTip*> mDataTips;

    // Elements
    QCPTextElement* mpTitle;
};

}

#endif // CUSTOMPLOT_H

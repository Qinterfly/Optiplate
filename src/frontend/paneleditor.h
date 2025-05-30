/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the PanelEditor class
 */

#ifndef PANELEDITOR_H
#define PANELEDITOR_H

#include <QWidget>

#include "panel.h"

class CustomDoubleSpinBox;

namespace Frontend
{

//! Widget to edit panel data
class PanelEditor : public QWidget
{
    Q_OBJECT

public:
    enum DataType
    {
        kDensity,
        kXCoords,
        kZCoords,
        kDepths
    };
    PanelEditor(Backend::Panel& panel, QWidget* pParent = nullptr);
    virtual ~PanelEditor();

    void update();

signals:
    void dataChanged();

private:
    // Content
    void createContent();
    CustomDoubleSpinBox* createDoubleSpinBox(QPair<double, double> const& limits, int numDecimals);

    // Signals & slots
    void processDataChange();

private:
    Backend::Panel& mPanel;
};

} // namespace Frontend

#endif // PANELEDITOR_H

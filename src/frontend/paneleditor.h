/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Declaration of the PanelEditor class
 */

#ifndef PANELEDITOR_H
#define PANELEDITOR_H

#include <QWidget>

#include "panel.h"

QT_FORWARD_DECLARE_CLASS(QGridLayout)

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

    QSize sizeHint() const override;
    void update();

signals:
    void dataChanged();

protected:
    void dragEnterEvent(QDragEnterEvent* pEvent) override;
    void dropEvent(QDropEvent* pEvent) override;

private:
    // Content
    void createContent();
    CustomDoubleSpinBox* createDoubleSpinBox(QPair<double, double> const& limits, int numDecimals);

    // Signals & slots
    void processDataChange();
    void processExport();
    void processRound();

private:
    Backend::Panel& mPanel;
    QGridLayout* mpDataLayout;
    static QString mLastPath;
};

} // namespace Frontend

#endif // PANELEDITOR_H

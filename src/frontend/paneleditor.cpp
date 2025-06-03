/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the PanelEditor class
 */

#include <QLabel>
#include <QVBoxLayout>

#include "paneleditor.h"
#include "qtpropertybrowser/customdoublespinbox.h"

using namespace Frontend;
using namespace Backend;

PanelEditor::PanelEditor(Backend::Panel& panel, QWidget* pParent)
    : QWidget(pParent)
    , mPanel(panel)
{
    createContent();
    update();
}

PanelEditor::~PanelEditor()
{
}

QSize PanelEditor::sizeHint() const
{
    return QSize(325, 100);
}

//! Create all the widgets and corresponding actions
void PanelEditor::createContent()
{
    // Constants
    int kNumDecimals = 5;
    QPair<double, double> kLimitsDensity = {0, 1e5};
    QPair<double, double> kLimitsCoords = {-100, 100};
    QPair<double, double> kLimitsDepths = {0, 1};

    // Create the layout
    QGridLayout* pLayout = new QGridLayout;

    // Create the widgets
    // Density
    pLayout->addWidget(new QLabel(tr("Density:")), kDensity, 0);
    pLayout->addWidget(createDoubleSpinBox(kLimitsDensity, kNumDecimals), kDensity, 1);
    // Coordinates
    pLayout->addWidget(new QLabel(tr("X coordinates:")), kXCoords, 0);
    pLayout->addWidget(new QLabel(tr("Z coordinates:")), kZCoords, 0);
    int numCoordinates = mPanel.xCoords().size();
    for (int i = 0; i != numCoordinates; ++i)
    {
        pLayout->addWidget(createDoubleSpinBox(kLimitsCoords, kNumDecimals), kXCoords, 1 + i);
        pLayout->addWidget(createDoubleSpinBox(kLimitsCoords, kNumDecimals), kZCoords, 1 + i);
    }
    // Depths
    pLayout->addWidget(new QLabel(tr("Depths:")), kDepths, 0);
    int numDepths = mPanel.depths().size();
    for (int i = 0; i != numDepths; ++i)
        pLayout->addWidget(createDoubleSpinBox(kLimitsDepths, kNumDecimals), kDepths, 1 + i);

    // Prevent stretching
    pLayout->setRowStretch(pLayout->rowCount() + 1, 1);
    pLayout->setColumnStretch(pLayout->columnCount() + 1, 1);

    // Set the resulting layout
    setLayout(pLayout);
}

//! Update values of widgets
void PanelEditor::update()
{
    QGridLayout* pLayout = (QGridLayout*) layout();

    // Create the setter
    auto setValue = [&pLayout](double value, int iRow, int iColumn)
    {
        CustomDoubleSpinBox* pSpinBox = (CustomDoubleSpinBox*) pLayout->itemAtPosition(iRow, iColumn)->widget();
        pSpinBox->blockSignals(true);
        pSpinBox->setValue(value);
        pSpinBox->blockSignals(false);
    };

    // Refresh the density
    setValue(mPanel.density(), kDensity, 1);

    // Refresh the coordinates
    int numCoordinates = mPanel.xCoords().size();
    for (int i = 0; i != numCoordinates; ++i)
    {
        setValue(mPanel.xCoords()[i], kXCoords, 1 + i);
        setValue(mPanel.zCoords()[i], kZCoords, 1 + i);
    }

    // Refresh the depths
    int numDepths = mPanel.depths().size();
    for (int i = 0; i != numDepths; ++i)
        setValue(mPanel.depths()[i], kDepths, 1 + i);
}

CustomDoubleSpinBox* PanelEditor::createDoubleSpinBox(QPair<double, double> const& limits, int numDecimals)
{
    CustomDoubleSpinBox* pSpinBox = new CustomDoubleSpinBox;
    pSpinBox->setMinimum(limits.first);
    pSpinBox->setMaximum(limits.second);
    pSpinBox->setDecimals(numDecimals);
    pSpinBox->setStepType(CustomDoubleSpinBox::AdaptiveDecimalStepType);
    pSpinBox->setWheelEnabled(false);
    connect(pSpinBox, &QDoubleSpinBox::valueChanged, this, [this](double) { processDataChange(); });
    return pSpinBox;
}

//! Process changing data of a panel
void PanelEditor::processDataChange()
{
    QGridLayout* pLayout = (QGridLayout*) layout();

    // Create the getter
    auto getValue = [&pLayout](int iRow, int iColumn)
    {
        CustomDoubleSpinBox* pSpinBox = (CustomDoubleSpinBox*) pLayout->itemAtPosition(iRow, iColumn)->widget();
        return pSpinBox->value();
    };

    // Modify the panel density
    mPanel.setDensity(getValue(kDensity, 1));

    // Modify the panel coordinates
    KCL::Vec4 xCoords = mPanel.xCoords();
    KCL::Vec4 zCoords = mPanel.zCoords();
    int numCoordinates = xCoords.size();
    for (int i = 0; i != numCoordinates; ++i)
    {
        xCoords[i] = getValue(kXCoords, 1 + i);
        zCoords[i] = getValue(kZCoords, 1 + i);
    }
    mPanel.setXCoords(xCoords);
    mPanel.setZCoords(zCoords);

    // Modify the panel depths
    KCL::Vec3 depths = mPanel.depths();
    int numDepths = xCoords.size();
    for (int i = 0; i != depths.size(); ++i)
        depths[i] = getValue(kDepths, 1 + i);
    mPanel.setDepths(depths);

    emit dataChanged();
}

/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Implementation of the PropertiesEditor class
 */

#include <qtpropertybrowser/customdoublespinbox.h>
#include <QGridLayout>
#include <QLabel>

#include "propertieseditor.h"

using namespace Frontend;

PropertiesEditor::PropertiesEditor(PropertyType type, Backend::Properties& properties, QWidget* pParent)
    : QWidget(pParent)
    , mProperties(properties)
{
    createContent(type);
}

PropertiesEditor::~PropertiesEditor()
{
}

QSize PropertiesEditor::sizeHint() const
{
    return QSize(500, 125);
}

//! Create all the widgets and corresponding actions
void PropertiesEditor::createContent(PropertyType type)
{
    // Constants
    double const kInf = 1e6;

    // Define data limits
    int numDecimals;
    QPair<double, double> limitsMass, limitsCenterGravity, limitsInertia;
    switch (type)
    {
    case PropertyType::kTarget:
        numDecimals = 5;
        limitsMass = {0, kInf};
        limitsCenterGravity = {-kInf, kInf};
        limitsInertia = {-kInf, kInf};
        break;
    case PropertyType::kWeight:
        numDecimals = 2;
        limitsMass = {0, 1e3};
        limitsCenterGravity = limitsMass;
        limitsInertia = limitsMass;
        break;
    }

    // Create the main layout
    QGridLayout* pLayout = new QGridLayout;

    // Create the labels
    pLayout->addWidget(new QLabel(tr("Mass:")), kMass, 0);
    pLayout->addWidget(new QLabel(tr("Gravity center:")), kCenterGravity, 0);
    pLayout->addWidget(new QLabel(tr("Inertia moments:")), kInertiaMoments, 0);
    pLayout->addWidget(new QLabel(tr("Inertia products:")), kInertiaProducts, 0);

    // Create the spinboxes
    pLayout->addWidget(createDoubleSpinBox(limitsMass, numDecimals), kMass, 1);
    int numDirections = mProperties.centerGravity.size();
    for (int i = 0; i != numDirections; ++i)
    {
        pLayout->addWidget(createDoubleSpinBox(limitsCenterGravity, numDecimals), kCenterGravity, 1 + i);
        pLayout->addWidget(createDoubleSpinBox(limitsInertia, numDecimals), kInertiaMoments, 1 + i);
        pLayout->addWidget(createDoubleSpinBox(limitsInertia, numDecimals), kInertiaProducts, 1 + i);
    }

    // Prevent stretching
    pLayout->setRowStretch(pLayout->rowCount() + 1, 1);
    pLayout->setColumnStretch(pLayout->columnCount() + 1, 1);

    // Set the resulting layout
    setLayout(pLayout);
}

//! Update values of widgets
void PropertiesEditor::update()
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

    // Refresh the values
    setValue(mProperties.mass, kMass, 1);
    int numDirections = mProperties.centerGravity.size();
    for (int i = 0; i != numDirections; ++i)
    {
        setValue(mProperties.centerGravity[i], kCenterGravity, 1 + i);
        setValue(mProperties.inertiaMoments[i], kInertiaMoments, 1 + i);
        setValue(mProperties.inertiaProducts[i], kInertiaProducts, 1 + i);
    }
}

//! Create a widget to represent a numeric value
CustomDoubleSpinBox* PropertiesEditor::createDoubleSpinBox(QPair<double, double> const& limits, int numDecimals)
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
void PropertiesEditor::processDataChange()
{
    QGridLayout* pLayout = (QGridLayout*) layout();

    // Create the getter
    auto getValue = [&pLayout](int iRow, int iColumn)
    {
        CustomDoubleSpinBox* pSpinBox = (CustomDoubleSpinBox*) pLayout->itemAtPosition(iRow, iColumn)->widget();
        return pSpinBox->value();
    };

    // Modify the properties
    mProperties.mass = getValue(kMass, 1);
    int numDirections = mProperties.centerGravity.size();
    for (int i = 0; i != numDirections; ++i)
    {
        mProperties.centerGravity[i] = getValue(kCenterGravity, 1 + i);
        mProperties.inertiaMoments[i] = getValue(kInertiaMoments, 1 + i);
        mProperties.inertiaProducts[i] = getValue(kInertiaProducts, 1 + i);
    }

    emit dataChanged();
}

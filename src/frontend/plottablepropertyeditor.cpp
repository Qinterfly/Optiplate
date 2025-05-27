/*!
 * \file
 * \author Pavel Lakiza
 * \date December 2023
 * \brief Implementation of the PlottablePropertyEditor class
 */

#include <qcustomplot.h>
#include <qtpropertymanager.h>
#include <qteditorfactory.h>

#include "plottablepropertyeditor.h"

using namespace Frontend;

static const char* skLineStyle = "lineStyle";
static const char* skScatterStyle = "scatterStyle";

PlottablePropertyEditor::PlottablePropertyEditor(QCPAbstractPlottable* pPlottable, QWidget* pParent)
    : CustomPropertyEditor(pParent)
    , mpPlottable(pPlottable)
{
    createManagers();
    createFactories();
    createProperties();
}

PlottablePropertyEditor::~PlottablePropertyEditor()
{
    unsetFactoryForManager(mpEnumManager);
    unsetFactoryForManager(mpColorManager);
    unsetFactoryForManager(mpDoubleManager);

    delete mpEnumEditorFactory;
    delete mpColorEditorFactory;
    delete mpDoubleSpinBoxFactory;

    delete mpEnumManager;
    delete mpColorManager;
    delete mpDoubleManager;
}

QSize PlottablePropertyEditor::sizeHint() const
{
    return QSize(400, 300);
}

void PlottablePropertyEditor::createManagers()
{
    mpEnumManager = new QtEnumPropertyManager;
    mpColorManager = new QtColorPropertyManager;
    mpDoubleManager = new QtDoublePropertyManager;

    connect(mpEnumManager, &QtEnumPropertyManager::valueChanged, this, &PlottablePropertyEditor::setIntValue);
    connect(mpColorManager, &QtColorPropertyManager::valueChanged, this, &PlottablePropertyEditor::setColorValue);
    connect(mpDoubleManager, &QtDoublePropertyManager::valueChanged, this, &PlottablePropertyEditor::setDoubleValue);
}

void PlottablePropertyEditor::createFactories()
{
    mpEnumEditorFactory = new QtEnumEditorFactory;
    mpColorEditorFactory = new QtColorEditorFactory;
    mpDoubleSpinBoxFactory = new QtDoubleSpinBoxFactory;

    setFactoryForManager(mpEnumManager, mpEnumEditorFactory);
    setFactoryForManager(mpColorManager, mpColorEditorFactory);
    setFactoryForManager(mpDoubleManager, mpDoubleSpinBoxFactory);
}

void PlottablePropertyEditor::createProperties()
{
    QStringList kMarkerShapeNames =
    {
        QString(),
        tr("Dot"), tr("Cross"), tr("Plus"),
        tr("Circle"), tr("Disc"), tr("Square"),
        tr("Diamond"), tr("Star"), tr("Triangle"),
        tr("Inverted Triangle"), tr("Cross Square"), tr("Plus Square"),
        tr("Cross Circle"), tr("Plus Circle"), tr("Peace"),
        tr("Pixmap"), tr("Custom")
    };

    QStringList kLineStyleNames =
    {
        QString(),
        tr("Line"), tr("Left step"), tr("Right step"),
        tr("Center step"), tr("Impulse")
    };

    // Slice the plottable data
    int lineStyle = mpPlottable->property(skLineStyle).toInt();
    QCPScatterStyle scatterStyle = mpPlottable->property(skScatterStyle).value<QCPScatterStyle>();

    QtProperty* pProperty;

    // Line style
    pProperty = mpEnumManager->addProperty(tr("Style"));
    mpEnumManager->setEnumNames(pProperty, kLineStyleNames);
    mpEnumManager->setValue(pProperty, lineStyle);
    addProperty(pProperty, kLineStyle);

    // Line color
    pProperty = mpColorManager->addProperty(tr("Line color"));
    mpColorManager->setValue(pProperty, mpPlottable->pen().color());
    addProperty(pProperty, kLineColor, nullptr, false);

    // Marker shape
    pProperty = mpEnumManager->addProperty(tr("Marker shape"));
    mpEnumManager->setEnumNames(pProperty, kMarkerShapeNames);
    mpEnumManager->setValue(pProperty, scatterStyle.shape());
    addProperty(pProperty, kMarkerShape);

    // Marker size
    pProperty = mpDoubleManager->addProperty(tr("Marker size"));
    mpDoubleManager->setValue(pProperty, scatterStyle.size());
    addProperty(pProperty, kMarkerSize);

    // Marker color
    pProperty = mpColorManager->addProperty(tr("Marker color"));
    mpColorManager->setValue(pProperty, scatterStyle.pen().color());
    addProperty(pProperty, kMarkerColor, nullptr, false);
}

//! Process changing of an integer value
void PlottablePropertyEditor::setIntValue(QtProperty* pProperty, int value)
{
    if (!mPropertyToID.contains(pProperty))
        return;

    QCPScatterStyle scatterStyle = mpPlottable->property(skScatterStyle).value<QCPScatterStyle>();
    int id = mPropertyToID[pProperty];
    switch (id)
    {
    case kLineStyle:
        mpPlottable->setProperty(skLineStyle, value);
        break;
    case kMarkerShape:
        scatterStyle.setShape((QCPScatterStyle::ScatterShape)value);
        break;
    }
    mpPlottable->setProperty(skScatterStyle, QVariant::fromValue(scatterStyle));
}

//! Process changing of a color
void PlottablePropertyEditor::setColorValue(QtProperty* pProperty, QColor const& value)
{
    if (!mPropertyToID.contains(pProperty))
        return;

    QCPScatterStyle scatterStyle = mpPlottable->property(skScatterStyle).value<QCPScatterStyle>();
    int id = mPropertyToID[pProperty];
    switch (id)
    {
    case kLineColor:
        mpPlottable->setPen(QPen(value));
        break;
    case kMarkerColor:
        scatterStyle.setPen(QPen(value));
        break;
    }
    mpPlottable->setProperty(skScatterStyle, QVariant::fromValue(scatterStyle));
}

//! Process changing of a double value
void PlottablePropertyEditor::setDoubleValue(QtProperty* pProperty, double value)
{
    if (!mPropertyToID.contains(pProperty))
        return;

    QCPScatterStyle scatterStyle = mpPlottable->property(skScatterStyle).value<QCPScatterStyle>();
    int id = mPropertyToID[pProperty];
    if (id == kMarkerSize)
        scatterStyle.setSize(value);
    mpPlottable->setProperty(skScatterStyle, QVariant::fromValue(scatterStyle));
}

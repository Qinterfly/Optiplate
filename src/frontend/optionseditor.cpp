/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the OptionsEditor class
 */

#include <qteditorfactory.h>
#include <qtpropertymanager.h>
#include <QApplication>

#include "optionseditor.h"

using namespace Frontend;

OptionsEditor::OptionsEditor(Backend::Optimizer::Options& options, QWidget* pParent)
    : CustomPropertyEditor(pParent)
    , mOptions(options)
{
    setTreeWidgetFont(qApp->font());
    createManagers();
    createFactories();
    createProperties();
}

OptionsEditor::~OptionsEditor()
{
    unsetFactoryForManager(mpBoolManager);
    unsetFactoryForManager(mpIntManager);
    unsetFactoryForManager(mpDoubleManager);

    delete mpCheckBoxFactory;
    delete mpSpinBoxFactory;
    delete mpDoubleSpinBoxFactory;

    delete mpBoolManager;
    delete mpIntManager;
    delete mpDoubleManager;
}

void OptionsEditor::update()
{
    clear();
    mpBoolManager->clear();
    mpIntManager->clear();
    mpDoubleManager->clear();
    createProperties();
}

//! Create the managers to handle creating different types of properties
void OptionsEditor::createManagers()
{
    mpBoolManager = new QtBoolPropertyManager;
    mpIntManager = new QtIntPropertyManager;
    mpDoubleManager = new QtDoublePropertyManager;

    connect(mpBoolManager, &QtBoolPropertyManager::valueChanged, this, &OptionsEditor::setBoolValue);
    connect(mpIntManager, &QtIntPropertyManager::valueChanged, this, &OptionsEditor::setIntValue);
    connect(mpDoubleManager, &QtDoublePropertyManager::valueChanged, this, &OptionsEditor::setDoubleValue);
}

//! Construct the factories to set the delegates of the items
void OptionsEditor::createFactories()
{
    mpCheckBoxFactory = new QtCheckBoxFactory;
    mpSpinBoxFactory = new QtSpinBoxFactory;
    mpDoubleSpinBoxFactory = new QtDoubleSpinBoxFactory;

    setFactoryForManager(mpBoolManager, mpCheckBoxFactory);
    setFactoryForManager(mpIntManager, mpSpinBoxFactory);
    setFactoryForManager(mpDoubleManager, mpDoubleSpinBoxFactory);
}

//! Create the properties to view and edit
void OptionsEditor::createProperties()
{
    // Add the properties
    QtProperty* pProperty;

    // Type
    pProperty = mpBoolManager->addProperty(tr("Autoscale"));
    mpBoolManager->setValue(pProperty, mOptions.autoScale);
    addProperty(pProperty, kAutoScale);

    // Maximum number of iterations
    pProperty = mpIntManager->addProperty(tr("Max. number of iterations"));
    mpIntManager->setValue(pProperty, mOptions.maxNumIterations);
    mpIntManager->setMinimum(pProperty, 1);
    addProperty(pProperty, kMaxNumIterations);

    // Timeout iteration
    pProperty = mpDoubleManager->addProperty(tr("Timeout iteration"));
    mpDoubleManager->setValue(pProperty, mOptions.timeoutIteration);
    mpDoubleManager->setMinimum(pProperty, 0.0);
    addProperty(pProperty, kTimeoutIteration);

    // Number of threads
    pProperty = mpIntManager->addProperty(tr("Number of threads"));
    mpIntManager->setValue(pProperty, mOptions.numThreads);
    mpIntManager->setMinimum(pProperty, 1);
    addProperty(pProperty, kNumThreads);

    // Maximum relative error
    pProperty = mpDoubleManager->addProperty(tr("Max. relative error"));
    mpDoubleManager->setValue(pProperty, mOptions.maxRelativeError);
    mpDoubleManager->setDecimals(pProperty, 5);
    mpDoubleManager->setMinimum(pProperty, 0.0);
    addProperty(pProperty, kMaxRelativeError);

    // Step size for Jacobian
    pProperty = mpDoubleManager->addProperty(tr("Diff step size"));
    mpDoubleManager->setValue(pProperty, mOptions.diffStepSize);
    mpDoubleManager->setDecimals(pProperty, 7);
    mpDoubleManager->setMinimum(pProperty, 1e-7);
    addProperty(pProperty, kDiffStepSize);

    // Accept threshold
    pProperty = mpDoubleManager->addProperty(tr("Accept threshold (green)"));
    mpDoubleManager->setValue(pProperty, mOptions.acceptThreshold);
    mpDoubleManager->setDecimals(pProperty, 3);
    mpDoubleManager->setMinimum(pProperty, 0);
    addProperty(pProperty, kAcceptThreshold);

    // Critical threshold
    pProperty = mpDoubleManager->addProperty(tr("Critical threshold (red)"));
    mpDoubleManager->setValue(pProperty, mOptions.criticalThreshold);
    mpDoubleManager->setDecimals(pProperty, 3);
    mpDoubleManager->setMinimum(pProperty, 0);
    addProperty(pProperty, kCriticalThreshold);
}

//! Process changing of a boolean value
void OptionsEditor::setBoolValue(QtProperty* pProperty, bool value)
{
    if (!mPropertyToID.contains(pProperty))
        return;

    int id = mPropertyToID[pProperty];
    if (id == kAutoScale)
        mOptions.autoScale = value;

    emit propertyChanged();
}

//! Process changing of an integer value
void OptionsEditor::setIntValue(QtProperty* pProperty, int value)
{
    if (!mPropertyToID.contains(pProperty))
        return;

    int id = mPropertyToID[pProperty];
    switch (id)
    {
    case kMaxNumIterations:
        mOptions.maxNumIterations = value;
        break;
    case kNumThreads:
        mOptions.numThreads = value;
        break;
    }

    emit propertyChanged();
}

//! Process changing of a double value
void OptionsEditor::setDoubleValue(QtProperty* pProperty, double value)
{
    if (!mPropertyToID.contains(pProperty))
        return;

    int id = mPropertyToID[pProperty];
    switch (id)
    {
    case kTimeoutIteration:
        mOptions.timeoutIteration = value;
        break;
    case kMaxRelativeError:
        mOptions.maxRelativeError = value;
        break;
    case kDiffStepSize:
        mOptions.diffStepSize = value;
        break;
    case kAcceptThreshold:
        mOptions.acceptThreshold = value;
        if (value > mOptions.criticalThreshold)
        {
            mOptions.criticalThreshold = value;
            mpDoubleManager->setValue(mIDToProperty[kCriticalThreshold], value);
        }
        break;
    case kCriticalThreshold:
        mOptions.criticalThreshold = value;
        if (value < mOptions.acceptThreshold)
        {
            mOptions.acceptThreshold = value;
            mpDoubleManager->setValue(mIDToProperty[kAcceptThreshold], value);
        }
        break;
    }

    emit propertyChanged();
}

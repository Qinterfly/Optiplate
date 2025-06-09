/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the OptionsEditor class
 */

#ifndef OPTIONSEDITOR_H
#define OPTIONSEDITOR_H

#include "custompropertyeditor.h"
#include "optimizer.h"

class QtBoolPropertyManager;
class QtIntPropertyManager;
class QtDoublePropertyManager;

class QtCheckBoxFactory;
class QtSpinBoxFactory;
class QtDoubleSpinBoxFactory;

namespace Frontend
{

class OptionsEditor : public CustomPropertyEditor
{
    Q_OBJECT

public:
    enum Type
    {
        kAutoScale,
        kMaxNumIterations,
        kTimeoutIteration,
        kNumThreads,
        kMaxRelativeError,
        kDiffStepSize,
        kAcceptThreshold,
        kCriticalThreshold
    };

    OptionsEditor(Backend::Optimizer::Options& options, QWidget* pParent = nullptr);
    virtual ~OptionsEditor();

    void update();

private:
    void createManagers();
    void createFactories();
    void createProperties();

    void setBoolValue(QtProperty* pProperty, bool value);
    void setIntValue(QtProperty* pProperty, int value);
    void setDoubleValue(QtProperty* pProperty, double value);

private:
    Backend::Optimizer::Options& mOptions;

    QtBoolPropertyManager* mpBoolManager;
    QtIntPropertyManager* mpIntManager;
    QtDoublePropertyManager* mpDoubleManager;

    QtCheckBoxFactory* mpCheckBoxFactory;
    QtSpinBoxFactory* mpSpinBoxFactory;
    QtDoubleSpinBoxFactory* mpDoubleSpinBoxFactory;
};

}

#endif // OPTIONSEDITOR_H

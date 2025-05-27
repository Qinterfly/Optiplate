/*!
 * \file
 * \author Pavel Lakiza
 * \date December 2023
 * \brief Declaration of the PlottablePropertyEditor class
 */

#ifndef PLOTTABLEPROPERTYEDITOR_H
#define PLOTTABLEPROPERTYEDITOR_H

#include "custompropertyeditor.h"

class QCPAbstractPlottable;
class QtColorPropertyManager;
class QtEnumPropertyManager;
class QtDoublePropertyManager;

class QtEnumEditorFactory;
class QtColorEditorFactory;
class QtDoubleSpinBoxFactory;

namespace Frontend
{

class PlottablePropertyEditor : public CustomPropertyEditor
{
    Q_OBJECT

public:
    enum Type
    {
        kLineStyle,
        kLineColor,
        kMarkerColor,
        kMarkerSize,
        kMarkerShape
    };

    PlottablePropertyEditor(QCPAbstractPlottable* pPlottable, QWidget* pParent = nullptr);
    virtual ~PlottablePropertyEditor();

protected:
    QSize sizeHint() const override;

private:
    void createManagers();
    void createFactories();

    void createProperties();

    void setIntValue(QtProperty* pProperty, int value);
    void setColorValue(QtProperty* pProperty, QColor const& value);
    void setDoubleValue(QtProperty* pProperty, double value);

    QCPAbstractPlottable* mpPlottable;

    QtEnumPropertyManager* mpEnumManager;
    QtColorPropertyManager* mpColorManager;
    QtDoublePropertyManager* mpDoubleManager;

    QtEnumEditorFactory* mpEnumEditorFactory;
    QtColorEditorFactory* mpColorEditorFactory;
    QtDoubleSpinBoxFactory* mpDoubleSpinBoxFactory;
};

}

#endif // PLOTTABLEPROPERTYEDITOR_H

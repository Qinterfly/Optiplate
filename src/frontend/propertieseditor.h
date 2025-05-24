/*!
 * \file
 * \author Pavel Lakiza
 * \date May 2025
 * \brief Declaration of the PropertiesEditor class
 */

#ifndef PROPERTIESEDITOR_H
#define PROPERTIESEDITOR_H

#include <QWidget>

#include "properties.h"

class CustomDoubleSpinBox;

namespace Frontend
{

enum class PropertyType
{
    kTarget,
    kWeight
};

//! Widget to edit panel properties
class PropertiesEditor : public QWidget
{
    Q_OBJECT

public:
    enum DataType
    {
        kMass,
        kCenterGravity,
        kInertiaMoments,
        kInertiaProducts
    };
    PropertiesEditor(PropertyType type, Backend::Properties& properties, QWidget* pParent = nullptr);
    virtual ~PropertiesEditor();

    void update();

signals:
    void dataChanged();

private:
    // Content
    void createContent(PropertyType type);
    CustomDoubleSpinBox* createDoubleSpinBox(QPair<double, double> const& limits, int numDecimals);

    // Signals & slots
    void processDataChange();

    Backend::Properties& mProperties;
};

} // namespace Frontend

#endif // PROPERTIESEDITOR_H

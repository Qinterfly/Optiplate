/*!
 * \file
 * \author Pavel Lakiza
 * \date October 2023
 * \brief Implementation of the CustomPropertyEditor class
 */

#include "custompropertyeditor.h"

using namespace Frontend;

CustomPropertyEditor::CustomPropertyEditor(QWidget* pParent)
    : QtTreePropertyBrowser(pParent)
{

}

CustomPropertyEditor::~CustomPropertyEditor()
{

}

//! Add property to the tree structure and register it
bool CustomPropertyEditor::addProperty(QtProperty* pProperty, int id, QtProperty* pParentProperty, bool isExpanded)
{
    // Try to register the property
    if (!registerProperty(pProperty, id))
        return false;

    // Insert the property and expand it
    QtBrowserItem* pItem;
    if (pParentProperty)
    {
        pParentProperty->addSubProperty(pProperty);
    }
    else
    {
        QtBrowserItem* pItem = QtTreePropertyBrowser::addProperty(pProperty);
        setExpanded(pItem, isExpanded);
    }

    return true;
}

//! Unregister all properties
void CustomPropertyEditor::clear()
{
    mPropertyToID.clear();
    mIDToProperty.clear();
}

bool CustomPropertyEditor::contains(QtProperty* pProperty) const
{
    return mPropertyToID.contains(pProperty);
}

bool CustomPropertyEditor::contains(int id) const
{
    return mIDToProperty.contains(id);
}

int CustomPropertyEditor::id(QtProperty* pProperty) const
{
    return mPropertyToID[pProperty];
}

QtProperty* CustomPropertyEditor::property(int id) const
{
    return mIDToProperty[id];
}

//! Register the property in the tree structure
bool CustomPropertyEditor::registerProperty(QtProperty* pProperty, int id)
{
    if (mIDToProperty.contains(id))
        return false;

    mPropertyToID[pProperty] = id;
    mIDToProperty[id] = pProperty;

    return true;
}

//! Unregister the property from the tree structure
bool CustomPropertyEditor::unregisterProperty(int id)
{
    if (!mIDToProperty.contains(id))
        return false;

    QtProperty* pProperty = mIDToProperty[id];
    mPropertyToID.remove(pProperty);
    mIDToProperty.remove(id);

    return true;
}

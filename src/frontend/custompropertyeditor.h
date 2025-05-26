/*!
 * \file
 * \author Pavel Lakiza
 * \date October 2023
 * \brief Declaration of the CustomPropertyEditor class
 */

#ifndef CUSTOMPROPERTYEDITOR_H
#define CUSTOMPROPERTYEDITOR_H

#include <qtpropertybrowser/qttreepropertybrowser.h>

namespace Frontend
{

class CustomPropertyEditor : public QtTreePropertyBrowser
{
    Q_OBJECT

public:
    CustomPropertyEditor(QWidget* pParent = nullptr);
    virtual ~CustomPropertyEditor();

    bool addProperty(QtProperty* pProperty, int id, QtProperty* pParentProperty = nullptr, bool isExpanded = true);
    void clear();

    bool contains(QtProperty* pProperty) const;
    bool contains(int id) const;
    int id(QtProperty* pProperty) const;
    QtProperty* property(int id) const;

signals:
    void propertyChanged();

protected:
    bool registerProperty(QtProperty* pProperty, int id);
    bool unregisterProperty(int id);

    QMap<QtProperty*, int> mPropertyToID;
    QMap<int, QtProperty*> mIDToProperty;
};

}

#endif // CUSTOMPROPERTYEDITOR_H

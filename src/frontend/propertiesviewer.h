/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Declaration of the PropertiesViewer class
 */

#ifndef PROPERTIESVIEWER_H
#define PROPERTIESVIEWER_H

#include <QWidget>

#include "panel.h"
#include "properties.h"

QT_FORWARD_DECLARE_CLASS(QTableWidget)

namespace Frontend
{

class PropertiesViewer : public QWidget
{
    Q_OBJECT

public:
    PropertiesViewer(QWidget* pParent = nullptr);
    virtual ~PropertiesViewer();

    void clear();
    void update(Backend::Panel const& panel, Backend::Properties const& target);
    void update(Backend::Properties const& current, Backend::Properties const& target);

    void keyPressEvent(QKeyEvent* pEvent) override;

private:
    void createContent();
    void appendRow(QString const& name, double current, double target, double error);

private:
    QTableWidget* mpTable;
};

} // namespace Frontend

#endif // PROPERTIESVIEWER_H

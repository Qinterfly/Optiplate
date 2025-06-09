/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Declaration of the PropertiesViewer class
 */

#ifndef PROPERTIESVIEWER_H
#define PROPERTIESVIEWER_H

#include <QWidget>

#include "optimizer.h"

QT_FORWARD_DECLARE_CLASS(QTableWidget)

namespace Backend
{
class Configuration;
class Panel;
class Properties;
}

namespace Frontend
{

class PropertiesViewer : public QWidget
{
    Q_OBJECT

public:
    PropertiesViewer(QWidget* pParent = nullptr);
    virtual ~PropertiesViewer();

    void clear();
    void update(Backend::Panel const& panel, Backend::Configuration const& configuration);
    void update(Backend::Properties const& current, Backend::Configuration const& configuration);

    void keyPressEvent(QKeyEvent* pEvent) override;

private:
    void createContent();
    void appendRow(QString const& name, double current, double target, double error, Backend::Optimizer::Options const& options);

private:
    QTableWidget* mpTable;
};

} // namespace Frontend

#endif // PROPERTIESVIEWER_H

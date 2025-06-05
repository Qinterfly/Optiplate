/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Declaration of the PanelViewer class
 */

#ifndef GEOMETRYPLOT_H
#define GEOMETRYPLOT_H

#include <vtkPolyDataMapper.h>
#include <QWidget>

#include "panel.h"

class QVTKOpenGLNativeWidget;

namespace Frontend
{

class GeometryPlot : public QWidget
{
public:
    GeometryPlot(QWidget* pParent = nullptr);
    virtual ~GeometryPlot();

    QSize sizeHint() const override;
    void plot(Backend::Panel const& panel);
    void clear();

private:
    void initialize();
    void createContent();
    vtkSmartPointer<vtkPolyDataMapper> createMapper(Backend::Panel const& panel);

private:
    QVTKOpenGLNativeWidget* mpRenderWidget;
};
}

#endif // GEOMETRYPLOT_H

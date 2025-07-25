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

#include "optimizer.h"
#include "panel.h"

class QVTKOpenGLNativeWidget;
class vtkColor3d;
class vtkOrientationMarkerWidget;
class vtkNamedColors;
class vtkPropAssembly;

class ClickInteractionStyle;

namespace Frontend
{

class GeometryPlot : public QWidget
{
    Q_OBJECT

public:
    GeometryPlot(QWidget* pParent = nullptr);
    virtual ~GeometryPlot();

    QSize sizeHint() const override;
    void clear();
    void plot(Backend::Panel const& panel, Backend::Optimizer::Solution const& solution = Backend::Optimizer::Solution());

private:
    void initialize();
    void createContent();

    // Drawing
    void drawAxes();
    void drawPanel(Backend::Panel const& panel, vtkColor3d color);
    vtkSmartPointer<vtkPolyDataMapper> createPanelMapper(Backend::Panel const& panel);
    vtkSmartPointer<vtkPropAssembly> createOrientationActor();

    // Slots
    void setIsometricView();
    void setPlaneView(int dir, int sign);

private:
    QVTKOpenGLNativeWidget* mRenderWidget;
    vtkSmartPointer<vtkRenderWindow> mRenderWindow;
    vtkSmartPointer<vtkRenderer> mRenderer;
    vtkSmartPointer<vtkOrientationMarkerWidget> mOrientation;
    vtkSmartPointer<vtkNamedColors> mColors;
    vtkSmartPointer<ClickInteractionStyle> mStyle;
};
}

#endif // GEOMETRYPLOT_H

/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the GeometryPlot class
 */

#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCameraOrientationWidget.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPointData.h>
#include <vtkPolygon.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSphereSource.h>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

#include "geometryplot.h"

using namespace Frontend;

static vtkNew<vtkNamedColors> const skColors;

GeometryPlot::GeometryPlot(QWidget* pParent)
    : QWidget(pParent)
{
    createContent();
    initialize();
}

GeometryPlot::~GeometryPlot()
{
}

QSize GeometryPlot::sizeHint() const
{
    return QSize(800, 600);
}

void GeometryPlot::initialize()
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    // Set up the scence
    vtkNew<vtkRenderer> renderer;
    renderer->SetBackground(skColors->GetColor3d("White").GetData());

    // Create the window
    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    renderWindow->AddRenderer(renderer);
    mpRenderWidget->setRenderWindow(renderWindow);
}

//! Create all the widgets and corresponding actions
void GeometryPlot::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;
    mpRenderWidget = new QVTKOpenGLNativeWidget;
    pLayout->addWidget(mpRenderWidget);
    setLayout(pLayout);
}

//! Represent geometry of a panel
void GeometryPlot::plot(Backend::Panel const& panel)
{
    clear();

    // Get the window and renderer
    auto renderWindow = mpRenderWidget->renderWindow();
    auto renderer = renderWindow->GetRenderers()->GetFirstRenderer();

    // Create an actor
    vtkNew<vtkActor> actor;
    actor->SetMapper(createMapper(panel));
    actor->GetProperty()->SetColor(skColors->GetColor3d("Red").GetData());

    // Add the actors to the scene.
    renderer->AddActor(actor);

    // Draw
    renderWindow->Render();
}

//! Remove all renderers
void GeometryPlot::clear()
{
    auto renderWindow = mpRenderWidget->renderWindow();
    auto renderer = renderWindow->GetRenderers()->GetFirstRenderer();
    auto actors = renderer->GetActors();
    while (actors->GetLastActor())
        renderer->RemoveActor(actors->GetLastActor());
}

//! Generate geometrical data for graphic library
vtkSmartPointer<vtkPolyDataMapper> GeometryPlot::createMapper(Backend::Panel const& panel)
{
    // Slice the geometrical data
    auto xCoords = panel.xCoords();
    auto yCoords = panel.allDepths();
    auto zCoords = panel.zCoords();

    // Setup the points
    vtkNew<vtkPoints> points;
    int numCoords = xCoords.size();
    for (int i = 0; i != numCoords; ++i)
        points->InsertNextPoint(xCoords[i], yCoords[i] / 2.0, zCoords[i]);
    for (int i = 0; i != numCoords; ++i)
        points->InsertNextPoint(xCoords[i], -yCoords[i] / 2.0, zCoords[i]);

    // Setup the indices
    QList<QList<int>> sets;
    sets.push_back({0, 1, 2, 3}); // Top
    sets.push_back({4, 5, 6, 7}); // Botoom
    sets.push_back({4, 0, 3, 7}); // Front
    sets.push_back({5, 1, 2, 6}); // Back
    sets.push_back({4, 5, 1, 0}); // Left
    sets.push_back({7, 6, 2, 3}); // Right

    // Create the polygons
    vtkNew<vtkCellArray> polygons;
    int numSets = sets.size();
    for (int i = 0; i != numSets; ++i)
    {
        auto indices = sets[i];
        int numIndices = indices.size();
        vtkNew<vtkPolygon> polygon;
        polygon->GetPointIds()->SetNumberOfIds(numIndices);
        for (int j = 0; j != numIndices; ++j)
        {
            int index = indices[j];
            polygon->GetPointIds()->SetId(j, index);
        }
        polygons->InsertNextCell(polygon);
    }

    // Create a PolyData
    vtkNew<vtkPolyData> polygonPolyData;
    polygonPolyData->SetPoints(points);
    polygonPolyData->SetPolys(polygons);

    // Build the mapper
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(polygonPolyData);

    return mapper;
}

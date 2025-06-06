/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the GeometryPlot class
 */

#include <vtkActor.h>
#include <vtkAnnotatedCubeActor.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCameraOrientationWidget.h>
#include <vtkCaptionActor2D.h>
#include <vtkCellData.h>
#include <vtkCubeSource.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPointData.h>
#include <vtkPointPicker.h>
#include <vtkPolygon.h>
#include <vtkPropAssembly.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSphereSource.h>
#include <vtkTextProperty.h>
#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>

#include "geometryplot.h"

using namespace Frontend;

vtkSmartPointer<vtkPropAssembly> MakeAnnotatedCubeActor(vtkNamedColors* colors);

// Define interaction style.
class MouseInteractionStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static MouseInteractionStyle* New();
    vtkTypeMacro(MouseInteractionStyle, vtkInteractorStyleTrackballCamera);

    virtual void OnLeftButtonDown() override
    {
        int* pPosition = Interactor->GetEventPosition();
        vtkSmartPointer<vtkRenderer> renderer = Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
        Interactor->GetPicker()->Pick(pPosition[0], pPosition[1], 0, renderer);
        double picked[3];
        Interactor->GetPicker()->GetPickPosition(picked);
        std::cout << "Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << std::endl;
        vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }
};

vtkStandardNewMacro(MouseInteractionStyle);

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

    mColors = vtkNamedColors::New();
    mOrientation = vtkOrientationMarkerWidget::New();

    // Set up the scence
    mRenderer = vtkRenderer::New();
    mRenderer->SetBackground(mColors->GetColor3d("White").GetData());
    mRenderer->GetActiveCamera()->Elevation(30);
    mRenderer->GetActiveCamera()->Azimuth(30);

    // Create the window
    mRenderWindow = vtkGenericOpenGLRenderWindow::New();
    mRenderWindow->AddRenderer(mRenderer);
    mVTKWidget->setRenderWindow(mRenderWindow);

    // Set up the picker
    vtkNew<vtkPointPicker> pointPicker;
    vtkNew<MouseInteractionStyle> style;
    auto renderWindowInteractor = mRenderWindow->GetInteractor();
    renderWindowInteractor->SetPicker(pointPicker);
    renderWindowInteractor->SetInteractorStyle(style);
}

//! Create all the widgets and corresponding actions
void GeometryPlot::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;
    mVTKWidget = new QVTKOpenGLNativeWidget;
    pLayout->addWidget(mVTKWidget);
    setLayout(pLayout);
}

//! Remove all actors
void GeometryPlot::clear()
{
    auto actors = mRenderer->GetActors();
    while (actors->GetLastActor())
        mRenderer->RemoveActor(actors->GetLastActor());
}

//! Plot initial and optimized geometries
void GeometryPlot::plot(Backend::Panel const& panel, Backend::Optimizer::Solution const& solution)
{
    clear();
    drawAxes();
    drawPanel(panel, mColors->GetColor3d("Black"));
    if (solution.isValid())
        drawPanel(solution.panel, mColors->GetColor3d("Red"));
    mRenderer->ResetCamera();
    mRenderWindow->Render();
}

//! Represent the coordinate system
void GeometryPlot::drawAxes()
{
    auto actor = createOrientationActor();

    double rgba[4]{0.0, 0.0, 0.0, 0.0};
    mColors->GetColor("Carrot", rgba);
    mOrientation->SetOutlineColor(rgba[0], rgba[1], rgba[2]);
    mOrientation->SetOrientationMarker(actor);
    mOrientation->SetInteractor(mRenderWindow->GetInteractor());
    mOrientation->SetViewport(0.0, 0.0, 0.2, 0.2);
    mOrientation->SetEnabled(1);
    mOrientation->InteractiveOn();
}

//! Represent geometry of a panel
void GeometryPlot::drawPanel(Backend::Panel const& panel, vtkColor3d color)
{
    // Create an actor
    vtkNew<vtkActor> frameActor;
    frameActor->SetMapper(createPanelMapper(panel));
    frameActor->GetProperty()->SetColor(color.GetData());
    frameActor->GetProperty()->SetRepresentationToWireframe();

    vtkNew<vtkActor> volumeActor;
    volumeActor->SetMapper(createPanelMapper(panel));
    volumeActor->GetProperty()->SetColor(color.GetData());
    volumeActor->GetProperty()->SetOpacity(0.1);

    // Add the actors to the scene.
    mRenderer->AddActor(volumeActor);
    mRenderer->AddActor(frameActor);
}

//! Slice panel geometrical data and create a mapper out of it
vtkSmartPointer<vtkPolyDataMapper> GeometryPlot::createPanelMapper(Backend::Panel const& panel)
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

//! Create the annotated cube actor with different colored faces
vtkSmartPointer<vtkPropAssembly> GeometryPlot::createOrientationActor()
{
    // Annotated Cube setup.
    vtkNew<vtkAnnotatedCubeActor> cube;
    cube->SetFaceTextScale(0.366667);

    // Anatomic labeling
    cube->SetXPlusFaceText("X+");
    cube->SetXMinusFaceText("X-");
    cube->SetYPlusFaceText("Y+");
    cube->SetYMinusFaceText("Y-");
    cube->SetZPlusFaceText("Z+");
    cube->SetZMinusFaceText("Z-");

    // Change the vector text colors.
    cube->GetTextEdgesProperty()->SetColor(mColors->GetColor3d("Black").GetData());
    cube->GetTextEdgesProperty()->SetLineWidth(1);

    cube->GetXPlusFaceProperty()->SetColor(mColors->GetColor3d("Turquoise").GetData());
    cube->GetXMinusFaceProperty()->SetColor(mColors->GetColor3d("Turquoise").GetData());
    cube->GetYPlusFaceProperty()->SetColor(mColors->GetColor3d("Mint").GetData());
    cube->GetYMinusFaceProperty()->SetColor(mColors->GetColor3d("Mint").GetData());
    cube->GetZPlusFaceProperty()->SetColor(mColors->GetColor3d("Tomato").GetData());
    cube->GetZMinusFaceProperty()->SetColor(mColors->GetColor3d("Tomato").GetData());
    cube->SetXFaceTextRotation(90);
    cube->SetYFaceTextRotation(180);
    cube->SetZFaceTextRotation(-90);

    // Make the annotated cube transparent.
    cube->GetCubeProperty()->SetOpacity(0);

    // Set colors of cube faces
    vtkNew<vtkCubeSource> source;
    source->Update();

    vtkNew<vtkUnsignedCharArray> faceColors;
    faceColors->SetNumberOfComponents(3);
    faceColors->InsertNextTypedTuple(mColors->GetColor3ub("Magenta").GetData());
    faceColors->InsertNextTypedTuple(mColors->GetColor3ub("Red").GetData());
    faceColors->InsertNextTypedTuple(mColors->GetColor3ub("Yellow").GetData());
    faceColors->InsertNextTypedTuple(mColors->GetColor3ub("Green").GetData());
    faceColors->InsertNextTypedTuple(mColors->GetColor3ub("Cyan").GetData());
    faceColors->InsertNextTypedTuple(mColors->GetColor3ub("Blue").GetData());

    source->GetOutput()->GetCellData()->SetScalars(faceColors);
    source->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(source->GetOutput());
    mapper->Update();

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);

    // Assemble the colored cube and annotated cube texts into a composite prop
    vtkNew<vtkPropAssembly> assembly;
    assembly->AddPart(cube);
    assembly->AddPart(actor);

    return assembly;
}

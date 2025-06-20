/*!
 * \file
 * \author Pavel Lakiza
 * \date June 2025
 * \brief Implementation of the GeometryPlot class
 */

#include <vtkActor.h>
#include <vtkAnnotatedCubeActor.h>
#include <vtkAxesActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCameraOrientationWidget.h>
#include <vtkCaptionActor2D.h>
#include <vtkCaptionRepresentation.h>
#include <vtkCaptionWidget.h>
#include <vtkCellData.h>
#include <vtkCellPicker.h>
#include <vtkCubeSource.h>
#include <vtkDataSetMapper.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPointData.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkPointPicker.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkPropAssembly.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <QHBoxLayout>
#include <QToolBar>
#include <QVTKOpenGLNativeWidget.h>

#include "geometryplot.h"
#include "uiconstants.h"
#include "uiutility.h"

using namespace Frontend;

void processKeyPress(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);

//! Define interaction style
class ClickInteractionStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static ClickInteractionStyle* New();

    ClickInteractionStyle()
        : tolerancePick(1e-2)
        , shiftLabel(0.025)
    {
        captionWidget = vtkCaptionWidget::New();
    }

    //! Add captions when points picked
    virtual void OnLeftButtonDown() override
    {
        // Get the click position
        int* screenPosition = Interactor->GetEventPosition();

        // Create the picker
        vtkNew<vtkPointPicker> picker;
        picker->SetTolerance(tolerancePick);

        // Pick at the click position
        vtkSmartPointer<vtkRenderer> renderer = GetDefaultRenderer();
        picker->Pick(screenPosition[0], screenPosition[1], 0, renderer);

        // Process the pick
        if (picker->GetPointId() != -1)
            add(picker);

        // Forward events
        GetInteractor()->GetRenderWindow()->Render();
        vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }

    //! Clear the caption
    void clear()
    {
        captionWidget->SetRepresentation(nullptr);
        captionWidget->SetEnabled(0);
        GetInteractor()->GetRenderWindow()->Render();
    }

    //! Add the text caption associated with the selected point
    void add(vtkSmartPointer<vtkPointPicker> picker)
    {
        auto constexpr kFormat = "X: {:.3g}\nY: {:.3g}\nZ: {:.3g}";

        vtkNew<vtkNamedColors> colors;

        vtkSmartPointer<vtkActor> actor = picker->GetActor();
        vtkSmartPointer<vtkDataSet> actorData = actor->GetMapper()->GetInput();

        // Retrieve the selected data to render
        double worldPosition[3];
        actorData->GetPoint(picker->GetPointId(), worldPosition);

        vtkNew<vtkCaptionRepresentation> captionRep;

        // Set the caption text
        vtkSmartPointer<vtkCaptionActor2D> captionActor = captionRep->GetCaptionActor2D();
        vtkSmartPointer<vtkTextProperty> captionTextProperty = captionActor->GetCaptionTextProperty();
        std::string const text = std::format(kFormat, worldPosition[0], worldPosition[1], worldPosition[2]);
        captionActor->SetCaption(text.data());
        captionActor->GetProperty()->SetColor(colors->GetColor3d("Black").GetData());
        captionTextProperty->SetColor(colors->GetColor3d("Black").GetData());
        captionTextProperty->BoldOff();
        captionTextProperty->ItalicOff();
        captionTextProperty->ShadowOff();

        // Set the anchor properties
        captionRep->GetAnchorRepresentation()->GetProperty()->SetOpacity(0.0);
        captionRep->GetAnchorRepresentation()->DragableOff();

        // Get the selected position in normalized display coordinates
        double x = worldPosition[0];
        double y = worldPosition[1];
        double z = worldPosition[2];
        GetDefaultRenderer()->WorldToDisplay(x, y, z);
        GetDefaultRenderer()->DisplayToNormalizedDisplay(x, y);

        // Set the caption position
        captionRep->SetAnchorPosition(worldPosition);
        captionRep->SetPosition(x + shiftLabel, y + shiftLabel);

        // Enable the caption
        captionWidget->SetInteractor(GetInteractor());
        captionWidget->SetEnabled(1);
        captionWidget->SetRepresentation(captionRep);
    }

    double tolerancePick;
    double shiftLabel;
    vtkSmartPointer<vtkCaptionWidget> captionWidget;
};

vtkStandardNewMacro(ClickInteractionStyle);

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
    vtkObject::GlobalWarningDisplayOff();

    mColors = vtkNamedColors::New();
    mOrientation = vtkOrientationMarkerWidget::New();

    // Set up the scence
    mRenderer = vtkRenderer::New();
    mRenderer->SetBackground(mColors->GetColor3d("WhiteSmoke").GetData());

    // Create the window
    mRenderWindow = vtkGenericOpenGLRenderWindow::New();
    mRenderWindow->AddRenderer(mRenderer);
    mRenderWidget->setRenderWindow(mRenderWindow);

    // Set up the picker
    mStyle = ClickInteractionStyle::New();
    mStyle->SetDefaultRenderer(mRenderer);
    auto renderWindowInteractor = mRenderWindow->GetInteractor();
    renderWindowInteractor->SetInteractorStyle(mStyle);

    // Enable keyboard user interactions
    vtkNew<vtkCallbackCommand> keypressCallback;
    keypressCallback->SetCallback(processKeyPress);
    renderWindowInteractor->AddObserver(vtkCommand::KeyPressEvent, keypressCallback);

    // Set the view
    setIsometricView();
}

//! Create all the widgets and corresponding actions
void GeometryPlot::createContent()
{
    QHBoxLayout* pLayout = new QHBoxLayout;

    // Create the VTK widget
    mRenderWidget = new QVTKOpenGLNativeWidget;

    // Create the toolbar
    QToolBar* pToolBar = new QToolBar;
    pToolBar->setOrientation(Qt::Vertical);
    pToolBar->setIconSize(Constants::Size::skToolBarIcon);

    // Create the actions
    QAction* pIsometricAction = new QAction(tr("Isometric view"), this);
    QAction* pLeftAction = new QAction(tr("Left view"), this);
    QAction* pRightAction = new QAction(tr("Right view"), this);
    QAction* pTopAction = new QAction(tr("Top view"), this);
    QAction* pBottomAction = new QAction(tr("Bottom view"), this);
    QAction* pFrontAction = new QAction(tr("Front view"), this);
    QAction* pRearAction = new QAction(tr("Rear view"), this);

    // Set the icons
    pIsometricAction->setIcon(QIcon(":/icons/view-isometric.svg"));
    pTopAction->setIcon(QIcon(":/icons/view-top.svg"));
    pBottomAction->setIcon(QIcon(":/icons/view-bottom.svg"));
    pLeftAction->setIcon(QIcon(":/icons/view-left.svg"));
    pRightAction->setIcon(QIcon(":/icons/view-right.svg"));
    pFrontAction->setIcon(QIcon(":/icons/view-front.svg"));
    pRearAction->setIcon(QIcon(":/icons/view-rear.svg"));

    // Set the shortcuts
    pIsometricAction->setShortcut(Qt::CTRL | Qt::Key_1);
    pTopAction->setShortcut(Qt::CTRL | Qt::Key_2);
    pBottomAction->setShortcut(Qt::CTRL | Qt::Key_3);
    pLeftAction->setShortcut(Qt::CTRL | Qt::Key_4);
    pRightAction->setShortcut(Qt::CTRL | Qt::Key_5);
    pFrontAction->setShortcut(Qt::CTRL | Qt::Key_6);
    pRearAction->setShortcut(Qt::CTRL | Qt::Key_7);
    Utility::setShortcutHints(pToolBar);

    // Connect the actions
    connect(pIsometricAction, &QAction::triggered, this, &GeometryPlot::setIsometricView);
    connect(pLeftAction, &QAction::triggered, this, [this]() { setPlaneView(0, 1); });
    connect(pRightAction, &QAction::triggered, this, [this]() { setPlaneView(0, -1); });
    connect(pTopAction, &QAction::triggered, this, [this]() { setPlaneView(1, 1); });
    connect(pBottomAction, &QAction::triggered, this, [this]() { setPlaneView(1, -1); });
    connect(pFrontAction, &QAction::triggered, this, [this]() { setPlaneView(2, 1); });
    connect(pRearAction, &QAction::triggered, this, [this]() { setPlaneView(2, -1); });

    // Add the actions
    pToolBar->addAction(pIsometricAction);
    pToolBar->addAction(pLeftAction);
    pToolBar->addAction(pRightAction);
    pToolBar->addAction(pTopAction);
    pToolBar->addAction(pBottomAction);
    pToolBar->addAction(pFrontAction);
    pToolBar->addAction(pRearAction);

    // Combine the widgets
    pLayout->addWidget(mRenderWidget);
    pLayout->addWidget(pToolBar);
    setLayout(pLayout);
}

//! Remove all actors
void GeometryPlot::clear()
{
    mStyle->clear();
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
    cube->SetXPlusFaceText("+X");
    cube->SetXMinusFaceText("-X");
    cube->SetYPlusFaceText("+Y");
    cube->SetYMinusFaceText("-Y");
    cube->SetZPlusFaceText("+Z");
    cube->SetZMinusFaceText("-Z");

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

//! Set the isometric view
void GeometryPlot::setIsometricView()
{
    vtkSmartPointer<vtkCamera> camera = mRenderer->GetActiveCamera();
    camera->SetPosition(1, 1, 1);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(0, 1, 0);
    mRenderer->ResetCamera();
    mRenderWindow->Render();
}

//! Set view perpendicular to one of the planes
void GeometryPlot::setPlaneView(int dir, int sign)
{
    int const kNumDirections = 3;
    vtkSmartPointer<vtkCamera> camera = mRenderer->GetActiveCamera();
    double position[kNumDirections];
    for (int i = 0; i != kNumDirections; ++i)
        position[i] = 0.0;
    position[dir] = 1.0 * sign;
    camera->SetPosition(position);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(0, -1, 0);
    mRenderer->ResetCamera();
    mRenderWindow->Render();
}

//! Helper callback function to process keypress events
void processKeyPress(vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData))
{
    vtkRenderWindowInteractor* renderWindowInteractor = static_cast<vtkRenderWindowInteractor*>(caller);
    QString keyName = renderWindowInteractor->GetKeySym();
    if (keyName == "Escape" || keyName == "Delete" || keyName == "BackSpace")
    {
        ClickInteractionStyle* style = (ClickInteractionStyle*) renderWindowInteractor->GetInteractorStyle();
        style->clear();
    }
}

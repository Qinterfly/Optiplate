
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGLWidgets)
find_package(qtadvanceddocking-qt6 CONFIG REQUIRED)
find_package(VTK REQUIRED COMPONENTS
  CommonColor
  CommonCore
  CommonDataModel
  FiltersSources
  InteractionStyle
  InteractionWidgets
  RenderingAnnotation
  RenderingContextOpenGL2
  RenderingCore
  RenderingFreeType
  RenderingGL2PSOpenGL2
  RenderingOpenGL2
  GUISupportQt
  RenderingQt
)

set(FRONTEND_HEADERS
    mainwindow.h
    paneleditor.h
    uiutility.h
    uiconstants.h
    propertieseditor.h
    optionseditor.h
    custompropertyeditor.h
    propertiesviewer.h
    customplot.h
    datatip.h
    plottablepropertyeditor.h
    customtable.h
    convergenceplot.h
    logger.h
    solutionbrowser.h
    solutionmodel.h
    solutionitem.h
    geometryplot.h
)

set(FRONTEND_SOURCES
    mainwindow.cpp
    paneleditor.cpp
    uiutility.cpp
    propertieseditor.cpp
    optionseditor.cpp
    custompropertyeditor.cpp
    propertiesviewer.cpp
    customplot.cpp
    datatip.cpp
    plottablepropertyeditor.cpp
    customtable.cpp
    convergenceplot.cpp
    logger.cpp
    solutionbrowser.cpp
    solutionmodel.cpp
    solutionitem.cpp
    geometryplot.cpp
)

qt_add_library(frontend STATIC
    ${FRONTEND_HEADERS}
    ${FRONTEND_SOURCES}
    ${RESOURCES_FILE}
)

target_link_libraries(frontend PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGLWidgets
    ads::qtadvanceddocking-qt6
    ${VTK_LIBRARIES}
    qcustomplot
    qtpropertybrowser
    backend
)

target_include_directories(frontend PUBLIC ${CMAKE_BINARY_DIR})
target_include_directories(frontend INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

vtk_module_autoinit(
  TARGETS frontend
  MODULES ${VTK_LIBRARIES}
)

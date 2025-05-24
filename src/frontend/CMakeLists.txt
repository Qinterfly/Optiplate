
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(qtadvanceddocking-qt6 CONFIG REQUIRED)

set(FRONTEND_HEADERS
    mainwindow.h
    paneleditor.h
    uiutility.h
    uiconstants.h
    propertieseditor.h
    optionseditor.h
    custompropertyeditor.h
)

set(FRONTEND_SOURCES
    mainwindow.cpp
    paneleditor.cpp
    uiutility.cpp
    propertieseditor.cpp
    optionseditor.cpp
    custompropertyeditor.cpp
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
    ads::qtadvanceddocking-qt6
    qcustomplot
    qtpropertybrowser
    backend
)

target_include_directories(frontend PUBLIC ${CMAKE_BINARY_DIR})
target_include_directories(frontend INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

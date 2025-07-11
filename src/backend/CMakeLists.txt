
find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)
find_package(Eigen3 CONFIG REQUIRED)
find_package(Ceres CONFIG REQUIRED)

set(BACKEND_HEADERS
    panel.h
    optimizer.h
    fileutility.h
    mathutility.h
    project.h
    properties.h
)

set(BACKEND_SOURCES
    panel.cpp
    optimizer.cpp
    fileutility.cpp
    mathutility.cpp
    project.cpp
    properties.cpp
)

qt_add_library(backend STATIC
    ${BACKEND_HEADERS}
    ${BACKEND_SOURCES}
)

target_include_directories(backend PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(backend PUBLIC
    Qt::Core
    Eigen3::Eigen
    Ceres::ceres
    ${KCL_LIB}
)

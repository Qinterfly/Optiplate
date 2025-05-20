
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)

qt_add_executable(testfrontend
    testfrontend.h
    testfrontend.cpp
)

set_target_properties(testfrontend PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(testfrontend PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
    backend
    frontend
)

target_include_directories(testfrontend PUBLIC ${CMAKE_BINARY_DIR})

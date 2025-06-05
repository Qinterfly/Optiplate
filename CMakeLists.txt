
cmake_minimum_required(VERSION 3.22)

# Set parameters

## Data
set(APP_NAME Optiplate)
set(APP_CREATOR "Pavel Lakiza")
## Version
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 5)
set(VERSION_FULL ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
## Directories
set(EXAMPLES_DIR ${CMAKE_SOURCE_DIR}/examples)
set(RESOURCES_DIR ${CMAKE_SOURCE_DIR}/resources)
set(TEMP_DIR ${CMAKE_BINARY_DIR})
# Files
set(RESOURCES_FILE ${RESOURCES_DIR}/resources.qrc)
set(INFO_FILE ${RESOURCES_DIR}/info.rc)

# Set language standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files.")
configure_file(cmake/config.h.in config.h @ONLY)

# Specify project options
project(optiplate LANGUAGES CXX)

# Create working functions
function(deployDependencies target)
    if (WIN32)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            find_program(TOOL_WINDEPLOYQT NAMES windeployqt.debug.bat)
        else()
            find_program(TOOL_WINDEPLOYQT NAMES windeployqt)
        endif()
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${TOOL_WINDEPLOYQT}
            $<TARGET_FILE:${target}>
        )
    endif()
endfunction()


# Add dependencies
add_subdirectory(lib)

# Add project elements
add_subdirectory(src)
add_subdirectory(tests)

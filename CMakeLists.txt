
cmake_minimum_required(VERSION 3.22)

# Set parameters
## Data
set(APP_NAME Optiplate)
set(APP_CREATOR "Pavel Lakiza")
## Version
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 1)
set(VERSION_FULL ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
## Directories
set(TEMP_DIR ${CMAKE_BINARY_DIR})

# Set language standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
configure_file(cmake/config.h.in config.h @ONLY)

# Specify project options
project(optiplate LANGUAGES CXX)

# Add dependencies
add_subdirectory(lib)

# Add project elements
add_subdirectory(src)
add_subdirectory(tests)

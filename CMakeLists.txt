cmake_minimum_required(VERSION 3.24)

project(LRenderDemo VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(cmake/Dependencies.cmake)
include(cmake/CompilerWarnings.cmake)

add_subdirectory(src)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT LRenderDemo)

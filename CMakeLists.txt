cmake_minimum_required(VERSION 3.8)
project(simpleocv)

set(CMAKE_CXX_STANDARD 11)

option(BUILD_STATIC "enable to build static simpleocv lib" ON)
option(DEBUG "Enable debug mode, for develop" ON)

add_compile_definitions(NCNN_PIXEL)
add_compile_definitions(NCNN_THREADS)

include(GNUInstallDirs)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB_RECURSE SRCS "src/*.cpp" "src/*.cc" "src/*.hpp" "src/*.h")
file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")

if (BUILD_STATIC)
    add_library(simpleocv STATIC ${SRCS} ${HEADERS})
else()
    add_library(simpleocv SHARED ${SRCS} ${HEADERS})
endif()

set_target_properties(
    simpleocv
    PROPERTIES
    PUBLIC_HEADER "${HEADERS}")
target_include_directories(simpleocv PRIVATE .)

add_subdirectory(examples/)

install(TARGETS simpleocv
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

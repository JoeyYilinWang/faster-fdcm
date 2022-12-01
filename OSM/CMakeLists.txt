cmake_minimum_required(VERSION 2.8.3)
project(render_unit)

set(CMAKE_BUILD_TYPE "release")
set(CMAKE_CXX_FLAGS "-std=c++11")

set(GeographicLib_INCLUDE_DIRS "/usr/local/include/GeographicLib")

find_package(OpenCV REQUIRED)

list(APPEND LINK_LIBS
${OpenCV_LIBS}
/usr/local/lib/libGeographicLib.so
/usr/local/lib/libGeographicLib.so.23
/usr/local/lib/libGeographicLib.so.23.0.1)

message("OpenCV is included in " ${OpenCV_INCLUDE_DIRS})
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_library(Element SHARED Element.cpp)

add_executable(render_unit main.cpp)
target_link_libraries(render_unit Element ${LINK_LIBS})
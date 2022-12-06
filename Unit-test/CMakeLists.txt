cmake_minimum_required(VERSION 2.8.3)
project(Unit-test)

set(CMAKE_BUILD_TYPE "Debug")
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

add_library(Element SHARED ../OSM/Element.cpp)
add_library(LineFitter SHARED ../LineFit/LineFitter.cpp)
add_library(LineSegment SHARED ../LineFit/LineSegment.cpp)
add_library(DistanceTransform SHARED ../Image/DistanceTransform.cpp)
add_library(LMDirectionalIntegralDistanceImage SHARED ../Fdcm/LMDirectionalIntegralDistanceImage.cpp)

add_library(LMDistanceImage SHARED ../Fdcm/LMDistanceImage.cpp)
target_link_libraries(LMDistanceImage LineSegment  LMDirectionalIntegralDistanceImage)

add_library(EIEdgeImage SHARED ../Fdcm/EIEdgeImage.cpp)


# test OSMRender-UNIT 
add_executable(OSMRender-UNIT OSMRender-UNIT.cpp)
target_link_libraries(OSMRender-UNIT Element ${LINK_LIBS})

# test LineFit-UNIT
add_executable(LineFit-UNIT LineFit-UNIT.cpp)
target_link_libraries(LineFit-UNIT LineFitter LineSegment ${LINK_LIBS})

# test dtImage3dGen-UNIT
add_executable(dtImage3dGen-UNIT dtImage3dGen-UNIT.cpp)
target_link_libraries(dtImage3dGen-UNIT DistanceTransform LineFitter LineSegment LMDistanceImage EIEdgeImage)

# test renderandlinefit 
add_executable(RenderAndLineFit RenderAndLineFit.cpp)
target_link_libraries(RenderAndLineFit Element LineFitter LineSegment DistanceTransform ${LINK_LIBS})



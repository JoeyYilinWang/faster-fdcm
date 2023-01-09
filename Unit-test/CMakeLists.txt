cmake_minimum_required(VERSION 2.8.3)
project(Unit-test)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "-std=c++17")

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
add_library(LMNonMaximumSuppression SHARED ../Fdcm/LMNonMaximumSuppression.cpp)

add_library(LMLineMatcher SHARED ../Fdcm/LMLineMatcher.cpp)
target_link_libraries(LMLineMatcher LMDistanceImage EIEdgeImage LMNonMaximumSuppression LineSegment MatchingCostMap)

add_library(MatchingCostMap SHARED ../Fdcm/MatchingCostMap.cpp)
add_library(LMDisplay SHARED ../Fdcm/LMDisplay.cpp)

add_library(LMDistanceImage SHARED ../Fdcm/LMDistanceImage.cpp)
target_link_libraries(LMDistanceImage LineSegment  LMDirectionalIntegralDistanceImage DistanceTransform)

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

# test window_matching-UNIT
add_executable(Matcher-UNIT Matcher-UNIT.cpp)
target_link_libraries(Matcher-UNIT LineFitter LMLineMatcher LMDisplay ${LINK_LIBS})

# test videoCapture
add_executable(VideoCap VideoCap.cpp)
target_link_libraries(VideoCap ${OpenCV_LIBS})

add_executable(templateListsConstruct OutputImages/tempImages/template_lists/template_list_construct.cpp)




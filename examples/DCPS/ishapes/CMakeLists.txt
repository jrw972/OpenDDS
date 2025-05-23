cmake_minimum_required(VERSION 3.3...4.0)
include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/opendds_version.cmake")
project(ishapes VERSION ${OpenDDS_VERSION} LANGUAGES CXX)
find_package(OpenDDS REQUIRED)

# Make sure the MPC-generated headers are gone so the CMake build will use the
# right ones. This is not needed in a real project.
file(GLOB headers "*.h")
list(LENGTH headers header_count)
if(header_count GREATER 0)
  file(REMOVE ${headers})
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
endif()
find_package(Qt5 REQUIRED COMPONENTS Widgets)

add_executable(ishapes
  # Normal C++ Files
  BouncingShapeDynamics.cpp
  Circle.cpp
  DDSShapeDynamics.cpp
  FilterDialog.cpp
  main.cpp
  ReaderQosDialog.cpp
  Shape.cpp
  ShapeDynamics.cpp
  ShapesDialog.cpp
  ShapesWidget.cpp
  Square.cpp
  Triangle.cpp
  WriterQosDialog.cpp

  # qrc files
  ishape.qrc

  # MOC Files
  FilterDialog.hpp
  ReaderQosDialog.hpp
  ShapesDialog.hpp
  ShapesWidget.hpp
  WriterQosDialog.hpp
)
target_include_directories(ishapes PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
opendds_target_sources(ishapes ShapeType.idl OPENDDS_IDL_OPTIONS -Gxtypes-complete)
target_link_libraries(ishapes
  Qt5::Widgets
  OpenDDS::Dcps
  OpenDDS::Tcp
  OpenDDS::Udp
  OpenDDS::Multicast
  OpenDDS::Rtps_Udp
  OpenDDS::InfoRepoDiscovery
)

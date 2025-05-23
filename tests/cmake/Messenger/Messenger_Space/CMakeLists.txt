# Distributed under the OpenDDS License. See accompanying LICENSE
# file or http://www.opendds.org/license.html for details.
#
# This file is setup the way it is for testing purposes. For a cleaner example
# see DevGuideExamples/DCPS/Messenger/CMakeLists.txt

# This test checks to make sure idl files can be processed when there is a space in the file path

cmake_minimum_required(VERSION 3.8...4.0)
project(opendds_messenger_space CXX)

find_package(OpenDDS REQUIRED ownership_profile)

if(CMAKE_GENERATOR MATCHES "Visual Studio 10.*")
  message(FATAL_ERROR "This test does not support Visual Studio 2010")
endif()

set(target_prefix "${PROJECT_NAME}_")
set(publisher "${target_prefix}publisher")

set(src "${CMAKE_CURRENT_SOURCE_DIR}/../../../DCPS/Messenger")
set(dst "${CMAKE_CURRENT_BINARY_DIR}/ Space")
set(all_targets ${publisher})

foreach(file
  Messenger.idl publisher.cpp Args.h
  pub.ini)
  configure_file(${src}/${file} ${dst}/${file} COPYONLY)
endforeach()

# Publisher
add_executable(${publisher}
  "${dst}/publisher.cpp"
)
set_target_properties(${publisher}
  PROPERTIES OUTPUT_NAME publisher
)
opendds_target_sources(${publisher} "${dst}/Messenger.idl")

foreach(t ${all_targets})
  # Using explicit scope here is a regression test for
  # https://github.com/OpenDDS/OpenDDS/issues/1336
  # See https://cmake.org/cmake/help/latest/policy/CMP0023.html
  target_link_libraries(${t} PUBLIC OpenDDS::OpenDDS OpenDDS_TestUtils)

  # Assert the mapping used was C++03
  get_property(mappings TARGET ${t} PROPERTY OPENDDS_LANGUAGE_MAPPINGS)
  if(NOT "C++03" IN_LIST mappings)
    message(FATAL_ERROR "${t}: C++03 NOT in mapping list: ${mappings}")
  endif()
endforeach()

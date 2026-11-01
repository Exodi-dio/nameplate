cmake_minimum_required(VERSION 3.21.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
  set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

project(nameplate VERSION 0.1.0)

file(GLOB SOURCES src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC src)

if(NOT DEFINED ENV{GEODE_SDK})
  message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
  message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

set(GEODE_SET_TARGET_AS_SYSTEM ON)

add_subdirectory($ENV{GEODE_SDK} $ENV{GEODE_SDK}/build)

target_link_libraries(${PROJECT_NAME} geode-sdk)
setup_geode_mod(${PROJECT_NAME})

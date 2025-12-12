cmake_minimum_required(VERSION 3.5)

project(ibl_utils LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories("/home/liup/Documents/code/utils/robin-map")
include_directories(${PROJECT_SOURCE_DIR}/../ibl/include)
include_directories(${PROJECT_SOURCE_DIR}/../math/include)
include_directories(${PROJECT_SOURCE_DIR}/../utils/include)


ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/../math math)

ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/../utils utils)

ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/../ibl ibl)

find_package(Qt5Core)

add_executable(ibl_utils
  main.cpp
)
target_link_libraries(ibl_utils ibl math utils pthread dl Qt5::Core)

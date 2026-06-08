cmake_minimum_required(VERSION 3.5)

project(qt_editor LANGUAGES CXX)
if(WIN32)
set(CMAKE_PREFIX_PATH "D:\\sdk\\msvc2017_64")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_definitions(-DPLATFORM_GLFW)

if(APPLE)
    #find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
else()
find_package(Qt5Core)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()

find_library(vulkan_path NAMES vulkan)
add_library(vulkan SHARED IMPORTED)
if(WIN32)
    set_target_properties(vulkan PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../qt/libs/win/vulkan-1.lib)
else()
    set_target_properties(vulkan PROPERTIES IMPORTED_LOCATION ${vulkan_path})
endif()

set(RENDERER_PROJECT_DIR ${PROJECT_SOURCE_DIR}/../../code)

# GLFW: build from source
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${RENDERER_PROJECT_DIR}/third_party/glfw glfw_build)

include_directories(/usr/local/include)
include_directories(${PROJECT_SOURCE_DIR}/../qt/include)

ADD_SUBDIRECTORY(${RENDERER_PROJECT_DIR}/renderer z_renderer)

include_directories(${RENDERER_PROJECT_DIR}/include/renderer/third_party
    ${RENDERER_PROJECT_DIR}/include/renderer
    ${RENDERER_PROJECT_DIR}/hmi_app include/renderer/third_party/jsoncpp)


add_executable(qt_editor
  main.cpp
  ../qt/jsoncpp/jsoncpp.cpp
  toolwindow.cpp
  editor_windows.cpp
  control/custom_treeview.cpp
  control/materal.cpp
)

if(APPLE)
    target_link_libraries(qt_editor  pthread dl z_renderer "-framework Cocoa -framework OpenGL -framework IOKit" glfw)
elseif(WIN32)
    target_link_libraries(qt_editor Qt5::Core Qt5::Widgets glfw z_renderer)
else()
    target_link_libraries(qt_editor Qt5::Core Qt5::Widgets glfw pthread dl X11 z_renderer)
endif()
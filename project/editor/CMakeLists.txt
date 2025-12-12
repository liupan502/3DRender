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

add_library(lib_shaderc STATIC IMPORTED)
if(APPLE)
    set_target_properties(lib_shaderc PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../qt/libs/mac/libshaderc_combined.a)
elseif(WIN32)
    set_target_properties(lib_shaderc PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../qt/libs/win/shaderc_combined.lib)
else()
    set_target_properties(lib_shaderc PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../qt/libs/linux/libshaderc_combined.a)
endif()


find_library(vulkan_path NAMES vulkan)
add_library(vulkan SHARED IMPORTED)
if(WIN32)
    set_target_properties(vulkan PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../qt/libs/win/vulkan-1.lib)
else()
    set_target_properties(vulkan PROPERTIES IMPORTED_LOCATION ${vulkan_path})
endif()


add_library(lib_glfw STATIC IMPORTED)

if(APPLE)
    set_target_properties(lib_glfw PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../qt/libs/mac/libglfw3.a)
elseif(WIN32)
    set_target_properties(lib_glfw PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../qt/libs/win/glfw3.lib)
else()
    set_target_properties(lib_glfw PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../qt/libs/linux/libglfw3.a)
endif()

include_directories(/usr/local/include)
include_directories(${PROJECT_SOURCE_DIR}/../qt/include)

set(RENDERER_PROJECT_DIR ${PROJECT_SOURCE_DIR}/../../code)
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
    target_link_libraries(qt_editor  pthread dl z_renderer "-framework Cocoa -framework OpenGL -framework IOKit" lib_glfw)
elseif(WIN32)
    target_link_libraries(qt_editor Qt5::Core Qt5::Widgets lib_glfw z_renderer)
else()
    target_link_libraries(qt_editor Qt5::Core Qt5::Widgets lib_glfw pthread dl X11 z_renderer)
endif()

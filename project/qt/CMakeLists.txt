cmake_minimum_required(VERSION 3.5)

if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

if (WIN32) 
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

project(qt_console LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_definitions(-DPLATFORM_GLFW)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(APPLE)
    #find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
else()
find_package(Qt5Core)
endif()
find_package(Vulkan)

set(RENDERER_PROJECT_DIR ${PROJECT_SOURCE_DIR}/../../code)

# GLFW: build from source
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${RENDERER_PROJECT_DIR}/third_party/glfw glfw_build)

include_directories(/usr/local/include)
include_directories(${PROJECT_SOURCE_DIR}/include)

ADD_SUBDIRECTORY(${RENDERER_PROJECT_DIR}/renderer z_renderer)

#imgui
set(IMGUI_DIR ${RENDERER_PROJECT_DIR}/third_party/imgui)

include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)

set(IMGUI_CORE_SOURCES
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp 
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp 
    ${IMGUI_DIR}/imgui.cpp 
    ${IMGUI_DIR}/imgui_draw.cpp 
    ${IMGUI_DIR}/imgui_demo.cpp 
    ${IMGUI_DIR}/imgui_tables.cpp 
    ${IMGUI_DIR}/imgui_widgets.cpp
)

include_directories(
    ${RENDERER_PROJECT_DIR}/include/renderer/third_party
    ${RENDERER_PROJECT_DIR}/third_party/vma/include
    ${RENDERER_PROJECT_DIR}/include/renderer
    ${RENDERER_PROJECT_DIR}/include/renderer/core
    ${RENDERER_PROJECT_DIR}/hmi_app 
    include/renderer/third_party/jsoncpp)


add_executable(qt_console
  main.cpp
  jsoncpp/jsoncpp.cpp
  polygon_node.cpp
  message.cpp
  point_set_optimize.cpp
  ${IMGUI_CORE_SOURCES}
)

#set_property(TARGET qt_console PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
if(APPLE)
target_link_libraries(qt_console  pthread dl z_renderer "-framework Cocoa -framework OpenGL -framework IOKit" glfw)
elseif(WIN32)
    target_link_libraries(qt_console Qt5::Core glfw z_renderer)
else()
target_link_libraries(qt_console Qt5::Core glfw pthread dl X11 z_renderer)
endif()


cmake_minimum_required(VERSION 3.4.1)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall")





add_definitions(-DPLATFORM_ANDROID)

ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/renderer)



add_library(hmi3d SHARED
        jni_test.cpp
        apa_adaptor_imp.cpp
        #hmi_app
        hmi_app/common/car.cpp
        hmi_app/common/application.cpp
        hmi_app/common/model_manager.cpp
        hmi_app/common/texture_manager.cpp
        hmi_app/apa/parking_space.cpp
        hmi_app/apa/apa_application.cpp
        )

#add_definitions(-DPLATFORM_ANDROID)



include_directories(include/renderer/third_party include/renderer hmi_app include/renderer/third_party/jsoncpp)




target_link_libraries(hmi3d log android vulkan z_renderer)


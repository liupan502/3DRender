
project(z_renderer)

include_directories(../include/renderer ../include/renderer/third_party ../include/renderer/third_party/jsoncpp )


add_library(z_renderer
        STATIC
        #SHARED#
        #renderer
        src/render_context.cpp
        src/render_frame.cpp
        src/render_target.cpp
        #src/renderer.cpp
        src/multipass_renderer.cpp
        src/avm_renderer.cpp

        # scenegraph
        src/scenegraph/scene.cpp
        src/scenegraph/node.cpp
        src/scenegraph/light_manager.cpp
        src/scenegraph/component.cpp
        src/scenegraph/components/camera.cpp
        src/scenegraph/components/light.cpp
        src/scenegraph/components/material.cpp
        src/scenegraph/components/mesh.cpp
        src/scenegraph/components/sampler.cpp
        src/scenegraph/components/texture.cpp
        src/scenegraph/components/transform.cpp
        src/scenegraph/components/skin.cpp

        #geometry
        src/scenegraph/geometry/rect_node.cpp
        src/scenegraph/geometry/line_path_node.cpp
        src/scenegraph/geometry/obj_node.cpp
        src/scenegraph/geometry/gltf_node.cpp
        src/scenegraph/geometry/geometry_node.cpp

        #animation
        src/scenegraph/components/animation/animation_channel.cpp
        src/scenegraph/components/animation/animation_clip.cpp
        src/scenegraph/components/animation/animation_manager.cpp
        src/scenegraph/components/animation/animation_controller.cpp
        src/scenegraph/components/animation/serialize_frame_animation.cpp

        # core
        src/core/buffer.cpp
        src/core/device.cpp
        src/core/vulkan_resource.cpp
        src/core/command_buffer.cpp
        src/core/command_pool.cpp
        src/core/descriptor.cpp
        src/core/framebuffer.cpp
        src/core/image.cpp
        src/core/image_view.cpp
        src/core/physical_device.cpp
        src/core/pipeline.cpp
        src/core/render_pass.cpp
        src/core/shader_module.cpp
        src/core/swapchain.cpp
        src/core/instance.cpp
        src/core/sampler.cpp
        src/core/vk_common.cpp
        src/core/queue.cpp
        src/core/pipeline_manager.cpp
        src/core/renderers/renderer_interface.cpp
        src/core/renderers/light_renderer.cpp
        src/core/renderers/quad_renderer.cpp
        src/core/renderers/bloom.cpp
        src/core/renderers/color_grading.cpp
        src/core/renderers/taa_renderer.cpp
        src/core/renderers/avm_video_renderer.cpp
        src/core/renderers/sky_renderer.cpp

        # fg
        src/core/fg/frame_graph.cpp
        src/core/fg/fg_render_pass.cpp
        src/core/fg/fg_render_resource.cpp

        #utils
        src/utils/obj_loader.cpp
        src/utils/file_helper.cpp
        src/utils/code_generator.cpp
        src/utils/shader_generator.cpp
        src/utils/point_set_optimize.cpp
        #src/utils/tri.c
        #src/utils/monotone.c
        #src/utils/misc.c
        #src/utils/construct.c
        #src/utils/triangulate_warrper.cpp

        #jsoncpp
        ../include/renderer/third_party/jsoncpp/jsoncpp.cpp
        )


    #set_property(TARGET z_renderer PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    if (ANDROID)
        target_link_libraries(z_renderer log android vulkan lib_shaderc )
    elseif(APPLE)
        target_link_libraries(z_renderer pthread vulkan lib_shaderc  "-framework Cocoa -framework OpenGL -framework IOKit" lib_glfw)
    elseif(WIN32)
        #target_link_libraries(z_renderer vulkan lib_shaderc )
        target_link_libraries(z_renderer ${Vulkan_LIBRARY} lib_shaderc lib_glfw)
    else()
        target_link_libraries(z_renderer pthread vulkan lib_shaderc)
    endif()

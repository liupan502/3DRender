//
// Created by zhida.ji1 on 2022/8/16.
//
#include "renderer.h"

#include "render_context.h"
#include "core/swapchain.h"
#include "render_frame.h"
#include "core/command_pool.h"
#include "core/descriptor.h"
#include "core/image_view.h"
#include "core/image.h"
#include "core/device.h"
#include "core/physical_device.h"
#include "core/swapchain.h"
#include "core/render_pass.h"
#include "core/pipeline.h"
#include "core/pipeline_manager.h"

#include "core/core.h"
#include "scenegraph/scene_graph.h"
#include <scenegraph/components/skin.h>

#include <utils/log.h>
// #include "../vulkan/vulkan_util.h"

using namespace zr;


#ifdef PLATFORM_ANDROID

bool Renderer::init(AAssetManager* asset_mgr, ANativeWindow* window,
                    VkSampleCountFlagBits sample_count) {
    LOGD("renderer init start");
    _context = std::make_shared<RenderContext>();
    _context->init(asset_mgr, window, VK_FORMAT_R8G8B8A8_SRGB);
    LOGD("context init success");
    
    return init_internel(sample_count);
}

#elif  PLATFORM_GLFW
bool  Renderer::init(GLFWwindow* window, VkSampleCountFlagBits sample_count) {
    _context = std::make_shared<RenderContext>();
    _context->init(window, VK_FORMAT_B8G8R8A8_SRGB);
    return init_internel(sample_count);
} 
#endif

bool Renderer::init_internel(VkSampleCountFlagBits sample_count) {
    std::shared_ptr<core::Device> device = _context->get_device();

    std::shared_ptr<core::CommandPool> cmd_pool = _context->get_cmd_pool();
    uint32_t swapchain_len = _context->get_swapchain()->len();
    _render_pass = std::make_shared<core::RenderPass>(_context->get_device(), _context->get_swapchain(),
                                                      true, false, sample_count);
    // LOGD("_render_pass init success");
    /*_graphic_pipeline = std::make_shared<core::BasePipeline>(_context->get_device(),
                                                             _context->get_swapchain()->get_display_size(),
                                                             _render_pass, _context->get_pipeline_layout());*/
    VkExtent2D display_size = _context->get_swapchain()->get_display_size();
    _pipeline_manager = std::make_shared<core::PipelineManager>(_context->get_device(),
                                                                _render_pass,
                                                                display_size);

    for (uint32_t i = 0; i < swapchain_len; i++) {

        std::shared_ptr<RenderFrame> frame = std::make_shared<RenderFrame>();
        frame->set_cmd_buf(std::make_shared<core::CommandBuffer>(device, cmd_pool));
        frame->set_display_view(_context->get_swapchain()->get_display_image_views()[i]);
        frame->set_display_image(_context->get_swapchain()->get_display_images()[i]);
        frame->set_framebuffer(_render_pass->get_framebuffers()[i]);
        _frames.emplace_back(frame);
    }

    init_sync_res();

    _viewport = VkViewport{};
    _viewport.x = 0.0;
    _viewport.y = 0.0;
    _viewport.width = display_size.width;
    _viewport.height = display_size.height;
    _viewport.minDepth = 0.0f;
    _viewport.maxDepth = 1.0f;
    return true;
}

bool Renderer::init_sync_res() {
    VkSemaphoreCreateInfo semapore_ci{};
    semapore_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    CALL_VK(vkCreateSemaphore(_context->get_device()->get_device(), &semapore_ci, nullptr, &_vk_semaphore));

    VkFenceCreateInfo fence_ci{};
    fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    CALL_VK(vkCreateFence(_context->get_device()->get_device(), &fence_ci, nullptr, &_vk_fence));

    return true;
}

void Renderer::set_viewport(int offset_x, int offset_y, uint16_t width, uint16_t height) {
    _viewport.x = offset_x;
    _viewport.y = offset_y;
    _viewport.width = width;
    _viewport.height = height;
}

bool Renderer::begin_frame() {
    // LOGD("begin frame")
    std::shared_ptr<core::Swapchain> swapchain = _context->get_swapchain();
    VkDevice vk_device = _context->get_device()->get_device();
    auto ret = vkAcquireNextImageKHR(vk_device, swapchain->get(),
                                  UINT64_MAX, _vk_semaphore, VK_NULL_HANDLE, &_active_frame);
    if (ret != VK_SUCCESS) {
        return false;
    }

    CALL_VK(vkResetFences(vk_device, 1, &_vk_fence));

    auto frame = _frames[_active_frame];

    std::shared_ptr<core::CommandBuffer> current_cmd_buf = frame->get_cmd_buf();
    current_cmd_buf->begin();
    /*frame->get_display_image()->set_layout(current_cmd_buf->get(), VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    */

    std::vector<VkClearValue> clear_vals{
                {
                        .color = {
                                .float32 = {1.0f, 1.0f, 1.0f, 1.0f}
                        }},
                {
                        .depthStencil = {1.0f, 0}
                },
                {
                        .color = {
                                .float32 = {0.0f, 1.0f, 1.0f, 1.0f}
                        }}};
    _render_pass->begin(current_cmd_buf, frame->get_framebuffer(), clear_vals);
    // _graphic_pipeline->bind(current_cmd_buf);
    return true;
}

void Renderer::end_frame() {
    // LOGD("end frame")
    auto cmd_buf = _frames[_active_frame]->get_cmd_buf();
    _render_pass->end(cmd_buf);
    cmd_buf->end();

    VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkCommandBuffer vk_cmd_buf = _frames[_active_frame]->get_cmd_buf()->get();
    VkSubmitInfo submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &_vk_semaphore,
            .pWaitDstStageMask = &wait_stage_mask,
            .commandBufferCount = 1,
            .pCommandBuffers = &vk_cmd_buf,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr,
    };

    VkQueue vk_queue = _context->get_queue()->get();
    VkDevice vk_device = _context->get_device()->get_device();
    CALL_VK(vkQueueSubmit(vk_queue, 1, &submit_info, _vk_fence));
    VkResult ret = vkWaitForFences(vk_device, 1, &_vk_fence, VK_TRUE, 100000000);
    if (ret != VK_SUCCESS) {
        return;
    }

    VkSwapchainKHR vk_swapchain = _context->get_swapchain()->get();

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 0;
    present_info.pWaitSemaphores = nullptr;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vk_swapchain;
    present_info.pImageIndices = &_active_frame;
    present_info.pResults = &ret;

    vkQueuePresentKHR(vk_queue, &present_info);
}

void Renderer::render_scene(std::shared_ptr<sg::Scene> scene) {

    scene->update();

    scene->upload(_context->get_device());



    std::shared_ptr<sg::PerspectiveCamera> camera = std::dynamic_pointer_cast<sg::PerspectiveCamera>(scene->get_active_camera());
    // VkExtent2D display_size = _viewport.;
    camera->set_aspect_ratio(_viewport.width / (float)_viewport.height);

    scene->pre_frame();
    if (!begin_frame()) {
        return;
    }

    scene->update_light();
    core::LightInfo light_info = scene->get_light_info();

    auto renderable_nodes = scene->get_renderable_nodes();
    auto d_buffers = scene->get_light_manager()->get_directional_light_buffers();
    auto s_buffer = scene->get_light_manager()->get_spot_light_buffer();
    auto p_buffer = scene->get_light_manager()->get_point_light_buffer();
    auto l_buffer = scene->get_light_manager()->get_light_vertex_buffer();
    
    std::vector<std::shared_ptr<sg::Node>> opaque_nodes;
    std::vector<std::shared_ptr<sg::Node>> transparent_nodes;

    for (auto node : renderable_nodes) {
        if (node->is_transparent()) {
            transparent_nodes.emplace_back(node);
        }
        else {
            opaque_nodes.emplace_back(node);
        }
    }

    auto render_nodes = [&](const std::vector<std::shared_ptr<sg::Node>>& input_nodes) {
        uint32_t input_size = input_nodes.size();

        for (uint32_t i = 0; i < input_size; i++) {
            auto node = input_nodes[i];
            std::shared_ptr<sg::Material> material = node->get_component<sg::Material>();
            core::LightInfo node_light_info = material->get_ability().enable_light ? light_info : core::LightInfo();
            _current_graphic_pipeline = _pipeline_manager->get_pipeline(node_light_info, material,
                                                                        node->get_mesh()->get_vtx_attrs());

            if (!_current_graphic_pipeline) {
                LOGD("_current_graphic_pipeline is null ptr");
            }

            core::UniformBufferObject ubo(node->get_component<sg::Transform>()->get_world_matrix(),
                                        camera->get_view(), camera->get_projection(), 
                                        glm::vec4(node->get_visibility(), 0.0, 0.0, 0.0));

            node->get_ubo_buffer()->update((const uint8_t*) &ubo, sizeof(core::UniformBufferObject), 0, false);
            std::shared_ptr<core::DescriptorSet> desc_set = input_nodes[i]->get_desc_set(_current_graphic_pipeline);
            desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{node->get_ubo_buffer()});
            // std::shared_ptr<sg::Material> mat = node->get_component<sg::Material>();
            std::shared_ptr<sg::Texture> tex_diffuse = material->get_tex(sg::TEXTURE_TYPE_DIFFUSE);
            if (tex_diffuse) {
                desc_set->update_desc_set_texture(tex_diffuse, 10);
            }

            if (node_light_info.directional_light_count && material->get_light_enabled()) {
                desc_set->update_desc_set_buffer(d_buffers, node_light_info.directional_light_count);
            }

            if (node_light_info.spot_light_count && material->get_light_enabled()) {
                desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{s_buffer}, node_light_info.spot_light_count);
            }

            if (node_light_info.point_light_count && material->get_light_enabled()) {
                desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{p_buffer}, node_light_info.point_light_count);
            }

            if ((material->get_specular_enabled() || node_light_info.spot_light_count || node_light_info.point_light_count) &&
                    material->get_light_enabled())
            {
                desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{l_buffer});
            }

            auto skin = node->get_component<sg::Skin>();
            if (material->get_skin_enabled() && skin) {
                auto palette_matrices_buf = skin->get_palette_matrices_buf();
                desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{palette_matrices_buf}, 1);
            }

            if (material->get_light_enabled())
            {
                desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{l_buffer});
            }

            auto pbr_buf = material->get_pbr_params_buf();
            if (pbr_buf) {
                desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>({pbr_buf}), 1);
            }

            if (material->get_environment_enabled() &&
                light_info.environment_light_count &&
                material->get_light_enabled()) {
                desc_set->update_desc_set_texture(scene->get_light_manager()->get_prefiltered_tex(), 15);
                desc_set->update_desc_set_texture(scene->get_light_manager()->get_dfg_tex(), 16);
                auto sh_buf = scene->get_light_manager()->get_sh_buf();
                desc_set->update_desc_set_buffer(std::vector<std::shared_ptr<core::UniformBuffer>>{sh_buf});
            }
        }

        for (uint32_t i = 0; i < input_size; i++) {
            auto node = input_nodes[i];
            if (!node->is_visible()) {
                continue;
            }

            std::shared_ptr<sg::Material> material = node->get_component<sg::Material>();
            core::LightInfo node_light_info = material->get_ability().enable_light ? light_info : core::LightInfo();
            _current_graphic_pipeline = _pipeline_manager->get_pipeline(node_light_info, material,
                                                                        node->get_mesh()->get_vtx_attrs());


            auto cmd_buf = _frames[_active_frame]->get_cmd_buf();
            _current_graphic_pipeline->bind(cmd_buf);
            input_nodes[i]->get_desc_set(_current_graphic_pipeline)->bind(cmd_buf, _current_graphic_pipeline->get_pipeline_layout());
            std::shared_ptr<sg::Mesh> mesh = node->get_mesh();
            // LOGD("render mesh")
            render_mesh(mesh);
        }
    };

    render_nodes(opaque_nodes);
    for (auto node : transparent_nodes) {
        auto mat = node->get_component<sg::Material>();
        if (!mat) {
            continue;
        }
        mat->set_cull_mode(1);
    }
    render_nodes(transparent_nodes);
    for (auto node : transparent_nodes) {
        auto mat = node->get_component<sg::Material>();
        if (!mat) {
            continue;
        }
        mat->set_cull_mode(2);
    }
    render_nodes(transparent_nodes);
    for (auto node : transparent_nodes) {
        auto mat = node->get_component<sg::Material>();
        if (!mat) {
            continue;
        }
        mat->set_cull_mode(0);
    }
    end_frame();
}

void Renderer::render_mesh(std::shared_ptr<sg::Mesh> mesh) {
    auto frame = _frames[_active_frame];
    auto cmd_buf = frame->get_cmd_buf();

    std::vector<std::shared_ptr<core::Buffer>> vtx_buffers = mesh->get_vtx_buffers();
    VkDeviceSize offset = 0;
    for (uint32_t i = 0; i < vtx_buffers.size(); i++) {
        VkBuffer vk_buf = vtx_buffers[i]->get();
        vkCmdBindVertexBuffers(cmd_buf->get(), i, 1, &vk_buf, &offset);
    }

    auto index_buffer = mesh->get_index_buffer()->get();
    // vkCmdBindIndexBuffer(cmd_buf->get(), index_buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindIndexBuffer(cmd_buf->get(), index_buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(cmd_buf->get(), mesh->get_indice_count(), 1, 0, 0, 0);
    // vkCmdDrawIndexed(cmd_buf->get(), 3, 1, 0, 0, 0);
}

Renderer::~Renderer() {
    auto device = _context->get_device()->get_device();
    vkDeviceWaitIdle(device);
    vkDestroySemaphore(device, _vk_semaphore, nullptr);
    vkDestroyFence(device, _vk_fence, nullptr);

    _current_graphic_pipeline = nullptr;
    _pipeline_manager = nullptr;

    _targets.clear();
    _frames.clear();
    _render_pass = nullptr;
    _context = nullptr;
}



#include <multipass_renderer.h>
#include <core/device.h>
#include <core/swapchain.h>
#include <core/vk_common.h>
#include <core/renderers/light_renderer.h>
#include <core/renderers/quad_renderer.h>
#include <core/renderers/bloom.h>
#include <core/renderers/color_grading.h>
#include <core/renderers/taa_renderer.h>
#include <core/renderers/sky_renderer.h>
#include <core/pipeline_manager.h>
#include <core/command_buffer.h>
#include <memory>
#include <sstream>
#include <utils/log.h>

using namespace zr;


#ifdef PLATFORM_ANDROID

bool MultiPassRenderer::init(AAssetManager* asset_mgr, ANativeWindow* window,
                    VkSampleCountFlagBits sample_count, VkFormat swapchain_fmt) {
    LOGD("renderer init start");
    _context = std::make_shared<RenderContext>();
    _context->init(asset_mgr, window, swapchain_fmt);
    LOGD("context init success");
    
    return init_internel(sample_count);
}

#elif  PLATFORM_GLFW
bool  MultiPassRenderer::init(GLFWwindow* window, 
            VkSampleCountFlagBits sample_count, VkFormat swapchain_fmt) {
    _context = std::make_shared<RenderContext>();
    _context->init(window, swapchain_fmt);
    return init_internel(sample_count);
} 
#endif

MultiPassRenderer::MultiPassRenderer() {
    _scene = new sg::Scene*;
}

MultiPassRenderer::~MultiPassRenderer() {
    delete _scene;
    _scene = nullptr;

    _fg = nullptr;

}

bool MultiPassRenderer::init_internel(VkSampleCountFlagBits sample_count) {
    init_sync_res();
    _sample_count = sample_count;
    _fg = std::make_shared<core::FrameGraph>();
    // add_transmittance_pass();
    // add_sky_view_pass();
    add_main_pass();
    add_taa_pass();
    add_bloom_pass();
    add_color_grading_pass();
    add_output_pass();
    if (_fg->need_bake()) {
        _fg->bake(_context->get_device(),
                    _context->get_swapchain(), _context->get_cmd_pool());
    }
    return true;
}

void MultiPassRenderer::render_scene(std::shared_ptr<sg::Scene> scene) {
    scene->update();
    scene->upload(_context->get_device());
    scene->update_light();
    scene->pre_frame();
    *_scene = scene.get();
    std::shared_ptr<sg::PerspectiveCamera> camera = std::dynamic_pointer_cast<sg::PerspectiveCamera>(scene->get_active_camera());

    camera->set_aspect_ratio(_viewport.width / (float)_viewport.height);
    prepare_taa();

    uint32_t active_frame_idx = 8;
    std::shared_ptr<core::Swapchain> swapchain = _context->get_swapchain();
    VkDevice vk_device = _context->get_device()->get_device();

    // printf("frame idx %d\n", active_frame_idx);
    VkResult result=vkAcquireNextImageKHR(vk_device, swapchain->get(),
    UINT64_MAX, _vk_semaphore, VK_NULL_HANDLE, &active_frame_idx);
    if(VK_SUCCESS !=result){
        return;
    }

    _fg->execute(active_frame_idx, _context->get_device());

    VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkCommandBuffer vk_cmd_buf = _fg->get_command_buf(active_frame_idx)->get();
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
    // VkDevice vk_device = _context->get_device()->get_device();
    // CALL_VK(vkQueueSubmit(vk_queue, 1, &submit_info, _vk_fence));
    CALL_VK(vkResetFences(vk_device, 1, &_vk_fence));
    auto fence_status = vkGetFenceStatus(_context->get_device()->get_device(), _vk_fence);
    auto tmp_ret = vkQueueSubmit(vk_queue, 1, &submit_info, _vk_fence);
    CALL_VK(vkWaitForFences(vk_device, 1, &_vk_fence, VK_TRUE, 100000000));

    VkSwapchainKHR vk_swapchain = _context->get_swapchain()->get();
    VkResult ret;

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 0;
    present_info.pWaitSemaphores = nullptr;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vk_swapchain;
    present_info.pImageIndices = &active_frame_idx;
    present_info.pResults = &ret;
    vkQueuePresentKHR(vk_queue, &present_info);
    ++_frame_id;
}


bool MultiPassRenderer::init_sync_res() {
    VkSemaphoreCreateInfo semapore_ci{};
    semapore_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    CALL_VK(vkCreateSemaphore(_context->get_device()->get_device(), 
            &semapore_ci, nullptr, &_vk_semaphore));

    VkFenceCreateInfo fence_ci{};
    fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    CALL_VK(vkCreateFence(_context->get_device()->get_device(), &fence_ci, nullptr, &_vk_fence));

    return true;
}

void MultiPassRenderer::prepare_taa() {
    // disable taa
    if (!get_taa_render()) {
        return;
    }
    auto camera = (*_scene)->get_active_camera();
    _camera_info.proj_mat = camera->get_projection() /* camera->get_view()*/;
    _camera_info.view_mat = camera->get_view();

    glm::vec2 jitter = core::TaaRenderer::halton(_frame_id / 1) - glm::vec2(0.5f);
    glm::vec2 size = 2.0f / glm::vec2(_viewport.width, _viewport.height);
    jitter *= size;
    _camera_info.proj_mat[2].x -= jitter.x;
    _camera_info.proj_mat[2].y -= jitter.y;

    if (get_taa_render()) {
        get_taa_render()->set_jitter(jitter);
    }
}

std::shared_ptr<core::LightRenderer>  MultiPassRenderer::get_light_render(){
    if (_render_map.find("light_render") == _render_map.end()) {
        return nullptr;
    }
    return std::dynamic_pointer_cast<core::LightRenderer>(_render_map["light_render"][0]);
}

std::shared_ptr<core::TaaRenderer> MultiPassRenderer::get_taa_render() {
    if (_render_map.find("taa_render") == _render_map.end()) {
        return nullptr;
    }

    return std::dynamic_pointer_cast<core::TaaRenderer>(_render_map["taa_render"][0]);
}

void MultiPassRenderer::add_main_pass() {
    std::shared_ptr<core::FgRenderPass> main_pass = _fg->add_pass("main_pass", false);
    core::FgRenderpassSetupData set_up;
    set_up.pp_scene = _scene;
    set_up.renderer_interface = std::make_shared<core::LightRenderer>();
    set_up.renderer_interface->set_aa_option(core::AA_OPTION_TAA);
    set_up.renderer_interface->set_camera_info(&_camera_info);
    // set_up.renderer_interface = std::make_shared<core::QuadRenderer>();
    main_pass->set_setup_data(set_up);

    std::shared_ptr<core::Swapchain> sc = _context->get_swapchain();

    core::AttachmentInfo color_output_info{};
    color_output_info.fmt = /*VK_FORMAT_R16G16B16A16_SFLOAT*/VK_FORMAT_B10G11R11_UFLOAT_PACK32 /*sc->get_suitable_format()*/;
    color_output_info.depth = 1;
    color_output_info.width = sc->get_display_size().width;
    color_output_info.height = sc->get_display_size().height;
    color_output_info.img_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (_sample_count != VK_SAMPLE_COUNT_1_BIT) {
        color_output_info.img_usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    }
    color_output_info.layer = 0;
    color_output_info.level = 0;
    color_output_info.samples = _sample_count;
    color_output_info.clear_val = VkClearValue {
        .color = {1.0f, 1.0f, 1.0f, 1.0f}
    };
    color_output_info.is_reused = false;
    // color_output_info.store_op = VK_ATTACHMENT_STORE_OP_STORE;
    color_output_info.img_name = "main_pass_color_output_img";

    std::string pass_name = "main_pass_color_output";
    if (_sample_count != VK_SAMPLE_COUNT_1_BIT) {
        pass_name = "tmp_main_pass_color_output";
    }
    else {
        color_output_info.store_op = VK_ATTACHMENT_STORE_OP_STORE;
    }
    main_pass->add_color_output(pass_name, color_output_info);

    core::AttachmentInfo velocity_output_info{};
    velocity_output_info.fmt = VK_FORMAT_R16G16_SFLOAT /*VK_FORMAT_B10G11R11_UFLOAT_PACK32*/;
    velocity_output_info.depth = 1;
    velocity_output_info.width = sc->get_display_size().width;
    velocity_output_info.height = sc->get_display_size().height;
    velocity_output_info.img_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    velocity_output_info.layer = 0;
    velocity_output_info.level = 0;
    velocity_output_info.samples = _sample_count;
    velocity_output_info.clear_val = VkClearValue {
        .color = {0.0f, 0.0f, 0.0f, 0.0f}
    };
    velocity_output_info.is_reused = false;
    velocity_output_info.img_name = "main_pass_velocity_output_img";
    velocity_output_info.store_op = VK_ATTACHMENT_STORE_OP_STORE;
    std::string attachment_name = "main_pass_velocity_output";
    main_pass->add_color_output(attachment_name, velocity_output_info);


    // resloved attchment
    if (_sample_count != VK_SAMPLE_COUNT_1_BIT) {
        core::AttachmentInfo resloved_output_info = color_output_info;
        resloved_output_info.samples = VK_SAMPLE_COUNT_1_BIT;
        resloved_output_info.clear_val = VkClearValue {
            .color = {0.0f, 1.0f, 0.0f, 1.0f}
        };
        resloved_output_info.store_op = VK_ATTACHMENT_STORE_OP_STORE;
        resloved_output_info.img_name = "main_pass_resloved_output_img";
        main_pass->add_reslove_output("main_pass_color_output", resloved_output_info);
    }

    // core::AttachmentInfo reslove_output_info{};
    // main_pass->add_reslove_output("main_pass_reslove_output", reslove_output_info);

    core::AttachmentInfo depth_stencil_info = color_output_info;
    depth_stencil_info.fmt = VK_FORMAT_D32_SFLOAT;
    depth_stencil_info.img_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT /*|
                                     VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT*/;

    depth_stencil_info.clear_val = VkClearValue {
        .depthStencil = {1.0f, 0}
    };
    depth_stencil_info.store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_stencil_info.img_name = "main_pass_depth_stencil_output_img";
    depth_stencil_info.is_reused = false;
    main_pass->add_depth_stencil_output("main_pass_depth_stencil_output", depth_stencil_info);
}


void MultiPassRenderer::add_output_pass() {
    std::shared_ptr<core::FgRenderPass> output_pass = _fg->add_pass("output_pass", true);

    core::FgRenderpassSetupData set_up;
    set_up.pp_scene = _scene;
    set_up.renderer_interface = std::make_shared<core::QuadRenderer>();
    output_pass->set_setup_data(set_up);
    // output_pass->add_color_input("main_pass_color_resloved_output");
    // output_pass->add_texture_sample("main_pass_color_resloved_output");
    // std::string tex_sample_name = "main_pass_color_output";
    // std::string tex_sample_name = "bloom_upsample_pass_color_output3";
    std::string tex_sample_name = "color_grading_pass_output";
    // std::string tex_sample_name = "taa_pass_color_output";
    // std::string tex_sample_name = "bloom_downsample_pass_color_output3";
    // std::string tex_sample_name = "sky_view_pass_output";
    // std::string tex_sample_name = "transmittance_pass_output";
    output_pass->add_texture_sample(tex_sample_name);
    std::shared_ptr<core::Swapchain> sc = _context->get_swapchain();
    core::AttachmentInfo color_output_info{};
    color_output_info.fmt = sc->get_suitable_format();
    color_output_info.depth = 1;
    color_output_info.width = sc->get_display_size().width;
    color_output_info.height = sc->get_display_size().height;
    color_output_info.img_usage =  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    color_output_info.layer = 0;
    color_output_info.level = 0;
    color_output_info.samples = VK_SAMPLE_COUNT_1_BIT;
    color_output_info.clear_val = VkClearValue {
        .color = {1.0f, 0.0f, 1.0f, 1.0f}
    };
    color_output_info.store_op = VK_ATTACHMENT_STORE_OP_STORE;
    color_output_info.img_name = "output_pass_color_output_img";
    output_pass->add_color_output("output_pass_color_output", color_output_info);
}

void MultiPassRenderer::add_taa_pass() {
    // TaaRenderer renderer;
    core::TaaInfo taa_info{};
    // uint32_t size = sizeof(taa_info);
    // uint32_t tmp = size + 1;

    std::shared_ptr<core::Swapchain> sc = _context->get_swapchain();
    auto width = sc->get_display_size().width;
    auto height = sc->get_display_size().height;

    std::shared_ptr<core::FgRenderPass> taa_pass = _fg->add_pass("taa_pass", false);
    core::FgRenderpassSetupData set_up;
    set_up.pp_scene = _scene;
    auto taa_renderer = std::make_shared<core::TaaRenderer>();
    taa_renderer->set_frame_size(glm::vec2(width, height));
    _render_map.insert({"taa_render", {taa_renderer}});
    set_up.renderer_interface = taa_renderer;
    taa_pass->set_setup_data(set_up);

    std::string tex_sample_name = "main_pass_color_output";
    taa_pass->add_texture_sample(tex_sample_name);

    // taa_pass->add_texture_sample("main_pass_depth_stencil_output");

    taa_pass->add_texture_sample("main_pass_velocity_output");



    
    core::AttachmentInfo taa_output_info{};
    taa_output_info.fmt = /*VK_FORMAT_R16G16B16A16_SFLOAT*/ VK_FORMAT_B10G11R11_UFLOAT_PACK32 /*sc->get_suitable_format()*/;
    taa_output_info.depth = 1;
    taa_output_info.width = width;
    taa_output_info.height = height;
    taa_output_info.img_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    taa_output_info.layer = 0;
    taa_output_info.level = 0;
    taa_output_info.samples = VK_SAMPLE_COUNT_1_BIT;
    taa_output_info.clear_val = VkClearValue {
        .color = {0.0f, 0.0f, 1.0f, 1.0f}
    };
    taa_output_info.is_reused = true;
    taa_output_info.store_op = VK_ATTACHMENT_STORE_OP_STORE;
    taa_output_info.img_name = "taa_pass_color_output_img";

    taa_pass->add_color_output("taa_pass_color_output", taa_output_info);

}

void MultiPassRenderer::add_bloom_pass() {
    uint8_t level = 4;


    // add down sample passes
    std::shared_ptr<core::Swapchain> sc = _context->get_swapchain();
    uint16_t width = sc->get_display_size().width / 2;
    uint16_t height = sc->get_display_size().height / 2;

    // uint16_t width = sc->get_display_size().width;
    // uint16_t height = sc->get_display_size().height;
    std::stringstream ss;
    
    for (uint8_t i = 0; i < level; i++) {

        ss.str("");
        ss << "bloom_down_sample_" << i;
        std::string pass_name = ss.str();
        std::shared_ptr<core::FgRenderPass> bloom_down_sample_pass = _fg->add_pass(pass_name);
        core::FgRenderpassSetupData set_up;
        set_up.pp_scene = _scene;
        // set_up.renderer_interface 
        auto renderer = std::make_shared<core::BloomDownSampleRenderer>();
        core::BloomDownSampleRenderer::Parameters params = {};
        params.level = i;
        params.inv_highlight = 0.05f;
        params.threshold = 1.0f;
        renderer->set_parameters(params);
        set_up.renderer_interface = renderer;
        bloom_down_sample_pass->set_setup_data(set_up);

        core::AttachmentInfo color_output_info{};
        color_output_info.fmt = VK_FORMAT_B10G11R11_UFLOAT_PACK32 /*VK_FORMAT_R16G16B16A16_SFLOAT*/;
        color_output_info.depth = 1;
        color_output_info.width = width >> i;
        color_output_info.height = height >> i;
        color_output_info.img_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        color_output_info.layer = 0;
        color_output_info.level = i;
        color_output_info.samples = VK_SAMPLE_COUNT_1_BIT;
        color_output_info.clear_val = VkClearValue {
            .color = {0.0f, 0.0f, 0.0f, 1.0f}
        };
        color_output_info.store_op = VK_ATTACHMENT_STORE_OP_STORE;
        // color_output_info.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_output_info.img_name = "bloom_downsample_pass_color_output_img";
        ss.str("");
        ss << "bloom_downsample_pass_color_output" << std::to_string(i);
        std::string color_output_name = ss.str();
        // LOGD(color_output_name.c_str());
        bloom_down_sample_pass->add_color_output(color_output_name, color_output_info);
        
        std::string tex_name = "";
        if (i == 0) {
            // tex_name = "main_pass_color_output";
            tex_name = "taa_pass_color_output";
        }
        else {
            ss.str("");
            ss << "bloom_downsample_pass_color_output" << std::to_string(i - 1);
            tex_name = ss.str();
        }
        bloom_down_sample_pass->add_texture_sample(tex_name);
    }

    // add up sample passes
    for (uint8_t i = 0; i < level; i++) {
        ss.str("");
        ss << "bloom_up_sample_" << i;
        std::string pass_name = ss.str();

        std::shared_ptr<core::FgRenderPass> bloom_up_sample_pass = _fg->add_pass(pass_name);
        core::FgRenderpassSetupData set_up;
        set_up.pp_scene = _scene;
        // set_up.renderer_interface 
        auto renderer = std::make_shared<core::BloomUpSampleRenderer>();
        core::BloomUpSampleRenderer::Parameters params{};
        params.level = (level - i - 1);
        params.resolution[2] = (1.0f / (width >> (level - i - 1)));
        params.resolution[3] = (1.0f / (height >> (level - i - 1)));
        renderer->set_parameters(params);
        set_up.renderer_interface = renderer;
        bloom_up_sample_pass->set_setup_data(set_up);

        core::AttachmentInfo color_output_info{};
        color_output_info.fmt = VK_FORMAT_B10G11R11_UFLOAT_PACK32 /*VK_FORMAT_R16G16B16A16_SFLOAT*/;
        color_output_info.depth = 1;
        color_output_info.width = width >> (level - i - 1);
        color_output_info.height = height >> (level - i - 1);
        color_output_info.img_usage =  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        color_output_info.layer = 0;
        color_output_info.level = level - i - 1;
        color_output_info.samples = VK_SAMPLE_COUNT_1_BIT;
        color_output_info.clear_val = VkClearValue {
            .color = {0.0f, 0.0f, 0.0f, 1.0f}
        };
        color_output_info.img_name = "bloom_upsample_pass_color_output_img";
        color_output_info.store_op = VK_ATTACHMENT_STORE_OP_STORE;
        ss.str("");
        ss << "bloom_upsample_pass_color_output" << std::to_string(i);
        std::string color_output_name = ss.str();
        // LOGD(color_output_name.c_str());
        bloom_up_sample_pass->add_color_output(color_output_name, color_output_info);

        std::string tex_name = "";
        if (i == 0) {
            tex_name = "bloom_downsample_pass_color_output3";
        }
        else {
            ss.str("");
            ss << "bloom_upsample_pass_color_output" << std::to_string(i - 1);
            tex_name = ss.str();
        }
        bloom_up_sample_pass->add_texture_sample(tex_name);
    }
}


void MultiPassRenderer::add_color_grading_pass() {
    std::shared_ptr<core::FgRenderPass> color_grading_pass = _fg->add_pass("color_grading_pass", true);

    core::FgRenderpassSetupData set_up;
    set_up.pp_scene = _scene;
    auto renderer = std::make_shared<core::ColorGradingRenderer>();
    renderer->set_color_grading_path("shaders/color_grading.bin");
    set_up.renderer_interface = renderer;
    color_grading_pass->set_setup_data(set_up);

    std::shared_ptr<core::Swapchain> sc = _context->get_swapchain();
    uint16_t width = sc->get_display_size().width;
    uint16_t height = sc->get_display_size().height;

    core::AttachmentInfo color_output_info{};
    color_output_info.fmt = /*VK_FORMAT_B10G11R11_UFLOAT_PACK32*/ sc->get_suitable_format()/*VK_FORMAT_R16G16B16A16_SFLOAT*/;
    color_output_info.depth = 1;
    color_output_info.width = width;
    color_output_info.height = height;
    color_output_info.img_usage =  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    color_output_info.layer = 0;
    color_output_info.level = 0;
    color_output_info.samples = VK_SAMPLE_COUNT_1_BIT;
    color_output_info.clear_val = VkClearValue {
        .color = {1.0f, 0.0f, 1.0f, 1.0f}
    };
    color_output_info.store_op = VK_ATTACHMENT_STORE_OP_STORE;
    color_output_info.img_name = "color_grading_output_img";
    color_grading_pass->add_color_output("color_grading_pass_output", color_output_info);

    color_grading_pass->add_texture_sample("taa_pass_color_output");
    color_grading_pass->add_texture_sample("bloom_upsample_pass_color_output3");
}

void MultiPassRenderer::add_transmittance_pass() {
    std::shared_ptr<core::FgRenderPass> transmittance_pass = _fg->add_pass("transmittance_pass");

    core::FgRenderpassSetupData set_up;
    set_up.pp_scene = _scene;
    set_up.renderer_interface = std::make_shared<core::TransmittanceLutRenderer>();
    transmittance_pass->set_setup_data(set_up);
    
    core::AttachmentInfo color_output_info{};
    color_output_info.fmt = VK_FORMAT_R16G16B16A16_SFLOAT;
    color_output_info.depth = 1;
    color_output_info.width = 256;
    color_output_info.height = 64;
    color_output_info.img_usage =  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    color_output_info.layer = 0;
    color_output_info.level = 0;
    color_output_info.samples = VK_SAMPLE_COUNT_1_BIT;
    color_output_info.clear_val = VkClearValue {
        .color = {1.0f, 0.0f, 1.0f, 1.0f}
    };
    color_output_info.store_op = VK_ATTACHMENT_STORE_OP_STORE;
    color_output_info.img_name = "transmittance_output_img";

    transmittance_pass->add_color_output("transmittance_pass_output", color_output_info);
}

void MultiPassRenderer::add_sky_view_pass() {
    std::shared_ptr<core::FgRenderPass> sky_view_pass = _fg->add_pass("sky_view_pass");

    core::FgRenderpassSetupData set_up;
    set_up.pp_scene = _scene;
    set_up.renderer_interface = std::make_shared<core::SkyViewLutRenderer>();
    sky_view_pass->set_setup_data(set_up);
    
    core::AttachmentInfo color_output_info{};
    color_output_info.fmt = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    color_output_info.depth = 1;
    color_output_info.width = 192;
    color_output_info.height = 108;
    color_output_info.img_usage =  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    color_output_info.layer = 0;
    color_output_info.level = 0;
    color_output_info.samples = VK_SAMPLE_COUNT_1_BIT;
    color_output_info.clear_val = VkClearValue {
        .color = {1.0f, 0.0f, 1.0f, 1.0f}
    };
    color_output_info.store_op = VK_ATTACHMENT_STORE_OP_STORE;
    color_output_info.img_name = "sky_view_output_img";

    sky_view_pass->add_color_output("sky_view_pass_output", color_output_info);
    sky_view_pass->add_texture_sample("transmittance_pass_output");
}

void MultiPassRenderer::set_viewport(int offset_x, int offset_y, uint16_t width, uint16_t height) {
    _viewport = VkViewport{};
    _viewport.minDepth = 0.0f;
    _viewport.maxDepth = 1.0f;
    _viewport.width = width;
    _viewport.height = height;
    _viewport.x = offset_x;
    _viewport.y = offset_y;

    _fg->set_viewport(_viewport);
    if (get_taa_render()) {
        get_taa_render()->set_frame_size(glm::vec2(width, height));
    }
}

void MultiPassRenderer::wait_idle() {
    auto device = _context->get_device()->get_device();
    vkDeviceWaitIdle(device);
}


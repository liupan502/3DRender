//
// Created by zhida.ji1 on 2022/8/9.
//
#include <core/core.h>
#include <utils/file_helper.h>
#include <utils/log.h>
#include <string.h>

using namespace  zr::core;

void ShaderModule::load_shader_from_file(const char *file_path) {

    std::string content = utils::FileHelper().load_content(file_path);
    VkShaderModuleCreateInfo shader_module_ci;
    
    shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_ci.pNext = nullptr;
    shader_module_ci.flags = 0;
    shader_module_ci.codeSize = content.size();
    shader_module_ci.pCode = (const uint32_t*) content.c_str();
    
    CALL_VK(vkCreateShaderModule(_device->get_device(), &shader_module_ci, nullptr, &_vk_shader_module));
}

ShaderModule::ShaderModule(std::shared_ptr<Device> device, const char *file_path) : _device(device) {
    load_shader_from_file(file_path);
}

ShaderModule::~ShaderModule() {
    vkDestroyShaderModule(_device->get_device(), _vk_shader_module, nullptr);
}
#include <core/fg/fg_render_resource.h>

using namespace zr::core;

FgRenderTextureResource::FgRenderTextureResource(const std::string& name) : 
    FgRenderResource(name) {
    _type = RENDER_RESOURCE_TYPE_TEXTURE;
    _img_usage = 0;
}
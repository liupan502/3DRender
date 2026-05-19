#include <texture_cache.h>
#include <rhi/rhi.h>

namespace zr {
namespace core {

TexturePool& TexturePool::instance() {
    static TexturePool inst;
    return inst;
}

rhi::TextureRef TexturePool::acquire(const rhi::TextureCreateInfo& ci) {
    std::lock_guard<std::mutex> lock(_mutex);

    for (auto& entry : _pool) {
        if (!entry.available) {
            continue;
        }
        const auto& eci = entry.ci;
        if (eci.width == ci.width &&
            eci.height == ci.height &&
            eci.depth == ci.depth &&
            eci.mip_num == ci.mip_num &&
            eci.layer_num == ci.layer_num &&
            eci.type == ci.type &&
            eci.format == ci.format &&
            eci.flags == ci.flags) {
            entry.available = false;
            return entry.tex;
        }
    }

    rhi::TextureRef tex = rhi::rhi_instance->create_texture(ci);
    _pool.push_back({tex, ci, false});
    return tex;
}

void TexturePool::release(const rhi::TextureRef& tex) {
    std::lock_guard<std::mutex> lock(_mutex);

    for (auto& entry : _pool) {
        if (entry.tex.get() == tex.get()) {
            entry.available = true;
            return;
        }
    }
}

void TexturePool::clear() {
    std::lock_guard<std::mutex> lock(_mutex);
    _pool.clear();
}

}
}

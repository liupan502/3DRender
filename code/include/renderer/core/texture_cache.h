#pragma once

#include <rhi/rhi.h>
#include <vector>
#include <mutex>

namespace zr {
namespace core {

class TexturePool {
public:
    static TexturePool& instance();

    rhi::TextureRef acquire(const rhi::TextureCreateInfo& ci);

    void release(const rhi::TextureRef& tex);

    void clear();

    TexturePool(const TexturePool&) = delete;
    TexturePool& operator=(const TexturePool&) = delete;

private:
    TexturePool() = default;

    struct Entry {
        rhi::TextureRef tex;
        rhi::TextureCreateInfo ci;
        bool available;
    };

    std::vector<Entry> _pool;
    std::mutex _mutex;
};

}
}

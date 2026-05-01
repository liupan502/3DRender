#pragma once

#include <rhi/rhi_definitions.h>

namespace rhi {
    class Resource {

    };

    enum class ColorFormat : uint8_t {
        None = 0,
        R8G8B8A8_UNORM = 1,
        R8G8B8A8_SRGB = 2,
        R8G8B8A8_SNORM = 3,
        R8G8B8A8_UINT = 4,
        R8G8B8A8_SINT = 5,
        R16G16B16A16_SFLOAT = 6,
        R32G32B32A32_SFLOAT = 7
    };

    enum class TextureType : uint8_t {
        Texture2D,
        Texture2DArray,
        Texture3D,
        TextureCube,
        TextureCubeArray
    };

    enum class TextureCreateFlags : uint64_t {
        None = 0,
        RenderTargetable                  = 1ull << 0,
        ResolveTargetable                 = 1ull << 1,
        DepthStencilTargetable            = 1ull << 2,
        ShaderResource                    = 1ull << 3,
        CPUWritable                       = 1ull << 4,
        Dynamic                           = 1ull << 5,
        InputAttachmentRead               = 1ull << 6,
        Memoryless                        = 1ull << 7,
        Presentable                       = 1ull << 8,
        CPUReadback                       = 1ull << 9,
        DepthStencilResolveTarget         = 1ull << 10,
    };

    struct TextureCreateInfo {
    public:
        uint16_t width = 1;
        uint16_t height = 1;
        uint16_t depth = 1;
        uint16_t mip_num = 1;
        uint16_t layer_num = 1;
        TextureType type = TextureType::Texture2D;
        ColorFormat format = ColorFormat::R8G8B8A8_UNORM;
        TextureCreateFlags flags = TextureCreateFlags::None;
    };

    class Texture : public Resource {
    public:
    
    protected:
    TextureCreateInfo _create_info;
    };

    enum class BufferUsageFlags : uint32_t {
        None = 0,
        VertexBuffer = 1u << 1,
        IndexBuffer = 1u << 2,
        UniformBuffer = 1u << 3,
        StorageBuffer = 1u << 4,
        IndirectBuffer = 1u << 5,
        Static = 1u << 6,
        Dynamic = 1u << 7,
        ShaderResource = 1u << 8,
        CopySrc = 1u << 9,
        CopyDst = 1u << 10
    };

    struct BufferCreateInfo {
    public:
       uint32_t size = 0; 
       uint32_t stride = 0;
       BufferUsageFlags usage = BufferUsageFlags::None;   
    };


    class Buffer : public Resource {
    public:
    protected:
        BufferCreateInfo _create_info;
    };

    struct SampleStateCreateInfo {
        SamplerFilterType min_filter_type = SamplerFilterType::SF_LINEAR;
        SamplerFilterType mag_filter_type = SamplerFilterType::SF_LINEAR;
        SamplerAddressMode address_u = SamplerAddressMode::SAM_REPEAT;
        SamplerAddressMode address_v = SamplerAddressMode::SAM_REPEAT;
        SamplerAddressMode address_w = SamplerAddressMode::SAM_REPEAT;
        float mip_bias = 0;
        uint32_t max_anisotropy = 0;
        float min_mip_level = 0;
        float max_mip_level = 1000.0;
        uint32_t border_color = 0;
        SamplerCompareFunction scf = SamplerCompareFunction::SCF_NEVER;
    };

    class SampleState : public Resource {

    };
};
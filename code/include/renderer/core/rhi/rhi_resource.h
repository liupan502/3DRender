#pragma once

#include <rhi/rhi_definitions.h>
#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>

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
        R32G32B32A32_SFLOAT = 7,
        R32G32_SFLOAT = 8,
        R32G32B32_SFLOAT = 9,
        R16G16_SFLOAT = 10,
        B10G11R11_UFLOAT_PACK32 = 11,
        D32_SFLOAT = 12,
        B8G8R8A8_SRGB = 13,
        R8G8B8_UINT = 14,
        R16G16B16A16_UINT
    };

    enum class TextureType : uint8_t {
        Texture2D,
        Texture2DArray,
        Texture3D,
        TextureCube,
        TextureCubeArray
    };

    enum class TextureCreateFlagBit : uint64_t {
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
    using TextureCreateFlags = uint64_t;

    inline constexpr TextureCreateFlags operator|(TextureCreateFlagBit lhs, TextureCreateFlagBit rhs) noexcept {
        return static_cast<TextureCreateFlags>(lhs) | static_cast<TextureCreateFlags>(rhs);
    }
    inline constexpr TextureCreateFlags operator|(TextureCreateFlags lhs, TextureCreateFlagBit rhs) noexcept {
        return lhs | static_cast<TextureCreateFlags>(rhs);
    }
    inline constexpr TextureCreateFlags operator|(TextureCreateFlagBit lhs, TextureCreateFlags rhs) noexcept {
        return static_cast<TextureCreateFlags>(lhs) | rhs;
    }
    inline constexpr TextureCreateFlags& operator|=(TextureCreateFlags& lhs, TextureCreateFlagBit rhs) noexcept {
        return lhs = lhs | static_cast<TextureCreateFlags>(rhs);
    }
    inline constexpr TextureCreateFlags operator&(TextureCreateFlags lhs, TextureCreateFlagBit rhs) noexcept {
        return lhs & static_cast<TextureCreateFlags>(rhs);
    }

    struct TextureCreateInfo {
    public:
        uint16_t width = 1;
        uint16_t height = 1;
        uint16_t depth = 1;
        uint16_t mip_num = 1;
        uint16_t layer_num = 1;
        TextureType type = TextureType::Texture2D;
        ColorFormat format = ColorFormat::R8G8B8A8_UNORM;
        TextureCreateFlags flags = 0;
    };

    class Texture : public Resource {
    public:
    
    protected:
    TextureCreateInfo _create_info;
    };

    enum class BufferUsageFlagBit : uint32_t {
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
    using BufferUsageFlags = uint32_t;

    inline constexpr BufferUsageFlags operator|(BufferUsageFlagBit lhs, BufferUsageFlagBit rhs) noexcept {
        return static_cast<BufferUsageFlags>(lhs) | static_cast<BufferUsageFlags>(rhs);
    }
    inline constexpr BufferUsageFlags operator|(BufferUsageFlags lhs, BufferUsageFlagBit rhs) noexcept {
        return lhs | static_cast<BufferUsageFlags>(rhs);
    }
    inline constexpr BufferUsageFlags operator|(BufferUsageFlagBit lhs, BufferUsageFlags rhs) noexcept {
        return static_cast<BufferUsageFlags>(lhs) | rhs;
    }
    inline constexpr BufferUsageFlags& operator|=(BufferUsageFlags& lhs, BufferUsageFlagBit rhs) noexcept {
        return lhs = lhs | static_cast<BufferUsageFlags>(rhs);
    }
    inline constexpr BufferUsageFlags operator&(BufferUsageFlags lhs, BufferUsageFlagBit rhs) noexcept {
        return lhs & static_cast<BufferUsageFlags>(rhs);
    }

    struct BufferCreateInfo {
    public:
       uint32_t size = 0; 
       uint32_t stride = 0;
       BufferUsageFlags usage = 0;   
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

    struct ShaderModuleCreateInfo {
        ShaderModuleType type = ShaderModuleType::SMT_NONE;
        uint8_t* content = nullptr;
        uint32_t len = 0;
        bool is_bin = true;
    };

    class ShaderModule : public Resource {
        public:
        ShaderModule(const ShaderModuleCreateInfo& info) : _ci(info) {};
        protected:
        ShaderModuleCreateInfo _ci;
    };

    struct VertexAttributeDesc {
        uint32_t location = 0;
        uint32_t binding = 0;
        ColorFormat format = ColorFormat::R32G32B32_SFLOAT;
        uint32_t offset = 0;
    };

    struct VertexBindingDesc {
        uint32_t binding = 0;
        uint32_t stride = 0;
        VertexInputRate input_rate = VertexInputRate::VIR_VERTEX;
    };

    struct ColorBlendAttachmentState {
        bool blend_enable = false;
        BlendFactor src_color_factor = BlendFactor::BF_SRC_ALPHA;
        BlendFactor dst_color_factor = BlendFactor::BF_ONE_MINUS_SRC_ALPHA;
        BlendOp color_blend_op = BlendOp::BO_ADD;
        BlendFactor src_alpha_factor = BlendFactor::BF_ZERO;
        BlendFactor dst_alpha_factor = BlendFactor::BF_ONE;
        BlendOp alpha_blend_op = BlendOp::BO_ADD;
        uint8_t color_write_mask = 0xF;
    };

    struct Viewport {
        int32_t left = 0;
        int32_t top = 0;
        uint32_t width = 0;
        uint32_t height = 0;
    };

    class DescriptorSetLayout;

    struct GraphicsPipelineCreateInfo {
        std::shared_ptr<ShaderModule> vertex_shader = nullptr;
        std::shared_ptr<ShaderModule> fragment_shader = nullptr;

        std::vector<VertexBindingDesc> vertex_bindings;
        std::vector<VertexAttributeDesc> vertex_attributes;

        PrimitiveTopology topology = PrimitiveTopology::PT_TRIANGLE_LIST;
        bool primitive_restart_enable = false;

        PolygonMode polygon_mode = PolygonMode::PM_FILL;
        CullMode cull_mode = CullMode::CM_BACK;
        FrontFace front_face = FrontFace::FF_COUNTER_CLOCKWISE;
        bool depth_clamp_enable = false;
        float line_width = 1.0f;

        SampleCount sample_count = SampleCount::SC_COUNT_1;

        bool depth_test_enable = true;
        bool depth_write_enable = true;
        CompareOp depth_compare_op = CompareOp::CO_LESS;

        std::vector<ColorBlendAttachmentState> color_blend_attachments;

        Viewport viewport;

        uint32_t subpass = 0;

        std::shared_ptr<DescriptorSetLayout> descriptor_set_layout;
    };

    class GraphicsPipeline : public Resource {
    public:
        GraphicsPipeline(const GraphicsPipelineCreateInfo& info) : _ci(info) {};
        virtual ~GraphicsPipeline() = default;
    protected:
        GraphicsPipelineCreateInfo _ci;
    };

    struct AttachmentInfo {
        public:
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;        
        rhi::ColorFormat fmt = ColorFormat::None;
        rhi::SampleCount samples = SampleCount::SC_COUNT_1;
        rhi::AttachmentLoadOp load_op = AttachmentLoadOp::ALO_LOAD;
        rhi::AttachmentStoreOp store_op = AttachmentStoreOp::ASO_STORE;
        uint16_t level = 0;
        uint16_t layer = 0;
        uint16_t mip_num = 1;
        uint16_t layer_num = 1;    

        rhi::TextureCreateFlags img_usage = static_cast<uint64_t>(TextureCreateFlagBit::ShaderResource);
        glm::vec4 color_clear_val;
        float depth_clear_val;
        uint32_t stencil_clear_val;
        std::string img_name;
        // 对应的image是否在不同的frame buffer 中复用，如果复用则不同的frame buffer 中的image view 指向
        // 相同的image，反之则image 与image view一一对应
        bool is_reused = true;
    };

    using Attachment = std::pair<AttachmentInfo, std::shared_ptr<Texture>>;
    struct RenderTargetCreateInfo {
        std::vector<Attachment> color_attachments;
        Attachment depth_attachment;
        Attachment stencil_attachment;
    };

    class RenderTarget : public Resource {
    public:
        RenderTarget(const RenderTargetCreateInfo& info) : _ci(info) {};

    protected:
        RenderTargetCreateInfo _ci;
    };


    struct RenderTargetClearInfo{
        glm::vec4 color;
        double depth;
        uint32_t stencil;
    };

    struct RenderPassParams {
        Viewport vp;
        RenderTargetClearInfo ci;
    };

    struct RenderPrimitive {
        std::shared_ptr<Buffer> vtx_buf;
        std::shared_ptr<Buffer> idx_buf;
    };

    struct DescriptorBindingInfo {
            uint32_t binding_idx;
            ShaderStageType shader_stage;
            DescriptorType desc_type;
            uint32_t desc_count;
    };

    struct DescriptorSetLayoutCreateInfo {
        std::vector<DescriptorBindingInfo> binding_infos;
    };

    class DescriptorSetLayout : public Resource {
        public:
        DescriptorSetLayout(const DescriptorSetLayoutCreateInfo& ci) : _ci(ci) {};

        protected:
        DescriptorSetLayoutCreateInfo _ci;
    };

    class DescriptorSet : public Resource {
        public:
        DescriptorSet(std::shared_ptr<DescriptorSetLayout> layout) {};
    };
};
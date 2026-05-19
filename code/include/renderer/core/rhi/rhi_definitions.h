#pragma once

namespace rhi{
    enum SamplerFilterType
    {
        SF_NEAREST,
        SF_LINEAR,
        SF_ANISOTROPIC_NEAREST,
        SF_ANISOTROPIC_LINEAR
    };

    enum SamplerAddressMode
    {
        SAM_REPEAT,
        SAM_MIRROR_REPEAT,
        SAM_CLAMP_EDGE,
        SAM_CLAMP_BORDER
    };

    enum SamplerCompareFunction
    {
	    SCF_NEVER,
	    SCF_LESS
    };

    enum ShaderModuleType {
        SMT_NONE,
        SMT_VERTEX,
        SMT_FRAGMENT
    };

    enum PrimitiveTopology
    {
        PT_POINT_LIST,
        PT_LINE_LIST,
        PT_LINE_STRIP,
        PT_TRIANGLE_LIST,
        PT_TRIANGLE_STRIP,
        PT_TRIANGLE_FAN,
    };

    enum PolygonMode
    {
        PM_FILL,
        PM_LINE,
        PM_POINT,
    };

    enum CullMode
    {
        CM_NONE,
        CM_FRONT,
        CM_BACK,
        CM_FRONT_AND_BACK,
    };

    enum FrontFace
    {
        FF_COUNTER_CLOCKWISE,
        FF_CLOCKWISE,
    };

    enum CompareOp
    {
        CO_NEVER,
        CO_LESS,
        CO_EQUAL,
        CO_LESS_OR_EQUAL,
        CO_GREATER,
        CO_NOT_EQUAL,
        CO_GREATER_OR_EQUAL,
        CO_ALWAYS,
    };

    enum BlendFactor
    {
        BF_ZERO,
        BF_ONE,
        BF_SRC_ALPHA,
        BF_ONE_MINUS_SRC_ALPHA,
        BF_DST_ALPHA,
        BF_ONE_MINUS_DST_ALPHA,
        BF_SRC_COLOR,
        BF_ONE_MINUS_SRC_COLOR,
        BF_DST_COLOR,
        BF_ONE_MINUS_DST_COLOR,
    };

    enum BlendOp
    {
        BO_ADD,
        BO_SUBTRACT,
        BO_REVERSE_SUBTRACT,
        BO_MIN,
        BO_MAX,
    };

    enum SampleCount
    {
        SC_COUNT_1 = 1,
        SC_COUNT_2 = 2,
        SC_COUNT_4 = 4,
        SC_COUNT_8 = 8,
    };

    enum VertexInputRate
    {
        VIR_VERTEX,
        VIR_INSTANCE,
    };

    enum AttachmentLoadOp {
        ALO_LOAD = 0,
        ALO_CLEAR = 1,
        ALO_DONT_CARE = 2
    };

    enum AttachmentStoreOp {
        ASO_STORE = 0,
        ASO_DONT_CARE = 1
    };

    enum AttachmentType{
        ATTACHMENT_TYPE_COLOR = 0,
        ATTACHMENT_TYPE_DEPTH = 1,
        ATTACHMENT_TYPE_RESLOVE = 2,
        ATTACHMENT_TYPE_INPUT = 3,
        ATTACHMENT_TYPE_PRESERVE = 4
    };
};
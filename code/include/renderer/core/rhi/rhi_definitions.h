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
};
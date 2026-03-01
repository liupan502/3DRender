#version 450
layout(binding = 60) uniform sampler2D source_sampler;
layout(binding = 61) uniform BloomUpSampleParams {
    vec4 resolution;
    float level;
} bloom_up_sample_params;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_color;

void main() {
    float lod = bloom_up_sample_params.level;
    const float radius = 1.0;
    vec4 d = vec4(bloom_up_sample_params.resolution.zw, -bloom_up_sample_params.resolution.zw) * radius;

    vec3 c0, c1;
    
    c0 = textureLod(source_sampler, uv + d.zw, lod).rgb;
    c0 += textureLod(source_sampler, uv + d.xw, lod).rgb;
    c0 += textureLod(source_sampler, uv + d.xy, lod).rgb;
    c0 += textureLod(source_sampler, uv + d.zy, lod).rgb;
    c0 += 4.0 * textureLod(source_sampler, uv, lod).rgb;

    c1 = textureLod(source_sampler, uv + vec2(d.z, 0.0), lod).rgb;
    c1 += textureLod(source_sampler, uv + vec2(0.0, d.w), lod).rgb;
    c1 += textureLod(source_sampler, uv + vec2(d.x, 0.0), lod).rgb;
    c1 += textureLod(source_sampler, uv + vec2(0.0, d.y), lod).rgb;

    out_color.rgb = (c0 + 2.0 * c1) * (1.0 / 16.0);
}
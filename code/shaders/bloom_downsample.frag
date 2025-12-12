layout(binding = 50) uniform sampler2D source_sampler;
layout(binding = 51) uniform BloomParams {
    float level;
    float threshold;
    float inv_highlight;
} bloom_params;

layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 out_color;

float max3(const vec3 v) {
    return max(v.x, max(v.y, v.z));
}

void threshold(inout vec3 color) {
    color = max(vec3(0.0), color - 1.0f);
    highp float f = max3(color);
    color *= (1.0 / (1.0 + f * bloom_params.inv_highlight));
}

vec3 box4x4(vec3 s0, vec3 s1, vec3 s2, vec3 s3) {
    return (s0 + s1 + s2 + s3) * 0.25;
}

vec3 box4x4_reinhard(vec3 s0, vec3 s1, vec3 s2, vec3 s3) {
    float w0 = 1.0 / (1.0 + max3(s0));
    float w1 = 1.0 / (1.0 + max3(s1));
    float w2 = 1.0 / (1.0 + max3(s2));
    float w3 = 1.0 / (1.0 + max3(s3));
    vec3 s = s0 * w0 + s1 * w1 + s2 * w2 + s3 * w3;
    float w = 1.0f / (w0 + w1 + w2 + w3);
    return s * w;
}

void main() {
    float lod = bloom_params.level;

    vec3 c = textureLod(source_sampler, uv, lod).rgb;
    
    vec3 lt = textureLodOffset(source_sampler, uv, lod, ivec2(-1, -1)).rgb;
    vec3 rt = textureLodOffset(source_sampler, uv, lod, ivec2(1, -1)).rgb;
    vec3 rb = textureLodOffset(source_sampler, uv, lod, ivec2(1, 1)).rgb;
    vec3 lb = textureLodOffset(source_sampler, uv, lod, ivec2(-1, 1)).rgb;

    vec3 lt2 = textureLodOffset(source_sampler, uv, lod, ivec2(-2, -2)).rgb;
    vec3 rt2 = textureLodOffset(source_sampler, uv, lod, ivec2(2, -2)).rgb;
    vec3 rb2 = textureLodOffset(source_sampler, uv, lod, ivec2(2, 2)).rgb;
    vec3 lb2 = textureLodOffset(source_sampler, uv, lod, ivec2(-2, 2)).rgb; 

    vec3 l = textureLodOffset(source_sampler, uv, lod, ivec2(-2, 0)).rgb;
    vec3 r = textureLodOffset(source_sampler, uv, lod, ivec2(2, 0)).rgb;
    vec3 b = textureLodOffset(source_sampler, uv, lod, ivec2(0, 2)).rgb;
    vec3 t = textureLodOffset(source_sampler, uv, lod, ivec2(0, -2)).rgb; 

    vec3 c0, c1;

    if (lod <= 0.5) {
        if (bloom_params.threshold > 0.0) {
            threshold(c);
            threshold(lt);
            threshold(rt);
            threshold(rb);
            threshold(lb);
            threshold(lt2);
            threshold(rt2);
            threshold(rb2);
            threshold(lb2);
            threshold(l);
            threshold(t);
            threshold(r);
            threshold(b);
        }

        c0  = box4x4_reinhard(lt, rt, rb, lb);
        c1  = box4x4_reinhard(c, l, t, lt2);
        c1 += box4x4_reinhard(c, r, t, rt2);
        c1 += box4x4_reinhard(c, r, b, rb2);
        c1 += box4x4_reinhard(c, l, b, lb2);
    }
    else {
        c0  = box4x4(lt, rt, rb, lb);
        c1  = box4x4(c, l, t, lt2);
        c1 += box4x4(c, r, t, rt2);
        c1 += box4x4(c, r, b, rb2);
        c1 += box4x4(c, l, b, lb2);
    }

    out_color.rgb = c0 * 0.5 + c1 * 0.125;
}



#define TEXTURE_SPACE_UP    -1
#define TEXTURE_SPACE_DN     1

#define USE_YCoCg
// #define FILTER_HISTORY

layout(binding = 0)  uniform sampler2D color_sampler;
layout(binding = 1)  uniform sampler2D depth_sampler;
layout(binding = 2)  uniform sampler2D history_sampler;

layout(binding = 3)  uniform TaaInfo{
    mat4 reproject_mat;
    float filter_weights[9];
    float alpha;
    // float 
} taa_info;

layout(binding = 4)  uniform sampler2D velocity_sampler;

layout (location = 0) out vec4 outColor;

layout(location = 0) in highp vec2 uv;

vec3 clipToBox(const vec3 boxmin,  const vec3 boxmax, const vec3 c, const vec3 h) {
    return clamp(h, boxmin, boxmax);
}

// Samples a texture with Catmull-Rom filtering, using 9 texture fetches instead of 16.
//      https://therealmjp.github.io/
// Some optimizations from here:
//      http://vec3.ca/bicubic-filtering-in-fewer-taps/ for more details
// Optimized to 5 taps by removing the corner samples
// And modified for mediump support
vec4 sampleTextureCatmullRom(const sampler2D tex, const highp vec2 uv, const highp vec2 texSize) {
    // We're going to sample a a 4x4 grid of texels surrounding the target UV coordinate. We'll do this by rounding
    // down the sample location to get the exact center of our "starting" texel. The starting texel will be at
    // location [1, 1] in the grid, where [0, 0] is the top left corner.

    highp vec2 samplePos = uv * texSize;
    highp vec2 texPos1 = floor(samplePos - 0.5) + 0.5;

    // Compute the fractional offset from our starting texel to our original sample location, which we'll
    // feed into the Catmull-Rom spline function to get our filter weights.
    highp vec2 f = samplePos - texPos1;
    highp vec2 f2 = f * f;
    highp vec2 f3 = f2 * f;

    // Compute the Catmull-Rom weights using the fractional offset that we calculated earlier.
    // These equations are pre-expanded based on our knowledge of where the texels will be located,
    // which lets us avoid having to evaluate a piece-wise function.
    vec2 w0 = f2 - 0.5 * (f3 + f);
    vec2 w1 = 1.5 * f3 - 2.5 * f2 + 1.0;
    vec2 w3 = 0.5 * (f3 - f2);
    vec2 w2 = 1.0 - w0 - w1 - w3;

    // Work out weighting factors and sampling offsets that will let us use bilinear filtering to
    // simultaneously evaluate the middle 2 samples from the 4x4 grid.
    vec2 w12 = w1 + w2;

    // Compute the final UV coordinates we'll use for sampling the texture
    highp vec2 texPos0 = texPos1 - vec2(1.0);
    highp vec2 texPos3 = texPos1 + vec2(2.0);
    highp vec2 texPos12 = texPos1 + w2 / w12;

    highp vec2 invTexSize = 1.0 / texSize;
    texPos0  *= invTexSize;
    texPos3  *= invTexSize;
    texPos12 *= invTexSize;

    float k0 = w12.x * w0.y;
    float k1 = w0.x  * w12.y;
    float k2 = w12.x * w12.y;
    float k3 = w3.x  * w12.y;
    float k4 = w12.x * w3.y;

    vec4 result =   textureLod(tex, vec2(texPos12.x, texPos0.y),  0.0) * k0
                  + textureLod(tex, vec2(texPos0.x,  texPos12.y), 0.0) * k1
                  + textureLod(tex, vec2(texPos12.x, texPos12.y), 0.0) * k2
                  + textureLod(tex, vec2(texPos3.x,  texPos12.y), 0.0) * k3
                  + textureLod(tex, vec2(texPos12.x, texPos3.y),  0.0) * k4;

    result *= 1.0 / (k0 + k1 + k2 + k3 + k4);

    return result;
}

float luminance(const vec3 linear) {
    #ifdef USE_YCoCg
        return linear.x;
    #else
        return dot(linear, vec3(0.2126, 0.7152, 0.0722));
    #endif
}

vec3 RGB_YCoCg(const vec3 c) {
    float Y  = dot(c.rgb, vec3( 1, 2,  1) * 0.25);
    float Co = dot(c.rgb, vec3( 2, 0, -2) * 0.25);
    float Cg = dot(c.rgb, vec3(-1, 2, -1) * 0.25);
    return vec3(Y, Co, Cg);
}

vec3 YCoCg_RGB(const vec3 c) {
    float Y  = c.x;
    float Co = c.y;
    float Cg = c.z;
    float r = Y + Co - Cg;
    float g = Y + Cg;
    float b = Y - Co - Cg;
    return vec3(r, g, b);
}


void main() {
    
    vec3 current = texture(color_sampler, uv).rgb;

    vec4 velocity = texture(velocity_sampler, uv);
    if (velocity.x > 999.0 && velocity.y > 999.0) {
        outColor.rgb = current;
        // outColor.rgb = vec3(1.0, 0.0, 1.0);
        outColor.a = 1.0f;
        return;
    }

    vec3 s[9];
    s[0] = textureLodOffset(color_sampler, uv, 0.0, ivec2(-1, TEXTURE_SPACE_DN)).rgb;
    s[1] = textureLodOffset(color_sampler, uv, 0.0, ivec2( 0, TEXTURE_SPACE_DN)).rgb;
    s[2] = textureLodOffset(color_sampler, uv, 0.0, ivec2( 1, TEXTURE_SPACE_DN)).rgb;
    s[3] = textureLodOffset(color_sampler, uv, 0.0, ivec2(-1, 0)).rgb;
    s[4] = current;
    s[5] = textureLodOffset(color_sampler, uv, 0.0, ivec2( 1, 0)).rgb;
    s[6] = textureLodOffset(color_sampler, uv, 0.0, ivec2(-1, TEXTURE_SPACE_UP)).rgb;
    s[7] = textureLodOffset(color_sampler, uv, 0.0, ivec2( 0, TEXTURE_SPACE_UP)).rgb;
    s[8] = textureLodOffset(color_sampler, uv, 0.0, ivec2( 1, TEXTURE_SPACE_UP)).rgb;

    #ifdef USE_YCoCg
        for (int i = 0 ; i < 9 ; i++) {
            s[i] = RGB_YCoCg(s[i]);
        }
        current = s[4];
    #endif

    vec3 filtered = vec3(0, 0, 0);
    for (int i = 0 ; i < 9 ; i++) {
         filtered += s[i] * taa_info.filter_weights[i];
    }

    // float depth = texture(depth_sampler, uv).r;

    // highp vec4 q = taa_info.reproject_mat * vec4(uv, depth, 1.0);
    // vec2 history_uv = (q.xy * (1.0 / q.w));
    // vec4 uv_offset = taa_info.reproject_mat * texture(velocity_sampler, uv)
    vec2 history_uv = uv + (taa_info.reproject_mat * velocity).xy;
    // history_uv = uv;
    #ifdef FILTER_HISTORY  
        vec3 history = sampleTextureCatmullRom(history_sampler, history_uv, vec2(textureSize(history_sampler, 0))).rgb;
    #else          
        vec3 history = texture(history_sampler, history_uv).rgb;
    #endif

    #ifdef USE_YCoCg
        history = RGB_YCoCg(history);
    #endif

    vec3 boxmin = min(s[4], min(min(s[1], s[3]), min(s[5], s[7])));
    vec3 boxmax = max(s[4], max(max(s[1], s[3]), max(s[5], s[7])));
    vec3 box9min = min(boxmin, min(min(s[0], s[2]), min(s[6], s[8])));
    vec3 box9max = max(boxmax, max(max(s[0], s[2]), max(s[6], s[8])));
    // round the corners of the 3x3 box
    boxmin = (boxmin + box9min) * 0.5;
    boxmax = (boxmax + box9max) * 0.5;

    history = clipToBox(boxmin, boxmax, filtered, history);

    float lumaColor   = luminance(filtered.rgb);
    float lumaHistory = luminance(history.rgb);

    // tonemapping for handling HDR
    filtered *= 1.0 / (1.0 + lumaColor);
    history  *= 1.0 / (1.0 + lumaHistory);

    float alpha = taa_info.alpha;
    vec3 ret = mix(history, filtered, alpha);

    ret *= 1.0 / (1.0 - luminance(ret));

    #ifdef USE_YCoCg
        ret = YCoCg_RGB(ret);
    #endif

    outColor.rgb = ret;
    outColor.a = 1.0f;
}

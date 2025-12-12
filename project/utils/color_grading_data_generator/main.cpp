#include <iostream>
#include "glm/glm.hpp"
#include <stdio.h>
#include <math.h>
using namespace std;

// RGB to luminance coefficients for ACEScg (AP1), from AP1_to_XYZ
glm::vec3 LUMINANCE_AP1{0.272229f, 0.674082f, 0.0536895f};

glm::mat3 AP0_to_AP1{
     1.4514393161f, -0.0765537734f,  0.0083161484f,
    -0.2365107469f,  1.1762296998f, -0.0060324498f,
    -0.2149285693f, -0.0996759264f,  0.9977163014f
};

glm::mat3 AP1_to_AP0{
     0.6954522414f,  0.0447945634f, -0.0055258826f,
     0.1406786965f,  0.8596711185f,  0.0040252103f,
     0.1638690622f,  0.0955343182f,  1.0015006723f
};

glm::mat3 XYZ_to_AP1{
     1.6410233797f, -0.6636628587f,  0.0117218943f,
    -0.3248032942f,  1.6153315917f, -0.0082844420f,
    -0.2364246952f,  0.0167563477f,  0.9883948585f
};

glm::mat3 Rec2020_to_XYZ{
     0.6369530f,  0.2626983f,  0.0000000f,
     0.1446169f,  0.6780088f,  0.0280731f,
     0.1688558f,  0.0592929f,  1.0608272f
};

glm::mat3 Rec2020_to_AP0 = AP1_to_AP0 * XYZ_to_AP1 * Rec2020_to_XYZ;

glm::mat3 XYZ_to_Rec2020{
    1.7166634f,  -0.6666738f,  0.0176425f,
    -0.3556733f,  1.6164557f, -0.0427770f,
    -0.2533681f,  0.0157683f,  0.9422433f
};

glm::mat3 AP1_to_XYZ{
     0.6624541811f,  0.2722287168f, -0.0055746495f,
     0.1340042065f,  0.6740817658f,  0.0040607335f,
     0.1561876870f,  0.0536895174f,  1.0103391003f
};

glm::mat3 AP1_to_Rec2020 = XYZ_to_Rec2020 * AP1_to_XYZ;

void LogC_to_linear(glm::vec3& x) {
    const float ia = 1.0f / 5.555556f;
    const float b  = 0.047996f;
    const float ic = 1.0f / 0.244161f;
    const float d  = 0.386036f;
    x = (x - d) * ic;
    x[0] = pow(10.0f, x[0]);
    x[1] = pow(10.0f, x[1]);
    x[2] = pow(10.0f, x[2]);
    x = (x - b) * ia;
}

void max(glm::vec3& arr, float val) {
    for (uint16_t i = 0; i < 3; i++) {
        float& x = arr[i]; 
        x = x > val ? x : val;
    }
}

float rgb_2_saturation(const glm::vec3& rgb) {
    // Input:  ACES
    // Output: OCES
    constexpr float TINY = 1e-5f;
    
    float mi = glm::min(rgb[0], glm::min(rgb[1], rgb[2]));
    float ma = glm::max(rgb[0], glm::max(rgb[1], rgb[2]));
    return (max(ma, TINY) - max(mi, TINY)) / max(ma, 1e-2f);
}


float rgb_2_yc(const glm::vec3& rgb) {
    constexpr float ycRadiusWeight = 1.75f;

    // Converts RGB to a luminance proxy, here called YC
    // YC is ~ Y + K * Chroma
    // Constant YC is a cone-shaped surface in RGB space, with the tip on the
    // neutral axis, towards white.
    // YC is normalized: RGB 1 1 1 maps to YC = 1
    //
    // ycRadiusWeight defaults to 1.75, although can be overridden in function
    // call to rgb_2_yc
    // ycRadiusWeight = 1 -> YC for pure cyan, magenta, yellow == YC for neutral
    // of same value
    // ycRadiusWeight = 2 -> YC for pure red, green, blue  == YC for  neutral of
    // same value.

    float r = rgb.r;
    float g = rgb.g;
    float b = rgb.b;

    float chroma = std::sqrt(b * (b - g) + g * (g - r) + r * (r - b));

    return (b + g + r + ycRadiusWeight * chroma) / 3.0f;
}

float sign(float x) {
    return x < 0.0 ? -1.0 : 1.0;
}

float sigmoid_shaper(float x) {
    // Sigmoid function in the range 0 to 1 spanning -2 to +2.
    float t = max(1.0f - std::abs(x / 2.0f), 0.0f);
    float y = 1.0f + sign(x) * (1.0f - t * t);
    return y / 2.0f;
}

float glow_fwd(float ycIn, float glowGainIn, float glowMid) {
    float glowGainOut;

    if (ycIn <= 2.0f / 3.0f * glowMid) {
        glowGainOut = glowGainIn;
    } else if ( ycIn >= 2.0f * glowMid) {
        glowGainOut = 0.0f;
    } else {
        glowGainOut = glowGainIn * (glowMid / ycIn - 1.0f / 2.0f);
    }

    return glowGainOut;
}

float rgb_2_hue(const glm::vec3& rgb) {
    // Returns a geometric hue angle in degrees (0-360) based on RGB values.
    // For neutral colors, hue is undefined and the function will return a quiet NaN value.
    float hue = 0.0f;
    // RGB triplets where RGB are equal have an undefined hue
    double F_PI  = 3.14159265358979323846264338327950288;
    float rad_to_deg = 180.0 / F_PI;
    if (!(rgb.x == rgb.y && rgb.y == rgb.z)) {
        hue = rad_to_deg * std::atan2(
                std::sqrt(3.0f) * (rgb.y - rgb.z),
                2.0f * rgb.x - rgb.y - rgb.z);
    }
    return (hue < 0.0f) ? hue + 360.0f : hue;
}

float center_hue(float hue, float centerH) {
    float hueCentered = hue - centerH;
    if (hueCentered < -180.0f) {
        hueCentered = hueCentered + 360.0f;
    } else if (hueCentered > 180.0f) {
        hueCentered = hueCentered - 360.0f;
    }
    return hueCentered;
}

float clamp(float v, float min, float max) noexcept {
    assert(min <= max);
    return float(std::min(max, std::max(min, v)));
}

float smoothstep(float e0, float e1, float x) noexcept {
    float t = clamp((x - e0) / (e1 - e0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

glm::vec3 XYZ_to_xyY(const glm::vec3& v) noexcept {
    float scale = 1.0f / max(v.x + v.y + v.z, 1e-5f);
    return {v.x * scale, v.y * scale, v.y};
}

glm::vec3 xyY_to_XYZ(const glm::vec3& v) noexcept {
    const float a = v.z / max(v.y, 1e-5f);
    return glm::vec3{v.x * a, v.z, (1.0f - v.x - v.y) * a};
}

glm::vec3 darkSurround_to_dimSurround(const glm::vec3& linearCV) {
    constexpr float DIM_SURROUND_GAMMA = 0.9811f;

    glm::vec3 XYZ = AP1_to_XYZ * linearCV;
    glm::vec3 xyY = XYZ_to_xyY(XYZ);

    // xyY.z = clamp(xyY.z, 0.0f, (float) std::numeric_limits<half>::max());
    xyY.z = max(xyY.z, 0.0f);//, (float) std::numeric_limits<half>::max());
    xyY.z = std::pow(xyY.z, DIM_SURROUND_GAMMA);

    XYZ = xyY_to_XYZ(xyY);
    return XYZ_to_AP1 * XYZ;
}

void aces(glm::vec3& color, float brightness) {
      // Some bits were removed to adapt to our desired output

    // "Glow" module constants
    constexpr float RRT_GLOW_GAIN = 0.05f;
    constexpr float RRT_GLOW_MID = 0.08f;

    // Red modifier constants
    constexpr float RRT_RED_SCALE = 0.82f;
    constexpr float RRT_RED_PIVOT = 0.03f;
    constexpr float RRT_RED_HUE   = 0.0f;
    constexpr float RRT_RED_WIDTH = 135.0f;

    // Desaturation constants
    constexpr float RRT_SAT_FACTOR = 0.96f;
    constexpr float ODT_SAT_FACTOR = 0.93f;

    glm::vec3 ap0 = Rec2020_to_AP0 * color;
    

    // Glow module
    float saturation = rgb_2_saturation(ap0);
    float ycIn = rgb_2_yc(ap0);
    float s = sigmoid_shaper((saturation - 0.4f) / 0.2f);
    float addedGlow = 1.0f + glow_fwd(ycIn, RRT_GLOW_GAIN * s, RRT_GLOW_MID);
    ap0 *= addedGlow;

    // Red modifier
    float hue = rgb_2_hue(ap0);
    float centeredHue = center_hue(hue, RRT_RED_HUE);
    float hueWeight = smoothstep(0.0f, 1.0f, 1.0f - std::abs(2.0f * centeredHue / RRT_RED_WIDTH));
    hueWeight *= hueWeight;

    ap0.r += hueWeight * saturation * (RRT_RED_PIVOT - ap0.r) * (1.0f - RRT_RED_SCALE);

    // ACES to RGB rendering space
    // glm::vec3 ap1 = clamp(AP0_to_AP1 * ap0, 0.0f, (float) std::numeric_limits<half>::max());
    glm::vec3 ap1 = AP0_to_AP1 * ap0;
    max(ap1, 0.0);

    // Global desaturation
    ap1 = glm::mix(glm::vec3(glm::dot(ap1, LUMINANCE_AP1)), ap1, RRT_SAT_FACTOR);

    // NOTE: This is specific to Filament and added only to match ACES to our legacy tone mapper
    //       which was a fit of ACES in Rec.709 but with a brightness boost.
    ap1 *= brightness;

    // Fitting of RRT + ODT (RGB monitor 100 nits dim) from:
    // https://github.com/colour-science/colour-unity/blob/master/Assets/Colour/Notebooks/CIECAM02_Unity.ipynb
    constexpr float a = 2.785085f;
    constexpr float b = 0.107772f;
    constexpr float c = 2.936045f;
    constexpr float d = 0.887122f;
    constexpr float e = 0.806889f;
    glm::vec3 rgbPost = (ap1 * (a * ap1 + b)) / (ap1 * (c * ap1 + d) + e);

    // Apply gamma adjustment to compensate for dim surround
    glm::vec3 linearCV = darkSurround_to_dimSurround(rgbPost);

    // Apply desaturation to compensate for luminance difference
    linearCV = glm::mix(glm::vec3(dot(linearCV, LUMINANCE_AP1)), linearCV, ODT_SAT_FACTOR);

    color = AP1_to_Rec2020 * linearCV;
}

void saturate(glm::vec3& lv) {
    lv[0] = clamp(lv[0], 0.0, 1.0);
    lv[1] = clamp(lv[1], 0.0, 1.0);
    lv[2] = clamp(lv[2], 0.0, 1.0);
}

void OETF_sRGB(glm::vec3& x) noexcept {
    constexpr float a  = 0.055f;
    constexpr float a1 = 1.055f;
    constexpr float b  = 12.92f;
    constexpr float p  = 1 / 2.4f;
    for (size_t i = 0; i < 3; i++) {
        x[i] = x[i] <= 0.0031308f ? x[i] * b : a1 * pow(x[i], p) - a;
    }
}

int main()
{
    float min_val = 100000.0f;
    float max_val = -1.0f;
    uint16_t dim_size = 32;
    const char* path = "/home/liup/Documents/color_grading.bin";
    uint8_t* buf = new uint8_t[dim_size * dim_size * dim_size * 4];
    for (uint16_t b = 0; b < dim_size; b++) {
        for (uint16_t g = 0; g < dim_size; g++) {
            for (uint16_t r = 0; r < dim_size; r++) {
                float scale = 1.0f / (dim_size - 1u);
                glm::vec3 vec(r * scale, g * scale, b * scale);
                

                LogC_to_linear(vec);

                max(vec, 0.0f);

                aces(vec, 1.0f);

                saturate(vec);

                OETF_sRGB(vec);

                for (uint16_t i = 0; i < 3; i++) {
                    if (vec[i] < min_val) {
                        min_val = vec[i];
                    }
                    if (vec[i] > max_val) {
                        max_val = vec[i];
                    }
                }

                uint32_t idx = (b * dim_size * dim_size + g * dim_size + r) * 4;
                uint16_t max = 255;
                buf[idx] = vec[0] * max;
                buf[idx + 1] = vec[1] * max;
                buf[idx + 2] = vec[2] * max;
                buf[idx + 3] = 0;
            }
        }
    }

    FILE* f = fopen(path, "wb");
    if (f) {
        fwrite(buf, sizeof(uint8_t),
               dim_size * dim_size * dim_size * 4, f);
    }
    fclose(f);
    return 0;
}

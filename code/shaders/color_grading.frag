layout (binding = 0) uniform sampler2D color;

layout (binding = 1) uniform sampler2D bloom;

layout (binding = 2) uniform sampler3D lut_sampler;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 out_color;

vec3 color_grade(sampler3D lut, const vec3 x) {
    // Alexa LogC EI 1000
    const float a = 5.555556;
    const float b = 0.047996;
    const float c = 0.244161 / log2(10.0);
    const float d = 0.386036;
    vec3 logc = c * log2(a * x + b) + d;

    // Remap to sample pixel centers
    // logc = materialParams.lutSize.x + logc * materialParams.lutSize.y;
    // lut size  = 32
    logc = (0.5f / 32.0f) + logc * (31.0f / 32.0f);
    
    return texture(lut, logc).rgb;
}
void main() {


    vec4 c = texture(color, uv);
    vec4 b = texture(bloom, uv);
    // vec4 b = vec4(0.0f);
    
    out_color.rgb = c.rgb + b.rgb;
    out_color.rgb = color_grade(lut_sampler, out_color.rgb);
    out_color.a = 1.0f;
}
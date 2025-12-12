// #version 450

// layout(input_attachment_index = 0, binding = 0) uniform subpassInput base_color;
layout(binding = 0)  uniform sampler2D tex_sampler;
layout(location = 0) in highp vec2 uv;
layout (location = 0) out vec4 outColor;
void main() {
    // outColor.rgb  = subpassLoad(base_color).rgb;
    outColor.rgb = texture(tex_sampler, uv).rgb;
    // outColor.rgb = vec3(1.0, 0.0, 0.0);
    outColor.a = 1.0f;
}
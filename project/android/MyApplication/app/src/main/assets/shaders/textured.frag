
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable



#define MAX_DIRECTIONAL_LIGHT_COUNT 5
#define MAX_SPOT_LIGHT_COUNT 3
#define MAX_POINT_LIGHT_COUNT 3

#ifndef DIRECTIONAL_LIGHT_COUNT
#define DIRECTIONAL_LIGHT_COUNT 0
#endif

#ifndef SPOT_LIGHT_COUNT
#define SPOT_LIGHT_COUNT 0
#endif

#ifndef POINT_LIGHT_COUNT
#define POINT_LIGHT_COUNT 0
#endif

#if (DIRECTIONAL_LIGHT_COUNT > 0) || (POINT_LIGHT_COUNT > 0) || (SPOT_LIGHT_COUNT > 0)
#define LIGHTING
#endif

layout(location = 0) in vec2 frag_tex_coord;
layout(binding = 10) uniform sampler2D tex_sampler;



#ifdef LIGHTING
layout(location = 1) smooth in vec3 normal;
layout(location = 2) in vec3 camera_pos;
layout (location = 5) in vec3 vtx_pos;
#endif

layout(location = 6) in float obj_visibility;
layout (location = 0) out vec4 uFragColor;

vec4 base_color;
vec3 camera_direction;

#ifdef LIGHTING
#include "pbr_light.frag"
#endif

layout(binding = 18) uniform FragmentUniformBuffer {
    vec4 emissive_color;
} frag_uni_buf;

vec4 add_emissive(vec4 color) {
   vec4 emissive_color = frag_uni_buf.emissive_color;
   // vec4 emissive_color = vec4(0.0, 1.0, 0.0, 3.0);
   vec3 tmp = emissive_color.rgb * (color.a * emissive_color.w);
   // tmp = vec3(0, 0, 2);
   color.rgb = color.rgb + tmp;
   return color;
}

void main() {
   uFragColor = texture(tex_sampler, frag_tex_coord);
   base_color = uFragColor;
   
#ifdef LIGHTING
   camera_direction = camera_pos - vtx_pos;
   uFragColor.rgb = get_light_pixel();
#endif

   uFragColor = add_emissive(uFragColor);
   uFragColor.a = uFragColor.a * obj_visibility;
}
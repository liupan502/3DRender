
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

layout (location = 0) in vec4 a_position;
layout (location = 1) in vec2 tex_coord;

#ifdef LIGHTING
layout (location = 2) in vec3 a_normal;
#endif

#if defined(SKINNING)
layout (location = 3) in vec4 a_blend_weight;
layout (location = 4) in ivec4 a_blend_indices;
#endif



layout (binding = 0) uniform UniformBufferObject {
    mat4 world_view_projection_mat;
    mat4 inverse_transpose_world_view_mat;
    mat4 world_view_mat;
    mat4 inverse_transpose_world_mat;
    mat4 world_mat;
    vec4 params;
} ubo;

#ifdef LIGHTING
layout(binding = 1) uniform LightVertexInfo {

    vec4 point_light_positions[MAX_POINT_LIGHT_COUNT];

    vec4 spot_light_positions[MAX_SPOT_LIGHT_COUNT];

	vec4 camera_position;

} light_vertex_info;
#endif

#if defined(SKINNING)
#define MAX_JOINT_NUM 20
layout (binding = 2) uniform UniformMatrixPalettes{
    mat4 joint_matrices[MAX_JOINT_NUM];
} u_matrix_palettes;
#endif

layout(location = 0) out vec2 frag_tex_coord;
#ifdef LIGHTING
layout (location = 1) smooth out vec3 normal;

layout (location = 2) out vec3 camera_pos;
layout (location = 5) out vec3 vtx_pos;


#if (POINT_LIGHT_COUNT > 0)
layout(location = 3) out vec3 v_vertex_to_point_light_directions[POINT_LIGHT_COUNT];
#endif

#if (SPOT_LIGHT_COUNT > 0)
layout(location = 4) out vec3 v_vertex_to_spot_light_directions[SPOT_LIGHT_COUNT];
#endif

#include "pbr_light.vert"

#endif

layout(location = 6) out float visibility;

#ifndef SKINNING
#include "skinning_none.vert"
#else
#include "skinning.vert"
#endif

void main() {
   vec4 pos = get_pos();
   gl_Position = ubo.world_view_projection_mat * pos;
   // gl_Position = a_position;
   frag_tex_coord = tex_coord;
   visibility = ubo.params.x;
   #ifdef LIGHTING
   vec3 tmp_normal = get_normal();
   mat3 inverse_transpose_world_mat = mat3(
   ubo.inverse_transpose_world_mat[0].xyz,
   ubo.inverse_transpose_world_mat[1].xyz,
   ubo.inverse_transpose_world_mat[2].xyz
   ) ;
   normal = normalize(inverse_transpose_world_mat * tmp_normal);
   // normal = tmp_normal;

   apply_light(pos);
   #endif
}


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

#ifdef VERTEX_COLOR
layout (location = 5) in vec4 a_color;
#endif


#define MAX_UNIFORM_BUFFER_OBJECT_COUNT 1

layout (binding = 0) uniform UniformBufferObject {
    mat4 world_view_projection_mat;
    mat4 inverse_transpose_world_view_mat;
    mat4 world_view_mat;
    mat4 inverse_transpose_world_mat;
    mat4 world_mat;
    vec4 params;
} ubo_arr[MAX_UNIFORM_BUFFER_OBJECT_COUNT];

#ifdef LIGHTING
layout(binding = 1) uniform LightVertexInfo {

    vec4 point_light_positions[MAX_POINT_LIGHT_COUNT];

    vec4 spot_light_positions[MAX_SPOT_LIGHT_COUNT];

	vec4 camera_position;

} light_vertex_info;
#endif

#if defined(SKINNING)
#define MAX_JOINT_NUM 100
layout (binding = 2) uniform UniformMatrixPalettes{
    mat4 joint_matrices[MAX_JOINT_NUM];
    mat4 pre_joint_matrices[MAX_JOINT_NUM];
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

layout(location = 7) out vec3 current_pdc_pos;
layout(location = 8) out vec3 pre_pdc_pos;

#ifdef VERTEX_COLOR
layout(location = 9) out vec4 vertex_color;
#endif


#ifndef SKINNING
#include "skinning_none.vert"
#else
#include "skinning.vert"
#endif

layout (binding = 25) uniform VelocityBufferObject {
    mat4 world_view_projection_mat;
    mat4 pre_world_view_projection_mat;
    // mat4 joint_matrices[MAX_JOINT_NUM];
} velocity_ubo_arr[MAX_UNIFORM_BUFFER_OBJECT_COUNT];

void compute_velocity(vec4 pos, vec4 pre_pos) {
    mat4 pre_mat = velocity_ubo_arr[gl_InstanceIndex].pre_world_view_projection_mat;
    mat4 cur_mat = velocity_ubo_arr[gl_InstanceIndex].world_view_projection_mat;
    vec4 current_proj_pos = cur_mat * pos;
    vec4 pre_proj_pos = pre_mat * pre_pos;
    pre_pdc_pos = pre_proj_pos.xyz * (1.0 / pre_proj_pos.w);
    current_pdc_pos = current_proj_pos.xyz * (1.0 / current_proj_pos.w);
}

void main() {
   vec4 pos = get_pos();
   vec4 pre_pos = get_pre_pos();
   compute_velocity(pos, pre_pos);
   gl_Position = ubo_arr[gl_InstanceIndex].world_view_projection_mat * pos;
   
   frag_tex_coord = tex_coord;
   visibility = ubo_arr[gl_InstanceIndex].params.x;
   #ifdef LIGHTING
   vec3 tmp_normal = get_normal();
   mat3 inverse_transpose_world_mat = mat3(
   ubo_arr[gl_InstanceIndex].inverse_transpose_world_mat[0].xyz,
   ubo_arr[gl_InstanceIndex].inverse_transpose_world_mat[1].xyz,
   ubo_arr[gl_InstanceIndex].inverse_transpose_world_mat[2].xyz
   ) ;
   normal = normalize(inverse_transpose_world_mat * tmp_normal);
   apply_light(pos);
   #endif

   #ifdef VERTEX_COLOR
   vertex_color = a_color;
   #endif 
}

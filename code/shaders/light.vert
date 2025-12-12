#ifndef SKINNING
vec3 get_normal() {
    return a_normal;
}
#endif


void apply_light(vec4 pos) {
    #if defined(SPECULAR) || (POINT_LIGHT_COUNT > 0) || (SPOT_LIGHT_COUNT > 0)
    vec4 position_world_view_space = ubo.world_view_mat * pos;
    #endif

    #if (POINT_LIGHT_COUNT > 0)
    for (int i = 0; i < POINT_LIGHT_COUNT; i++) {
        v_vertex_to_point_light_directions[i] = light_vertex_info.point_light_positions[i] - position_world_view_space.xyz;
    }
    #endif

    #if (SPOT_LIGHT_COUNT > 0)
    for (int i = 0; i < SPOT_LIGHT_COUNT; i++) {
        v_vertex_to_spot_light_directions[i] = light_vertex_info.spot_light_positions[i] - position_world_view_space.xyz;
    }
    #endif

    #ifdef SPECULAR
    camera_direction = light_vertex_info.camera_position.xyz - position_world_view_space.xyz;
    #endif
}

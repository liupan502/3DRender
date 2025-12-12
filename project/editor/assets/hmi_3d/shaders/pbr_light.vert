

void apply_light(vec4 pos) {
    // vec4 position_world_view_space = ubo.world_view_mat * pos;

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

    camera_pos = light_vertex_info.camera_position.xyz;
    vtx_pos = (ubo_arr[gl_InstanceIndex].world_mat * pos).xyz;
    // vtx_pos = pos.xyz;
   
}
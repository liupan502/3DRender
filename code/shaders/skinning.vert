vec4 _skinned_pos;

void skin_pos(mat4 mat, float blend_weight) {
    // mat4 mat = u_matrix_palettes.joint_matrices[mat_idx];
    _skinned_pos.xyz += (blend_weight * mat * a_position).xyz;
}

vec4 get_pos() {
    _skinned_pos = vec4(0.0, 0.0, 0.0, 1.0);
    mat4 mat_x = u_matrix_palettes.joint_matrices[a_blend_indices.x];
    skin_pos(mat_x, a_blend_weight.x);
    mat4 mat_y = u_matrix_palettes.joint_matrices[a_blend_indices.y];
    skin_pos(mat_y, a_blend_weight.y);
    mat4 mat_z = u_matrix_palettes.joint_matrices[a_blend_indices.z];
    skin_pos(mat_z, a_blend_weight.z);
    mat4 mat_w = u_matrix_palettes.joint_matrices[a_blend_indices.w];
    skin_pos(mat_w, a_blend_weight.w);
    return _skinned_pos;
}
vec4 get_pre_pos() {
    _skinned_pos = vec4(0.0, 0.0, 0.0, 1.0);
    mat4 mat_x = u_matrix_palettes.pre_joint_matrices[a_blend_indices.x];
    skin_pos(mat_x, a_blend_weight.x);
    mat4 mat_y = u_matrix_palettes.pre_joint_matrices[a_blend_indices.y];
    skin_pos(mat_y, a_blend_weight.y);
    mat4 mat_z = u_matrix_palettes.pre_joint_matrices[a_blend_indices.z];
    skin_pos(mat_z, a_blend_weight.z);
    mat4 mat_w = u_matrix_palettes.pre_joint_matrices[a_blend_indices.w];
    skin_pos(mat_w, a_blend_weight.w);
    return _skinned_pos;
}


#if defined(LIGHTING)
vec3 _skinned_normal;

void skin_tangent_space_vector(vec3 vector, int idx) {
    int mat_idx = a_blend_indices[idx];
    mat4 mat = u_matrix_palettes.joint_matrices[mat_idx];
    float blend_weight = a_blend_weight[idx];
    vec4 tmp = vec4(vector.x, vector.y, vector.z, 0.0);
    tmp = blend_weight * mat * tmp;
    _skinned_normal += tmp.xyz;
}

vec3 get_tangent_space_vector(vec3 vector) {
    _skinned_normal = vec3(0.0);
    skin_tangent_space_vector(vector, 0);
    skin_tangent_space_vector(vector, 1);
    skin_tangent_space_vector(vector, 2);
    skin_tangent_space_vector(vector, 3);
    return _skinned_normal;
}

vec3 get_normal() {
    return get_tangent_space_vector(a_normal);
}

#if defined(BUMPED)
vec3 get_tangent() {
    return get_tangent_space_vector(a_tangent);
}
vec3 get_binormal() {
    return get_tangent_space_vector(a_binormal);
}
#endif
#endif
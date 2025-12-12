vec4 get_pos() {
    return a_position;
}

vec4 get_pre_pos() {
    return a_position;
}

#if defined(LIGHTING)
vec3 get_normal() {
    return a_normal;
}
#endif
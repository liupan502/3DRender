#if (POINT_LIGHT_COUNT > 0)
layout(binding = 11)uniform PointLightInfo{
    vec3 color;
    float range_inverse;
} point_light_infos[POINT_LIGHT_COUNT];

layout(location = 3) in vec3 v_vertex_to_point_light_directions[POINT_LIGHT_COUNT];
#endif

#if (SPOT_LIGHT_COUNT > 0)
layout(binding = 12)uniform SpotLightInfo{
    vec3  color;
    vec3 direction;
    float range_inverse;
    float inner_angle_cos;
    float outer_angle_cos;
} spot_light_infos[SPOT_LIGHT_COUNT];

layout(location = 4) in vec3 v_vertex_to_spot_light_directions[SPOT_LIGHT_COUNT];
#endif

#if (DIRECTIONAL_LIGHT_COUNT > 0)

layout(binding = 13) uniform DirectionalLightInfo{
    vec3 color;
    vec3 direction;
} directional_light_infos[DIRECTIONAL_LIGHT_COUNT];

#endif

layout (binding = 15) uniform LightGlobalInfo{
    float specular_exponent;
    vec3 ambient_light_color;
} global_info;

vec3 compute_light_color(vec3 normal_vec, vec3 light_direction, vec3 light_color, float attenuation) {
    float diffuse = max(dot(normal_vec, light_direction), 0.0);
    vec3 diffuse_color = light_color * (base_color.rgb) * diffuse * attenuation;
    #ifdef SPECULAR
    // phong shading
    // vec3 vertex_to_eye = normalize(camera_direction);
    // vec3 specular_angle = normalize(normal_vec * diffuse * 2.0 - light_direction);
    // vec3 specular_color = vec3(pow(clamp(dot(specular_angle, vertex_to_eye), 0.0, 1.0), global_info.specular_exponent));

    // blinn-phong shading
    vec3 vertex_to_eye = normalize(camera_direction);
    vec3 half_vec = normalize(light_direction + vertex_to_eye);
    float specular_angle = clamp(dot(normal_vec, half_vec), 0.0, 1.0);
    vec3 specular_color = vec3(pow(specular_angle, global_info.specular_exponent)) * attenuation * 0.5;
    return diffuse_color + specular_color;
    #else
    return diffuse_color;
    #endif

}

vec3 get_light_pixel() {
    vec3 normal_vec = normalize(normal);
    vec3 ambient_color = base_color.rgb * global_info.ambient_light_color;
    vec3 combined_color = ambient_color;

    #if (DIRECTIONAL_LIGHT_COUNT > 0)
    for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++) {
        vec3 light_direction = normalize(directional_light_infos[i].direction);
        combined_color += compute_light_color(normal_vec, light_direction, directional_light_infos[i].color, 1.0);
    }
    #endif

    #if (POINT_LIGHT_COUNT > 0)
    for(int i = 0; i < POINT_LIGHT_COUNT; i++) {
        vec3 ldir = v_vertex_to_point_light_directions[i] * point_light_infos[i].range_inverse;
        float attenuation = clamp(1.0 - dot(ldir, ldir), 0.0, 1.0);
        combined_color += compute_light_color(normal_vec, normalize(v_vertex_to_point_light_directions[i]), point_light_infos[i].color, attenuation);
    }
    #endif

    #if (SPOT_LIGHT_COUNT > 0)
    for (int i = 0; i < SPOT_LIGHT_COUNT; i++) {
        vec3 ldir = v_vertex_to_spot_light_directions[i] * spot_light_infos[i].range_inverse;
        float attenuation = clamp(1.0 - dot(ldir, ldir), 0.0, 1.0);
        vec3 vertex_to_spot_light_direction = normalize(v_vertex_to_spot_light_directions[i]);
        vec3 spot_light_direction = normalize(spot_light_infos[i].direction * 2.0);

        float current_angle_cos = dot(spot_light_direction, -vertex_to_spot_light_direction);
        attenuation *= smoothstep(spot_light_infos[i].outer_angle_cos, spot_light_infos[i].inner_angle_cos, current_angle_cos);
        combined_color += compute_light_color(normal_vec, vertex_to_spot_light_direction, spot_light_infos[i].color, attenuation);
    }
    #endif

    return combined_color;
}
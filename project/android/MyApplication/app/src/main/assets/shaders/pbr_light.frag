#include "brdf.frag"
#ifdef ENVIRONMENT_LIGHT
#include "light_indirect.frag"        
#endif

#if (POINT_LIGHT_COUNT > 0)
layout(binding = 11)uniform PointLightInfo{
    vec4 color;
    float range_inverse;
} point_light_infos[POINT_LIGHT_COUNT];

layout(location = 3) in vec3 v_vertex_to_point_light_directions[POINT_LIGHT_COUNT];
#endif

#if (SPOT_LIGHT_COUNT > 0)
layout(binding = 12)uniform SpotLightInfo{
    vec4  color;
    vec3 direction;
    float range_inverse;
    float inner_angle_cos;
    float outer_angle_cos;
} spot_light_infos[SPOT_LIGHT_COUNT];

layout(location = 4) in vec3 v_vertex_to_spot_light_directions[SPOT_LIGHT_COUNT];
#endif

#if (DIRECTIONAL_LIGHT_COUNT > 0)

layout(binding = 13) uniform DirectionalLightInfo{
    vec4 color_intensity;
    vec3 direction;
} directional_light_infos[DIRECTIONAL_LIGHT_COUNT];

#endif


layout (binding = 14) uniform PbrPixelParams {
    float roughness;
    float ior;
    float metallic;
    float clear_coat;
    float clear_coat_roughness;
} pbr_pixel_params;

float ior_to_reflectence(float transmitted_ior, float incident_ior) {
    float val = (transmitted_ior - incident_ior) / (transmitted_ior + incident_ior);
    return val * val;
}

vec3 compute_f0(float metallic, float reflectance) {
    return base_color.rgb * metallic + (reflectance * (1.0 - metallic));
}

vec3 isotropic_specular_lobe(vec4 light_color_intensity, vec3 light_direction, float attenuation, vec3 f0, vec3 h, float NoV, float NoL, float NoH, float LoH) {
    float d = distribution(pbr_pixel_params.roughness, NoH, h);
    float v = visibility(pbr_pixel_params.roughness, NoV, NoL);

    vec3 f = fresnel(f0, LoH);
    return f;
}

vec3 diffuse_lobe(float metallic) {
    return base_color.rgb * (1.0 - metallic) * fd_lambert();
}

float clear_coat_lobe(const vec3 h, float NoH, float LoH, out float Fcc) {
    float clearCoatNoH = NoH;
    float D = distribution(pbr_pixel_params.clear_coat_roughness, clearCoatNoH, h);
    float V = visibility_clear_coat(LoH);
    float F = fresnel(0.04, 1.0, LoH) * pbr_pixel_params.clear_coat; // fix IOR to 1.5

    Fcc = F;
    return D*V*F;
}


vec3 compute_color(vec3 normal_vec, vec3 light_direction, vec4 light_color_intensity, float attenuation,  
            vec3 f0, vec3 v, float NoV) {
    float NoL = dot(normal_vec, light_direction);        
    if (NoL <= 0.0001) {
        return vec3(0.0, 0.0, 0.0);
    }

    vec3 h = normalize(light_direction + v);
    float NoH = dot(normal_vec, h);
    float LoH = dot(light_direction, h);
    if (LoH <= 0.0001) {
        return vec3(0.0, 0.0, 1.0);
    }

    vec3 specular = isotropic_specular_lobe(light_color_intensity, 
                light_direction, attenuation, f0, h, NoV, NoL, NoH, LoH);
    vec3 diffuse = diffuse_lobe(pbr_pixel_params.metallic);   

    vec3 color = specular + diffuse;
   
    if (pbr_pixel_params.clear_coat > 0.01) {
        float fcc = 0.0f;
        LoH = 0.5;
        float clearCoat = clear_coat_lobe(h, NoH, LoH, fcc);
        float attenuation = 1.0 - fcc;
        color = color * attenuation;
        color = color + clearCoat;
    }

    vec3 ret = (color * light_color_intensity.rgb) * (light_color_intensity.w * attenuation * NoL);
    
    return ret;         
}


vec3 get_light_pixel() {
    // vec3 normal_vec = normalize(normal);
    vec3 normal_vec = normal;
    vec3 combined_color = vec3(0.0, 0.0, 0.0);
    vec3 v = normalize(camera_direction);
    float NoV = dot(normal_vec, v);

    float reflectance = ior_to_reflectence(max(1.0, pbr_pixel_params.ior), 1.0);
    vec3 f0 = compute_f0(pbr_pixel_params.metallic, reflectance);

    #ifdef ENVIRONMENT_LIGHT
     evaluate_ibl(normal_vec, v, f0,
            NoV , pbr_pixel_params.roughness, combined_color);  
    // return combined_color;      
    #endif

    #if (DIRECTIONAL_LIGHT_COUNT > 0)
    for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++) {
        vec3 light_direction = normalize(directional_light_infos[i].direction);
        combined_color += compute_color(normal_vec, light_direction, directional_light_infos[i].color_intensity, 1.0, f0, v, NoV);
    }
    #endif

    #if (POINT_LIGHT_COUNT > 0)
    for(int i = 0; i < POINT_LIGHT_COUNT; i++) {
        vec3 ldir = v_vertex_to_point_light_directions[i] * point_light_infos[i].range_inverse;
        float attenuation = clamp(1.0 - dot(ldir, ldir), 0.0, 1.0);
        combined_color += compute_color(normal_vec, normalize(v_vertex_to_point_light_directions[i]), point_light_infos[i].color, attenuation, f0, v, NoV);

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
        combined_color += compute_light_color(normal_vec, vertex_to_spot_light_direction, spot_light_infos[i].color, attenuation, f0, v, NoV);
    }
    #endif

    return combined_color;
}
#include "sky_atmosphere_common.frag"

layout(location = 0) in highp vec2 frag_uv;
layout (location = 0) out vec4 outColor;


vec3 integrate_transmittance(in vec3 world_pos, in vec3 world_dir, in vec3 sun_dir,
    in AtmosphereParameters aps) {
    float t_max = compute_tmax(aps, world_pos, world_dir);

    float sample_count = 40.0f;
    float dt = t_max / sample_count;

    // phase Functions
    float uniform_phase = 1.0 / (4.0 * PI);
    float cos_theta = dot(sun_dir, -world_dir);

    float mie_phase_value = hg_phase(aps.mie_phase_g, cos_theta);
    float rayleigh_phase_value = rayleigh_phase(cos_theta);

    vec3 optical_depth = vec3(0.0f);
    float t = 0.0f;
    float sample_segment = 0.3f;

    for (float s = 0.0f; s < sample_count; s+= 1.0f) {
        t = dt * (s + sample_segment);
        vec3 p = world_pos + t * world_dir;
        // 距离地面的高度
        float view_height = length(p) - aps.bottom_radius;

        float density_mie = exp(aps.mie_density_exp_scale * view_height);
        float density_ray = exp(aps.rayleigh_density_exp_scale * view_height);
        float density_0zo = aps.absorption_density0_linear_term * view_height + aps.absorption_density0_constant_term;
        if (view_height > aps.absorption_density0_layer_width) {
            density_0zo = aps.absorption_density0_linear_term * view_height + aps.absorption_density1_constant_term;
        }
        density_0zo = clamp(density_0zo, 0.0, 1.0);


        vec3 extinction_mie = density_mie * aps.mie_extinction;
        vec3 extinction_ray = density_ray * aps.rayleigh_scattering;
        vec3 extinction_0zo = density_0zo * aps.absorption_extinction;
        vec3 extinction = extinction_mie + extinction_ray + extinction_0zo;
        optical_depth = optical_depth + extinction * dt;
    }
    
    return optical_depth;
}

void main() {

    AtmosphereParameters aps = get_aps();

    float view_height;
    float view_zenith_cos_angle;
    uv_to_lut_transmittance_params(aps, frag_uv, view_height, view_zenith_cos_angle);

    vec3 world_pos = vec3(0.0, 0.0, view_height);
    vec3 world_dir = vec3(0.0, sqrt(1.0 - view_zenith_cos_angle * view_zenith_cos_angle), view_zenith_cos_angle);
    
    outColor = vec4(1.0f);
    // sun direction 应该属于uniform buffer ,后续补充
    vec3 sun_direction = sky_render_info.sun_direction.xyz;
    outColor.rgb = exp(-integrate_transmittance(world_pos, world_dir, sun_direction, aps));
}
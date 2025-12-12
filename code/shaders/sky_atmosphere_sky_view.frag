#include "sky_atmosphere_common.frag"

layout(location = 0) in highp vec2 frag_uv;
layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D transmittance_tex;

vec3 integrate_sky_irradiance(in vec3 world_pos, in vec3 world_dir, in vec3 sun_dir,
    in AtmosphereParameters aps) {
    float t_max = compute_tmax(aps, world_pos, world_dir);
    float sample_count = 30.0f;
    float dt = t_max / sample_count;


    float uniform_phase = 1.0 / (4.0 * PI);
    float cos_theta = dot(sun_dir, -world_dir);

    float mie_phase_value = hg_phase(aps.mie_phase_g, cos_theta);
    float rayleigh_phase_value = rayleigh_phase(cos_theta);


    vec3 L = vec3(0.0f);
    vec3 throughput = vec3(1.0f);
    float t = 0.0f;
    float sample_segment_t = 0.3f;
    for (float s = 0.0f; s < sample_count; s += 1.0f) {
        t = t_max * (s + sample_segment_t) / sample_count;
        vec3 p = world_pos + t * world_dir;
        MediumSampleRGB medium = sample_medium_rgb(p, aps);
        vec3 sample_optical_depth = medium.extinction * dt;
        vec3 sample_transmittance = exp(-sample_optical_depth);

        float p_height = length(p);
        vec3 up = p / p_height;
        float sun_zenith_cos_angle = dot(sun_dir, up);
        vec2 transmittance_uv;
        lut_transmittance_params_to_uv(aps, p_height, sun_zenith_cos_angle, transmittance_uv);
        vec3 transmittance_to_sun = texture(transmittance_tex, transmittance_uv).rgb;

        vec3 phase_times_scattering = medium.scattering_mie * mie_phase_value + medium.scattering_ray * rayleigh_phase_value;
    
        float t_earth = ray_sphere_intersect_nearest(p, sun_dir, vec3(0.0) + PLANET_RADIUS_OFFSET * up, aps.bottom_radius);
        float earth_shadow = t_earth >= 0.0f ? 0.0f : 1.0f;
        vec3 global_l = vec3(8.0f);
        vec3 S = global_l * (earth_shadow * transmittance_to_sun * phase_times_scattering);
        vec3 Sint = (S - S * sample_transmittance) / medium.extinction;
        L += throughput * Sint;
        throughput *= sample_transmittance;
    }

    return L;
}

void main() {
    // uniform buffer 提供
    AtmosphereParameters aps = get_aps();
    vec3 camera_pos = sky_render_info.camera_pos.xyz;
    vec3 input_sun_dir = sky_render_info.sun_direction.xyz;


    vec3 world_pos = camera_pos + vec3(0.0, 0.0, aps.bottom_radius);
    float view_height = length(world_pos);

    float view_zenith_cos_angle;
    float light_view_cos_angle;
    uv_to_sky_view_params(aps, view_zenith_cos_angle, light_view_cos_angle, view_height, frag_uv);

    vec3 up_vector = normalize(world_pos);
    float sun_zenith_cos_angle = dot(up_vector, input_sun_dir);
    vec3 sun_dir = normalize(vec3(sqrt(1.0f - sun_zenith_cos_angle * sun_zenith_cos_angle), 0.0f, sun_zenith_cos_angle));

    world_pos = vec3(0.0f, 0.0f, view_height);

    float view_zenith_sin_angle = sqrt(1.0 - view_zenith_cos_angle * view_zenith_cos_angle);
    float light_view_sin_angle = sqrt(1.0 - light_view_cos_angle * light_view_cos_angle);
    vec3 world_dir = vec3(view_zenith_sin_angle * light_view_cos_angle, view_zenith_sin_angle * light_view_sin_angle, view_zenith_cos_angle);

    // move_to_top_atmosphere when camera out atmosphere
    
    outColor = vec4(1.0f);
    outColor.rgb = integrate_sky_irradiance(world_pos, world_dir, sun_dir, aps);
}
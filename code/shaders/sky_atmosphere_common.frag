#define PI 3.1415926535897932384626433832795f
#define PLANET_RADIUS_OFFSET 0.01f

struct AtmosphereParameters {
    float bottom_radius;
    float top_radius;

    float rayleigh_density_exp_scale;
    vec3 rayleigh_scattering;

    float mie_density_exp_scale;
    vec3 mie_scattering;
    vec3 mie_extinction;
    vec3 mie_absorption;
    float mie_phase_g;

    float absorption_density0_layer_width;
    float absorption_density0_constant_term;
    float absorption_density0_linear_term;

    float absorption_density1_constant_term;
    float absorption_density1_linear_term;

    vec3 absorption_extinction;

    vec3 group_albedo;
};

layout(binding = 0) uniform SkyRenderInfo {
    vec4 rayleigh_info;
    vec4 mie_scattering;
    vec4 mie_extinction;
    vec4 mie_absorption;
    vec4 absorption_density0;
    vec4 absorption_density1;
    vec4 absorption_extinction;
    vec4 group_albedo;
    vec4 sun_direction;
    vec4 camera_pos;
    float bottom_radius;
    float top_radius;
    float mie_density_exp_scale;
    float mie_phase_g;
} sky_render_info;

AtmosphereParameters get_aps() {
    AtmosphereParameters aps;

    aps.bottom_radius = sky_render_info.bottom_radius;
    aps.top_radius = sky_render_info.top_radius;

    aps.rayleigh_density_exp_scale = sky_render_info.rayleigh_info.w;
    aps.rayleigh_scattering = sky_render_info.rayleigh_info.xyz;

    aps.mie_density_exp_scale = sky_render_info.mie_density_exp_scale;
    aps.mie_scattering = sky_render_info.mie_scattering.xyz;
    aps.mie_extinction = sky_render_info.mie_extinction.xyz;
    aps.mie_absorption = sky_render_info.mie_absorption.xyz;
    aps.mie_phase_g = sky_render_info.mie_phase_g;

    aps.absorption_density0_layer_width = sky_render_info.absorption_density0.z;
    aps.absorption_density0_constant_term = sky_render_info.absorption_density0.y;
    aps.absorption_density0_linear_term = sky_render_info.absorption_density0.x;

    aps.absorption_density1_constant_term = sky_render_info.absorption_density1.y;
    aps.absorption_density1_linear_term = sky_render_info.absorption_density1.x;
    
    aps.absorption_extinction = sky_render_info.absorption_extinction.xyz;

    aps.group_albedo = sky_render_info.group_albedo.xyz;

    return aps;
}

// float max(float v1, float v2) {
//     return v1 > v2 ? v1 : v2;
// }

// float min(float v1, float v2) {
//     return v1 > v2 ? v2 : v1;
// }

// - r0: ray origin
// - rd: normalized ray direction
// - s0: sphere center
// - sr: sphere radius
// - Returns distance from r0 to first intersecion with sphere,
//   or -1.0 if no intersection.
float ray_sphere_intersect_nearest(vec3 r0, vec3 rd, vec3 s0, float sr) {
    float a = dot(rd, rd);
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - sr * sr;
    float delta = b * b - 4.0 * a * c;

    if (delta < 0.0 || a == 0.0) {
        return -1.0;
    }

    float sol0 = (-b - sqrt(delta)) / (2.0 * a);
    float sol1 = (-b + sqrt(delta)) / (2.0 * a);

    if (sol0 < 0.0 && sol1 < 0.0) {
        return -1.0;
    }

    if (sol0 < 0.0) {
        return max(0.0, sol1);
    }
    else if (sol1 < 0.0) {
        return max(0.0, sol0);
    } 
    return max(0.0, min(sol0, sol1));
}

// cornette shanks mie phase function
float hg_phase(float g, float cos_theta) {
    float k = 3.0f / (8.0 * PI) * (1.0 - g * g) / (2.0 * g * g);
    return k * (1.0 + cos_theta * cos_theta) / pow(1.0 + g * g + 2.0 * g * cos_theta, 1.5);
}

float rayleigh_phase(float cos_theta) {
    float factor = 3.0f / (16.0f * PI);
    return factor * (1.0f + cos_theta * cos_theta) * 1.0f;
    // return factor * 2.0f;
    // return 0.1f;
}

void uv_to_lut_transmittance_params(AtmosphereParameters aps, in vec2 uv, out float view_height, out float view_zenith_cos_angle) {
    float x_mu = uv.x;
    float x_r = uv.y;

    float tr = aps.top_radius;
    float br = aps.bottom_radius;
    float H = sqrt(tr * tr - br * br);
    float rho = H * x_r;

    view_height = sqrt(rho * rho + br * br);

    float d_min = tr - view_height;
    float d_max = rho + H;
    float d = d_min + x_mu * (d_max - d_min);
    view_zenith_cos_angle = 1.0f;
    if (d != 0.0) {
        view_zenith_cos_angle = (H * H - rho * rho - d * d) / (2.0 * view_height * d);
    }
    view_zenith_cos_angle = clamp(view_zenith_cos_angle, -1.0, 1.0); 
}

void lut_transmittance_params_to_uv(AtmosphereParameters aps, in float view_height, in float view_zenith_cos_angle, out vec2 output_uv) {
    float H = sqrt(max(0.0f, aps.top_radius * aps.top_radius - aps.bottom_radius * aps.bottom_radius));
    float rho = sqrt(max(0.0f, view_height * view_height - aps.bottom_radius * aps.bottom_radius));

    float discriminant = view_height * view_height * (view_zenith_cos_angle * view_zenith_cos_angle - 1.0f) + aps.top_radius * aps.top_radius;
    float d = max(0.0, (-view_height * view_zenith_cos_angle + sqrt(discriminant)));

    float d_min = aps.top_radius - view_height;
    float d_max = rho + H;
    float x_mu = (d - d_min) / (d_max - d_min);
    float x_r = rho / H;

    output_uv = vec2(x_mu, x_r);
}

float from_sub_uvs_to_unit(float u, float resolution) {
    return (u - 0.5f / resolution) * (resolution / (resolution - 1.0f));
}

float from_unit_to_sub_uvs(float u, float resolution) {
    return (u + 0.5f / resolution) * (resolution / (resolution + 1.0f));
}

void uv_to_sky_view_params(AtmosphereParameters aps, out float view_zenith_cos_angle, out float light_view_cos_angle, in float view_height, in vec2 input_uv) {
    input_uv = vec2(from_sub_uvs_to_unit(input_uv.x, 192.0f), from_sub_uvs_to_unit(input_uv.y, 108.f));
    float v_horizon = sqrt(view_height * view_height - aps.bottom_radius * aps.bottom_radius);
    float cos_beta = v_horizon / view_height;
    float beta = acos(cos_beta);
    float zenith_horizon_angle = PI - beta;

    if (input_uv.y < 0.5f) {
        float coord = 1.0f - 2.0 * input_uv.y;

        // nolinear map
        coord = coord * coord;
        coord = 1.0f - coord;
        view_zenith_cos_angle = cos(zenith_horizon_angle * coord);
    }
    else {
        float coord = input_uv.y * 2.0 - 1.0;
        coord = coord * coord;
        view_zenith_cos_angle = cos(zenith_horizon_angle + beta * coord);
    }

    float coord = input_uv.x;
    coord = coord * coord;
    light_view_cos_angle = -(coord * 2.0 - 1.0f);
}

float compute_tmax(in AtmosphereParameters aps, in vec3 world_pos, in vec3 world_dir) {
    vec3 earth0 = vec3(0.0f, 0.0f, 0.0f);
    float t_bottom = ray_sphere_intersect_nearest(world_pos, world_dir, earth0, aps.bottom_radius);
    float t_top = ray_sphere_intersect_nearest(world_pos, world_dir, earth0, aps.top_radius);
    float t_max = 0.0f;
    
    if (t_bottom < 0.0f) {
        if (t_top < 0.0f) {
            t_max = 0.0f;
        }
        else {
            t_max = t_top;
        }
    } 
    else {
        if (t_top > 0.0f) {
            t_max = min(t_top, t_bottom);
        }
    }

    t_max = min(t_max, 9000000.0f);
    return t_max;
}

struct MediumSampleRGB {
    vec3 scattering;
    vec3 absorption;
    vec3 extinction;

    vec3 scattering_mie;
    vec3 absorption_mie;
    vec3 extinction_mie;

    vec3 scattering_ray;
    vec3 absorption_ray;
    vec3 extinction_ray;

    vec3 scattering_0zo;
    vec3 absorption_0zo;
    vec3 extinction_0zo;

    vec3 albedo;
};

MediumSampleRGB sample_medium_rgb(in vec3 world_pos, in AtmosphereParameters aps) {
    float view_height = length(world_pos) - aps.bottom_radius;

    float density_mie = exp(aps.mie_density_exp_scale * view_height);
    float density_ray = exp(aps.rayleigh_density_exp_scale * view_height);
    float density_0zo = aps.absorption_density0_linear_term * view_height + aps.absorption_density0_constant_term;
    if (view_height > aps.absorption_density0_layer_width) {
          density_0zo = aps.absorption_density1_linear_term * view_height + aps.absorption_density1_constant_term;
    }

    MediumSampleRGB s;
    
    s.scattering_mie = density_mie * aps.mie_scattering;
    s.absorption_mie = density_mie * aps.mie_absorption;
    s.extinction_mie = density_mie * aps.mie_extinction;

    s.scattering_ray = density_ray * aps.rayleigh_scattering;
    s.absorption_ray = vec3(0.0f);
    s.extinction_ray = s.scattering_ray + s.absorption_ray;

    s.scattering_0zo = vec3(0.0f);
    s.absorption_0zo = density_0zo * aps.absorption_extinction;
    s.extinction_0zo = s.scattering_0zo + s.absorption_0zo;

    s.scattering = s.scattering_mie + s.scattering_ray + s.scattering_0zo;
    s.absorption = s.absorption_mie + s.absorption_ray + s.absorption_0zo;
    s.extinction = s.extinction_mie + s.extinction_ray + s.extinction_0zo;
    s.albedo = s.scattering / max(vec3(0.001), s.extinction);

    return s;
}
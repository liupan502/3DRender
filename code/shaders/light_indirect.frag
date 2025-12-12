layout(binding = 15) uniform samplerCube env_tex_sampler;
layout(binding = 16) uniform sampler2D   dfg_tex_sampler;
layout(binding = 17) uniform IBL {
    vec3 sh_params[9];
} ibl;

// ibl specular 
vec3 prefiltered_dfg(float roughness, float nov) {
    return textureLod(dfg_tex_sampler, vec2(nov, roughness), 0.0).xyz;
}

vec3 prefiltered_radiance(const vec3 r, float roughness) {   

    float lod = 4.0f * roughness * (2.0f - roughness);
    vec3 radiance = textureLod(env_tex_sampler, r, lod).xyz;
    return radiance;
    
}

// ibl diffuse
vec3 compute_irradiance_spherical_harmoics(const vec3 n) {
    vec3 ret = ibl.sh_params[0].xyz + 
                ibl.sh_params[1].xyz * (n.y) + 
                ibl.sh_params[2].xyz * (n.z) + 
                ibl.sh_params[3].xyz * (n.x) + 
                ibl.sh_params[4].xyz * (n.y * n.x) + 
                ibl.sh_params[5].xyz * (n.y * n.z) + 
                ibl.sh_params[6].xyz * (3.0 * n.z * n.z - 1.0) + 
                ibl.sh_params[7].xyz * (n.z * n.x) + 
                ibl.sh_params[8].xyz * (n.x * n.x - n.y * n.y);
    
     return max(ret, 0.0);           
}


void evaluate_ibl(const vec3 n, const vec3 v, const vec3 f0,
            float nov , float roughness, inout vec3 color) {
    nov = nov * 0.99;
    
    vec3 dfg = prefiltered_dfg(roughness, nov);
    vec3 e = mix(dfg.xxx, dfg.yyy, f0);
    
    vec3 r = reflect(-v, -n);
    vec3 fr = e * prefiltered_radiance(r, roughness); 
    vec3 fd = base_color.xyz * compute_irradiance_spherical_harmoics(-n) * (1.0 - e)  * fd_lambert();
    color = color + (fd + fr) * 0.18f;
}



#ifndef PI
#define PI 3.1415926
#endif

float distribution(float roughness, float NoH, const vec3 h) {
    float val = 1.0 - NoH * NoH;
    float a = NoH * roughness;
    float k = roughness / (val + a * a);
    float d = k * k * (1.0 / PI);
    return clamp(d, 0.0, 1.0);
}

float visibility(float roughness, float NoV, float NoL) {
  float a2 = roughness * roughness;
  float lamda_v = NoL * sqrt((NoV - a2 * NoV) * NoV + a2);
  float lamda_l = NoV * sqrt((NoL - a2 * NoL) * NoL + a2);
  float v = 0.5 / (lamda_v + lamda_l);

  // use fast compute method
  // float v = 0.5 / mix(2.0 * NoL * NoV, NoL + NoV, roughness);

  return clamp(v, 0.0, 1.0);
}

float visibility_clear_coat(float LoH) {
  float v = 0.25 / (LoH * LoH);
  return clamp(v, 0.0, 65504.0);
}

vec3 fresnel(const vec3 f0, float VoH) {
    float f = pow(1.0 - VoH, 5.0);
    return f + f0 * (1.0 - f);
}

float fresnel(float f0, float f90, float VoH) {
    return f0 + (f90 - f0) * pow(1.0 - VoH, 5.0);
}

vec3 fresnel(const vec3 f0, float f90, float VoH) {
    // Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
    return f0 + (f90 - f0) * pow(1.0 - VoH, 5.0);
}

float fd_lambert() {
  return 1.0 / PI;
}
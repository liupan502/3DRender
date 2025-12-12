layout(location = 0) in highp vec2 uv;

layout(binding = 0) uniform sampler2DArray surround_tex_sampler;
layout(binding = 1) uniform sampler2DArray side_tex_sampler;

layout(location = 0) out vec4 uFragColor;

layout(binding = 10) uniform AvmConfigUniformBuffer {
    vec4 frame_info;
    vec4 sampler_info;
    
} config_uni_buf;

layout(binding = 11) uniform SurroundCameraIntrinsics {
            vec4 frame_info; 
            vec4 affine;
            mat4 world2cam;
} surround_camera_intri_arr[4];

vec3 yuv2rgb(vec3 yuv) {
      vec3 rgb;
      // 2020 FULL range
      // rgb.r = yuv.r + (yuv.b - 0.5) * 1.4746;
      // rgb.g = yuv.r - (yuv.g - 0.5) * 0.1645 - (yuv.b - 0.5) * 0.5713;
      // rgb.b = yuv.r + (yuv.g - 0.5) * 1.8814;

      // 601 TV range
     // rgb.r = (yuv.r - 0.0625) * 1.164 + (yuv.b - 0.5) * 1.6853;
     // rgb.g = (yuv.r - 0.0625) * 1.164 - (yuv.g - 0.5) * 0.392 - (yuv.b - 0.5) * 0.812;
     // rgb.b = (yuv.r - 0.0625) * 1.164 + (yuv.g - 0.5) * 2.016;

      // 709 TV range
       rgb.r = (yuv.r - 0.0625) * 1.164 + (yuv.b - 0.5) * 1.792;
       rgb.g = (yuv.r - 0.0625) * 1.164 - (yuv.g - 0.5) * 0.213 - (yuv.b - 0.5) * 0.534;
       rgb.b = (yuv.r - 0.0625) * 1.164 + (yuv.g - 0.5) * 2.114;

      // 2020 TV range
      // rgb.r = (yuv.r - 0.0625) * 1.164 + (yuv.b - 0.5) * 1.596;
      // rgb.g = (yuv.r - 0.0625) * 1.164 - (yuv.g - 0.5) * 0.1881 - (yuv.b - 0.5) * 0.6529;
      // rgb.b = (yuv.r - 0.0625) * 1.164 + (yuv.g - 0.5) * 2.1501;

        // 601  FULL range
        // rgb.r = yuv.r + (yuv.b - 0.5) * 1.4075;
        // rgb.g = yuv.r - (yuv.g - 0.5) * 0.3455 - (yuv.b - 0.5) * 0.7169;
        // rgb.b = yuv.r + (yuv.g - 0.5) * 1.779;


      /* if (rgb.r > 1.0f) {
          rgb = vec3(1.0f, 0.0, 0.0);
      }
      if (rgb.g > 1.0f) {
          rgb = vec3(1.0f, 0.0, 0.0);
      }
      if (rgb.b > 1.0f) {
          rgb = vec3(1.0f, 0.0, 0.0);
      } */
    return rgb;
}

void main_tmp() {
    uFragColor.rgb = vec3(0.0, 1.0, 1.0);
    uFragColor.a = 1.0;
}

vec2 compute_uv(int idx, vec2 input_uv) {    
    vec2 new_uv = vec2(0.0);
    float z = config_uni_buf.sampler_info.z;
    float x = (input_uv.r - 0.5) * config_uni_buf.frame_info.x + config_uni_buf.frame_info.z;
    float y = (input_uv.g - 0.5) * config_uni_buf.frame_info.y + config_uni_buf.frame_info.w;

    vec3 pos = vec3(x, y, z);

    pos = normalize(pos);

    float radius = length(vec2(pos.x, pos.y));
    mat4 world2cam = surround_camera_intri_arr[idx].world2cam;
    if (radius > 0.000001) {
        float theta = atan(-pos.z / radius);
        float sum = 0.0;
        float factor = 1.0f;
        for (int i = 0; i < 11; i++) {
            int col_idx = i % 4;
            int row_idx = i / 4;
            sum += (factor * world2cam[row_idx][col_idx]);
            factor *= theta;
        }
        new_uv.x = pos[0] / radius * sum;
        new_uv.y = pos[1] / radius * sum;
    }
    
    new_uv.x = new_uv.x * surround_camera_intri_arr[idx].affine.r + 
               new_uv.y * surround_camera_intri_arr[idx].affine.g + 
                          surround_camera_intri_arr[idx].frame_info.b;
    new_uv.y = new_uv.x * surround_camera_intri_arr[idx].affine.b + 
               new_uv.y + surround_camera_intri_arr[idx].frame_info.a;
    new_uv.x = new_uv.x / surround_camera_intri_arr[idx].frame_info.r;
    new_uv.y = new_uv.y / surround_camera_intri_arr[idx].frame_info.g;
    
    return new_uv;
}

void main() {
    int camera_type = int(config_uni_buf.sampler_info.r);
    float camera_idx = config_uni_buf.sampler_info.g;
    vec3 yuv = vec3(0.0);

    vec2 new_uv = compute_uv(int(camera_idx), uv);
    // vec2 new_uv = uv;
    if (camera_type == 0) {
        yuv = texture(surround_tex_sampler, vec3(new_uv.r, new_uv.g, camera_idx)).rgb;
    }
    else {
        yuv = texture(side_tex_sampler, vec3(new_uv.r, new_uv.g, camera_idx)).rgb;
    }
    
    uFragColor.rgb = yuv2rgb(yuv);
    uFragColor.a = 1.0;
}

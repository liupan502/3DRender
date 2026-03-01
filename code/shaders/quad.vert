#version 450
layout(location = 0) in vec2 pos;
layout(location = 0) out highp vec2 uv;

// #include "inc/prerotate.h"

void main()
{
    gl_Position = vec4(pos , 0.0, 1.0);
    uv = 0.5 * pos + 0.5;
    // gl_Position = vec4(vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2) * 2.0f - 1.0f, 0.0f, 1.0f);
    // prerotate_fixup_clip_xy();
}
$input v_color0, v_normal, v_uv0, v_wpos

uniform vec4 u_camera;

#include "common.sh"
#include "bgfx_shader.sh"

SAMPLER2D(s_tex0,0);

void main()
{
    float v = texture2D(s_tex0, v_uv0).r;
    vec4 color = vec4(v,v,v,1.0);

    gl_FragColor = color;
}
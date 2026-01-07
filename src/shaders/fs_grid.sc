$input v_color0, v_normal, v_wpos

uniform vec4 u_camera;

void main()
{
    vec4 color = v_color0;

    float dis_w_center = length(v_wpos);
    float fade_dis = 25.f;
    float co = fade_dis - dis_w_center;
    co = co / fade_dis;

    gl_FragColor = color * co;
}
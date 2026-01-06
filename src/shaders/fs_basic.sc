$input v_color0, v_normal, v_wpos

void main()
{
    vec3 DIRECT_LIGHT = vec3(-1.0,-1.0,0.0);
    DIRECT_LIGHT = -normalize(DIRECT_LIGHT);

    float d = dot(DIRECT_LIGHT,v_normal);
    float amb = 0.2;
    float dirl = 0.8;
    float verti = 1.0 + clamp(v_wpos.y * 0.01,-0.33,0.33);

    float co = (amb + dirl * max(0.0,d)) * verti;
    vec4 color = co * vec4(1.0);
    color.w = 1.0;

    gl_FragColor = color;
}
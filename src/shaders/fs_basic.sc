$input v_color0, v_normal, v_wpos

uniform vec4 u_camera;

float linearizeDepth(float d)
{
    float n = u_camera.x;
    float f = u_camera.y;
    return (2.0 * n * f) / (f + n - (2.0 * d - 1.0) * (f - n));
}

void main()
{
    vec3 DIRECT_LIGHT = vec3(1.0,1.0,-1.0);
    DIRECT_LIGHT = -normalize(DIRECT_LIGHT);

    float d = dot(DIRECT_LIGHT,v_normal);
    float amb = 0.2;
    float dirl = 0.8;
    float verti = 1.0 + clamp(v_wpos.z * 0.05,-0.66,0.66);

    float co = (amb + dirl * max(0.0,d)) * verti;
    vec4 color = co * vec4(1.0);
    color.w = 1.0;

    float depth = 1.0 - gl_FragCoord.z;

    //float ld = linearizeDepth(depth);

    color.rgb *= (depth * 0.4 + 0.8);
    gl_FragColor = color;
}
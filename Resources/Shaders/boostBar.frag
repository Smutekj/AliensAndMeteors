#version 300 es 

precision mediump float;    

// #include "../../external/lygia/generative/pnoise.glsl"

uniform sampler2D u_noise_texture;


in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform int u_booster_disabled = 0;
uniform float u_booster_ratio = 0.;
uniform vec3 u_color = vec3(1.,1.2,0.);
uniform vec3 u_color_edge = vec3(1.,0.,1.);
uniform float u_time = 0.;

float sdBox( vec2 p, vec2 b )
{
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

float noise(vec2 uv)
{
    return texture(u_noise_texture, uv).r; 
}

void main()                                  
{   

    vec2 uv1 = v_tex_coord;
    vec2 uv2 = v_tex_coord;
    vec2 uv3 = v_tex_coord;
    
    float t = u_time * 0.369;

    vec2 scroll_speed1 = vec2(-0.001*sin(9.959*t), 0.3 + 0.*v_tex_coord.y * 0.2);
    vec2 scroll_speed2 = vec2(0.5, 0.15);
    vec2 scroll_speed3 = vec2(0.35, -0.13);

    uv1 = v_tex_coord / vec2(0.7, 0.8);
    uv2 = v_tex_coord / vec2(0.5, 1.0);
    uv3 = v_tex_coord / vec2(1.2, 0.69);
    
    uv1 = uv1 - t * scroll_speed1;
    uv2 = uv2 - t * scroll_speed2;
    uv3 = uv3 - t * scroll_speed2;

    float result_r = noise(uv2) * noise(uv3);
    
    float circle_sdf = length(v_tex_coord - vec2(0.5, 0.5));
    float circle_in = 1.-smoothstep(0.1, 0.12, circle_sdf);
    float circle_out = smoothstep(0.08, 0.14, circle_sdf) * (1. - smoothstep(0.14, 0.21, circle_sdf));
    float y_gradient = 1.-smoothstep(0.45, 0.55, v_tex_coord.y);

    float sdf = sdBox(v_tex_coord - vec2(0.5, 0.5), vec2(0.51, 0.47));
    float rect_shape = 1. - smoothstep(-0.05, 0.0, sdf) ;
    sdf = sdBox(v_tex_coord - vec2(0.5, 0.5), vec2(0.50, 0.5));
    float rect_border = smoothstep(-0.1, -0.05, sdf);

    float alpha = 1.-smoothstep(u_booster_ratio, u_booster_ratio + 0.03, v_tex_coord.x);
    float alpha_flame = 1.-smoothstep(0.1, 0.4, result_r);

    vec4 res = vec4(0.);
    res = alpha* vec4(mix(result_r *vec3(1.0, 0.2, 0.), vec3(1.0, 0.9, 0.1), alpha_flame),  1.0);

    //! goes from green to red depending on booster ratio. When disabled is red
    vec4 edge_color = mix(vec4(u_booster_ratio, 1.0 - u_booster_ratio * u_booster_ratio * u_booster_ratio, 0., 1.), vec4(1.0, 0., 0., 1.), float(u_booster_disabled));
    res = mix(res, edge_color, rect_border);

    FragColor = res;
}                                   



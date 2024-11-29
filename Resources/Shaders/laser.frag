#version 300 es 

precision mediump float;    
                 
in vec2 v_tex_coord;                          
in vec4 v_color;       

out vec4 FragColor;

uniform float u_time = 0.;

uniform vec3 u_freq_vec = vec3(10.2,5.4, 2.5);
uniform vec3 u_freq_vec2 = vec3(5.2,1.4, 2.5);
uniform vec3 u_color = vec3(1.,2.9,2.);
uniform vec3 u_color_fire = vec3(1.,1.,0.);
uniform vec3 u_color_edge = vec3(10.,5.,0.);


uniform sampler2D u_texture;

void main()                                  
{   
    vec2 center = vec2(0.5, 0.5);
    vec2 dr = v_tex_coord - center;
    // vec3 dr3 = vec3(15.*dr, u_time*u_time_multiplier);
    // vec3 dr33 = vec3(5.*dr, u_time*u_time_multiplier);

    
    float shape_factor = 1.;
    float edge_factor = 0.;
    if(v_tex_coord.y < 0.1 || v_tex_coord.y > 0.9 ||
        v_tex_coord.x < 0.1 || v_tex_coord.x > 0.9 ) //! border region
    {
        edge_factor = 1.;
    }   
    // float texture_factor = pnoise(dr3, u_freq_vec) *shape_factor;
    // float texture_factor2 = pnoise(dr33, u_freq_vec2) *shape_factor;

    float u_time_multiplier = 2.0;
    float u_space_multiplier = 5.5;

    float t = abs(v_tex_coord.y -0.5);
    vec3 gay_color = vec3(1.);
    gay_color.r = (2. + cos(u_time * u_time_multiplier + t*u_space_multiplier))/2.; 
    gay_color.g = (1. + sin(u_time * u_time_multiplier+ t*u_space_multiplier))/2.; 
    gay_color.b = (1. + mod(t*u_space_multiplier + u_time*u_time_multiplier, 1.0))/2.; 


    vec3 result = 2.*gay_color + edge_factor * u_color_edge;

    FragColor = vec4(result*shape_factor, shape_factor);
}                                          
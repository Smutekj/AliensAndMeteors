#version 300 es

precision highp float;                     
in vec2 v_tex_coord;
in vec4 v_color;                          

out vec4 FragColor;

uniform sampler2D u_texture;

void main()                                  
{                                            
    FragColor = vec4(1.,0.,0.,1.);//texture(u_texture, v_tex_coord); //vec4(texture(u_texture, v_tex_coord).a, 0., 0., 1.0);//
}                                            
#version 130

uniform float intensity = 4.0f;
uniform sampler2D image;
uniform sampler2D light_data;

void main(void)
{
	vec3 input_color = texture2D( image, gl_TexCoord[0].xy ).xyz;
	vec3 light = texture2D( light_data, gl_TexCoord[0].xy ).xyz;

    gl_FragColor = vec4(input_color+light, 1) ;

}

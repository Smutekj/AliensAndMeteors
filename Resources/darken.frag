#version 130

uniform float factor = 0.3f;
uniform sampler2D image;

void main(void)
{
	vec3 input_color = texture2D( image, gl_TexCoord[0].xy ).xyz;

    gl_FragColor = vec4(input_color-factor, 1) ;

}

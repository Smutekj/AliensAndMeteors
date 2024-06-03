#version 130

uniform intensity = 4.0f;
uniform sampler2D image;


void main(void)
{
	vec4 input_color = texture2D( image, gl_TexCoord[0].xy );

    gl_FragColor = input_color * gl_FrontColor * intensity;

}

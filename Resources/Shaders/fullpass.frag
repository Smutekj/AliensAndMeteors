#version 120

uniform sampler2D image;

void main(void)
{
	gl_FragColor = vec4(texture2D( image, gl_TexCoord[0].xy ));
}

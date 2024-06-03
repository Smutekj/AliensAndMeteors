#version 120

uniform sampler2D image1;
uniform sampler2D image2;

void main(void)
{

	gl_FragColor = texture2D( image1,  gl_TexCoord[0].xy) + texture2D( image2, gl_TexCoord[0].xy);

}

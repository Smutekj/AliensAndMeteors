#version 120

uniform sampler2D image;


void main(void)
{
	vec2 texCoords = gl_TexCoord[0].xy;
	vec4 vertexColor = gl_Color;

	gl_FragColor = texture2D( image, vec2(texCoords)) ;

}

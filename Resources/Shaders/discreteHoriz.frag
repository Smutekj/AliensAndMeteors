#version 140

uniform sampler2D image;

uniform float offset[5] = float[]( 0.0, 1.0, 2.0, 3.0, 4.0 );
uniform float weight[5] = float[]( 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 );

void main(void)
{
	vec2 texCoords = gl_TexCoord[0].xy;
	vec4 vertexColor = gl_Color;

	vec2 textureSize = textureSize(image, 0);

	vec4 result = texture2D( image, vec2(texCoords)) * weight[0];
	for (int i=1; i<5; i++)
	{
		result += texture2D( image, ( texCoords+vec2(offset[i]/textureSize.x, 0.0) ) ) * weight[i];
 		result += texture2D( image, ( texCoords-vec2(offset[i]/textureSize.x, 0.0) ) ) * weight[i];
	}
	gl_FragColor = vec4(result);
}

#version 120

uniform sampler2D image;

const float Threshold = 0.07;
const float Factor   = 2.0;

void main()
{
	vec4 sourceFragment = texture2D(image, gl_TexCoord[0].xy);
	float luminance = sourceFragment.r * 0.2126 + sourceFragment.g * 0.7152 + sourceFragment.b * 0.0722;
    sourceFragment *= clamp(luminance - Threshold, 0.0, 1.0) * Factor;
	gl_FragColor = sourceFragment;
}
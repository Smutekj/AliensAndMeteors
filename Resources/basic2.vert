#version 130


out vec4 position;
out vec4 color;
out vec2 texCoords;

void main()
{
    // transform the vertex position
    position = gl_ModelViewProjectionMatrix * gl_Vertex;

    // transform the texture coordinates
    texCoords = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;

    // forward the vertex color
    color = gl_Color;
}
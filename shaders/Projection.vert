#version 150

#extension GL_ARB_explicit_attrib_location : enable

#define POSITION  0
#define NORMAL    1
#define TEXTCOORD 3

layout(location = POSITION)  in vec3  vi_PositionOS;
layout(location = NORMAL)    in vec3    vi_NormalOS;
layout(location = TEXTCOORD) in vec2   vi_TextCoord;

out float cosine;
out vec2 texCoord;

uniform mat4       modelViewProjection;
uniform mat4 modelViewInverseTranspose;

void main()
{
    gl_Position = modelViewProjection * vec4(vi_PositionOS,1.0);
    texCoord = vi_TextCoord;
    cosine = (modelViewInverseTranspose * vec4(vi_NormalOS,1.0)).z;
}

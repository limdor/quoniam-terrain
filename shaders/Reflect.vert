#version 150

#extension GL_ARB_explicit_attrib_location : enable

#define TEXTCOORD 3

layout(location = TEXTCOORD) in vec2   vi_TextCoord;

uniform mat4 modelViewProjection; // World view projection matrix
uniform vec3 materialColor;

out vec4 color;

void main()
{
	vec4 posicio = vec4(vi_TextCoord.xy, -0.5, 1.0);
	gl_Position = modelViewProjection * vec4(vi_TextCoord, 0.0, 1.0);

	color = vec4(materialColor,1.0);
}

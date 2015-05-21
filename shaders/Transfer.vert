#version 150

#extension GL_ARB_explicit_attrib_location : enable

#define POSITION  0
#define TEXTCOORD 3

layout(location = POSITION)  in vec4  vi_PositionOS;
layout(location = TEXTCOORD) in vec4   vi_TextCoord;

uniform float direction;

out vec2 p1;
out vec2 p2;
out float factor;

void main()
{
	vec4 position = vi_PositionOS;
	vec4 texCoord = vi_TextCoord;
	if(((direction == 0) && (position.b != 1.0) && (position.b < 0.0) && ((texCoord.b > 0.0) || (texCoord.b == 1)))
	|| ((direction == 1) && (position.b != 1.0) && (position.b > 0.0) && ((texCoord.b < 0.0) || (texCoord.b == 1))))
	{

		p1 = vec2(position.r, position.g);

		p2 = vec2(texCoord.r, texCoord.g);

		gl_Position = vec4((p1 * 2.0) - vec2(1.0,1.0), 0.0,1.0);

		if (texCoord.b == 1) factor = 1;
		else factor = abs(texCoord.a - position.a);
	}
	else
	{
		gl_Position = vec4( 0.0,0.0, 2.0, 1.0 );
		p1 = p2 = vec2(1.0,1.0);
		factor = 0.5;
	}
}

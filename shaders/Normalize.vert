varying vec2 texCoord;

uniform mat4 modelViewProjection;

void main()
{
	vec4 posicio = vec4(gl_Vertex.xyz, 1.0);
	gl_Position = modelViewProjection * posicio;

	texCoord = gl_MultiTexCoord0.st;
}

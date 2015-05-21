#version 150

uniform float                   res;
uniform float                 first;
uniform sampler2D              ztex;
uniform int   polygonalTexturesSize;
uniform int	             offset;

in float cosine;
in vec2 texCoord;

void main()
{
	if( first == 0.0)
	{
		float depth = texture2D(ztex,gl_FragCoord.xy/res).a;
		if (gl_FragCoord.z < (depth + 0.000001)) discard;
	}
        gl_FragColor.rg = vec2( ((gl_PrimitiveID + offset) % polygonalTexturesSize), ((gl_PrimitiveID + offset) / polygonalTexturesSize) );
        gl_FragColor.rg += 0.5f;
        gl_FragColor.rg /= float(polygonalTexturesSize);

	gl_FragColor.b = cosine;
	gl_FragColor.a = gl_FragCoord.z;
}

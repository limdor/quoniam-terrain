#version 150

uniform sampler2D texture;
uniform float res;

varying vec2 texCoord;

void main()
{
	vec4 aux1 = texture2D(texture,texCoord);
	if(aux1.a > 10.0)
	{
		gl_FragColor = vec4(aux1.rgb/aux1.a,1.0);
	}
	else
	{
//		float res = 256.0;
		float dif = 1.0/res;

		float counter = 0.0;
		vec4 aux = vec4(0.0,0.0,0.0,0.0);

		vec4 aux2 = texture2D(texture,vec2(texCoord.r+dif,texCoord.g));
		vec4 aux3 = texture2D(texture,vec2(texCoord.r+dif,texCoord.g+dif));
		vec4 aux4 = texture2D(texture,vec2(texCoord.r,texCoord.g+dif));
		vec4 aux5 = texture2D(texture,vec2(texCoord.r-dif,texCoord.g+dif));
		vec4 aux6 = texture2D(texture,vec2(texCoord.r-dif,texCoord.g));
		vec4 aux7 = texture2D(texture,vec2(texCoord.r-dif,texCoord.g-dif));
		vec4 aux8 = texture2D(texture,vec2(texCoord.r,texCoord.g-dif));
		vec4 aux9 = texture2D(texture,vec2(texCoord.r+dif,texCoord.g-dif));
//		if(aux1.a>0.0)
//		{
//			aux += aux1;
//			counter += 1.0;
//		}
		if(aux2.a>10.0)
		{
			aux += aux2;
			counter += 1.0;
		}
		if(aux3.a>10.0)
		{
			aux += aux3;
			counter += 1.0;
		}
		if(aux4.a>10.0)
		{
			aux += aux4;
			counter += 1.0;
		}
		if(aux5.a>10.0)
		{
			aux += aux5;
			counter += 1.0;
		}
		if(aux6.a>10.0)
		{
			aux += aux6;
			counter += 1.0;
		}
		if(aux7.a>10.0)
		{
			aux += aux7;
			counter += 1.0;
		}
		if(aux8.a>10.0)
		{
			aux += aux8;
			counter += 1.0;
		}
		if(aux9.a>10.0)
		{
			aux += aux9;
			counter += 1.0;
		}
		if(counter < 1.0) gl_FragColor = vec4(0.0,0.0,0.0,0.0);
		else
		{
			aux /= counter;
			gl_FragColor = vec4(aux.rgb/aux.a,1.0);
		};
	}
}

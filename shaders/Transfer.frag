#version 150

uniform float dmax;

in vec2 p1;
in vec2 p2;
in float factor;

void main()
{
    vec3 color = vec3(1.0f, 1.0f, 1.0f);

    if(factor>=dmax)
    {
        gl_FragColor.rgb = vec3(1.0f, 1.0f, 1.0f);
    }
    else
    {
        gl_FragColor.rgb = color * sqrt(factor/dmax);
    }
    gl_FragColor.a = 1.0;
}

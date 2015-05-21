#version 150

uniform bool ignoreNormals;
uniform int offset;

void main()
{
    if(gl_FrontFacing || ignoreNormals)
    {
        float valor = float(gl_PrimitiveID + offset + 1);
        gl_FragColor.r = valor;
    }
    else
    {
        gl_FragColor.r = 0.0f;
    }
}

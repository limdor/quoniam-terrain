#version 150

#extension GL_EXT_gpu_shader4 : enable

uniform sampler2D       diffuseTexture;
uniform sampler2D     polygonalTexture;
uniform sampler2D    visibilityTexture;

uniform bool               faceCulling;
uniform bool       applyDiffuseTexture;
uniform bool   viewPolygonalVisibility;
uniform bool                   drawRGB;
uniform bool applyPolygonalInformation;
uniform bool  onlyPolygonalInformation;
uniform float                    power;
uniform vec3                materialKa;
uniform vec3                materialKd;
uniform float                   weight;
uniform float                luminance;
uniform int	 polygonalTexturesSize;
uniform int	                offset;

in vec2   vo_TextCoord; // Coordenades de textura

vec3 ThermalScale(in float);

vec4 ShadeFragment()
{
    vec4 returnColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    if( gl_FrontFacing )
    {
        vec2 textCoord = vec2( ((gl_PrimitiveID + offset) % polygonalTexturesSize), ((gl_PrimitiveID + offset) / polygonalTexturesSize) );
        textCoord += 0.5f;
        textCoord /= float(polygonalTexturesSize);

        if(onlyPolygonalInformation)
        {
            vec3 value = texture(polygonalTexture, textCoord).rgb;

            value = 1.0f - value;
            value = vec3(pow(value.r, power), pow(value.g, power), pow(value.b, power));

            if(drawRGB)
            {
                returnColor = vec4(ThermalScale(value.r), 1.0f);
            }
            else
            {
                returnColor = vec4(value, 1.0f);
            }
        }
        else
        {
            vec3 ka = materialKa;
            vec3 kd = materialKd;

            if(ka == vec3(0.0f, 0.0f, 0.0f))
            {
                ka = kd;
            }
            vec3 diffuseColor = vec3(1.0f, 1.0f, 1.0f);
            //vec3 diffuseColor = vec3(0.74f, 0.74f, 0.74f);

            if( applyDiffuseTexture )
            {
                diffuseColor = texture(diffuseTexture, vo_TextCoord).rgb;
            }

            kd *= diffuseColor;
            ka *= diffuseColor;

            vec3 intensityAmbientLight = vec3(1.0f, 1.0f, 1.0f);
            if(applyPolygonalInformation)
            {
                vec3 value = texture(polygonalTexture, textCoord).rgb;

                value = 1.0f - value;
                value = vec3(pow(value.r, power), pow(value.g, power), pow(value.b, power));
                if(drawRGB)
                {
                    returnColor = vec4( ( 1.0f - weight ) * ka + weight * ThermalScale(value.r), 1.0f);
                }
                else
                {
                    intensityAmbientLight = value;

                    //returnColor = vec4( ( 1.0f - weight ) * kd + weight * intensityAmbientLight * luminance, 1.0f );
                    returnColor = vec4( ka * ( weight * ( intensityAmbientLight / luminance - 1.0f ) + 1.0f ), 1.0f );
                }
            }
            else
            {
                returnColor = vec4( kd, 1.0f );
            }

            if(viewPolygonalVisibility)
            {
                returnColor = vec4( texture(visibilityTexture, textCoord).x, returnColor.y, returnColor.z, 1.0f );
            }
        }
    }
    else
    {
        if(faceCulling)
        {
            discard;
        }
    }
    return returnColor;
}

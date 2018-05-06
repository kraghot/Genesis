////change to 1 to enable, 0 to disable slope based blending (in case of 0, height based blending is enabled
#ifndef ENABLE_SLOPE_BASED_BLEND
#define ENABLE_SLOPE_BASED_BLEND 1
#endif

uniform vec3 uLightPos;
uniform vec3 uCamPos;

uniform sampler2D uTexColor;
uniform sampler2D uTexNormal;

uniform sampler2D uSplatmapTex;

uniform sampler2DArray uTerrainTex;
uniform sampler2DArray uTerrainNormal;

uniform float fRenderHeight;

in vec3 vWorldPosition;
in vec3 vNormal;
in vec4 vColor;
in vec3 vTangent;
in vec2 vTexCoord;
in float vSlopeY;

out vec4 fColor;

void main()
{
    const float fRange1 = 0.3f;
    const float fRange2 = 0.5f;
    const float fRange3 = 0.7f;
    const float fRange4 = 0.9f;

    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    //float fScale = uSlopeBlending? vSlopeY : (vWorldPosition.y*1.0f)/(fRenderHeight*1.0f);

#if ENABLE_SLOPE_BASED_BLEND
    float fScale = vSlopeY;
#else
    float fScale = (vWorldPosition.y*1.0f)/(fRenderHeight*1.0f);
#endif

    vec4 vTexColor = vec4(0.0f);
    vec4 vTexNormal = vec4(0.0f);

    if(fScale >= 0.0 && fScale <= fRange1){
        vTexColor = texture(uTerrainTex, vec3(vTexCoord, 0.3));
        vTexNormal = texture(uTerrainNormal, vec3(vTexCoord,0.3));

        r = 255;
    }
    else if(fScale <= fRange2){
        fScale -= fRange1;
        fScale /= (fRange2-fRange1);

       float fScale2 = fScale;
       fScale = 1.0-fScale;

       vTexColor += texture(uTerrainTex, vec3(vTexCoord, 0.0))*fScale;
       vTexColor += texture(uTerrainTex, vec3(vTexCoord, 1.0))*fScale2;

       vTexNormal += texture(uTerrainNormal, vec3(vTexCoord, 0.5)) *fScale;
       vTexNormal += texture(uTerrainNormal, vec3(vTexCoord, 1.3)) *fScale2;

       r += 255 * fScale;
       g += 255 * fScale2;

    }

    else if(fScale <= fRange3){
        vTexColor = texture(uTerrainTex, vec3(vTexCoord, 1.0));
        vTexNormal = texture(uTerrainNormal, vec3(vTexCoord, 1.0));

        g += 255;
    }
    else if(fScale <= fRange4)
    {
            fScale -= fRange3;
            fScale /= (fRange4-fRange3);

            float fScale2 = fScale;
            fScale = 1.0-fScale;

            vTexColor += texture(uTerrainTex, vec3(vTexCoord, 1.0))*fScale;
            vTexColor += texture(uTerrainTex, vec3(vTexCoord, 2))*fScale2;

            vTexNormal += texture(uTerrainNormal, vec3(vTexCoord, 1.0)) *fScale;
            vTexNormal += texture(uTerrainNormal, vec3(vTexCoord, 2)) *fScale2;

            g += 255 * fScale;
            b += 255 * fScale2;
    }
    else{
        vTexColor = texture(uTerrainTex, vec3(vTexCoord, 2));
        vTexNormal = texture(uTerrainNormal, vec3(vTexCoord, 2));

        b = 255;
       }


   // vec4 vFinalTexColor = vTexColor;
//    vec4 vFinalTexColor;
//    vFinalTexColor.r = r;
//    vFinalTexColor.g = g;
//    vFinalTexColor.b = b;
    vec3 normalMap = vTexNormal.rgb;


    vec3 temp = texture(uSplatmapTex, vTexCoord).rgb;
    vec4 vFinalTexColor = vec4(temp,1.f);



    // material settings
    float shininess = 8;
    float ka = 0.1;
    float kd = 0.7;
    float ks = 0.3;

    // read textures
    vec3 color = texture(uTexColor, vTexCoord).rgb;
    //vec3 normalMap = texture(uTexNormal, vTexCoord).rgb;

    // build local coordinate system
    vec3 L = normalize(uLightPos - vWorldPosition);
    vec3 V = normalize(uCamPos - vWorldPosition);
    vec3 H = normalize(V + L);
    vec3 N = normalize(vNormal);
    vec3 T = normalize(vTangent);
    vec3 B = normalize(cross(T, N));
    mat3 tbn = mat3(T, B, N);

    // apply normal mappingvVertexData
    normalMap.xy = normalMap.xy * 2 - 1;
    N = normalize(tbn * normalMap);

    // blinn-phong model
    float albedo = ka;
    float diffuse = kd * max(0, dot(N, L));
    float specular = ks * pow(max(0, dot(N, H)), shininess);
//    fColor = color * (albedo + diffuse + specular);
    fColor = vFinalTexColor * (albedo + diffuse + specular);
}

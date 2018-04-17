uniform vec3 uLightPos;
uniform vec3 uCamPos;

uniform sampler2D uTexColor;
uniform sampler2D uTexNormal;
uniform sampler2D gSampler0;
uniform sampler2D gSampler1;
uniform sampler2D gSampler2;
uniform float fRenderHeight;

in vec3 vWorldPosition;
in vec3 vNormal;
in vec4 vColor;
in vec3 vTangent;
in vec2 vTexCoord;

out vec4 fColor;

void main()
{
    const float fRange1 = 0.15f;
    const float fRange2 = 0.3f;
    const float fRange3 = 0.65f;
    const float fRange4 = 0.85f;

    float fScale = (vWorldPosition.y*1.0f)/(fRenderHeight*1.0f);
    vec4 vTexColor = vec4(0.0f);
    vec4 testColor = vec4(1, 1, 1, 1);

    if(fScale >= 0.0 && fScale <= fRange1)
        vTexColor = texture(gSampler0, vTexCoord);
    else if(fScale <= fRange2){
        fScale -= fRange1;
        fScale /= (fRange2-fRange1);

       float fScale2 = fScale;
       fScale = 1.0-fScale;

       vTexColor += texture(gSampler0, vTexCoord)*fScale;
       vTexColor += texture(gSampler1, vTexCoord)*fScale2;
    }

    else if(fScale <= fRange3)
        vTexColor = texture(gSampler1, vTexCoord);
    else if(fScale <= fRange4)
    {
            fScale -= fRange3;
            fScale /= (fRange4-fRange3);

            float fScale2 = fScale;
            fScale = 1.0-fScale;

            vTexColor += texture(gSampler1, vTexCoord)*fScale;
            vTexColor += texture(gSampler2, vTexCoord)*fScale2;
    }
    else vTexColor = texture(gSampler2, vTexCoord);


    vec4 vFinalTexColor = vTexColor;



    // material settings
    float shininess = 8;
    float ka = 0.1;
    float kd = 0.7;
    float ks = 0.3;

    // read textures
    vec3 color = texture(uTexColor, vTexCoord).rgb;
    vec3 normalMap = texture(uTexNormal, vTexCoord).rgb;

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
    fColor = vFinalTexColor;
}

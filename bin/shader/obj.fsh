uniform vec3 uLightPos;
uniform vec3 uCamPos;

uniform sampler2D uTexColor;
uniform sampler2D uTexNormal;

uniform sampler2D uSplatmapTex;
uniform sampler2D uAmbientOcclusionMap;

uniform sampler2DArray uTerrainTex;
uniform sampler2DArray uTerrainNormal;
uniform sampler2D uIndexMap;

uniform float fRenderHeight;
uniform bool uDrawDebugRain;

in vec3 vWorldPosition;
in vec3 vNormal;
in vec4 vColor;
in vec3 vTangent;
in vec2 vTexCoord;
in float vSlopeY;
in vec2 vHeightCoord;

out vec4 fColor;

void main()
{
    vec4 SplatmapColor = texture(uSplatmapTex, vHeightCoord);
    vec4 indexMap = texture(uIndexMap, vHeightCoord);
    indexMap *= 255;
    float weights[4] = {SplatmapColor.r, SplatmapColor.g, SplatmapColor.b, SplatmapColor.a};
    float indices[4] = {indexMap.r, indexMap.g, indexMap.b, indexMap.a};

    vec3 normalMap;
    vec4 vFinalTexColor;

    for(int i = 0; i < 4; i++)
    {
        // magic number that means no texture
        if(indices[i] == 255)
            continue;

        normalMap += texture(uTerrainNormal, vec3(vTexCoord, indices[i])).rgb * weights[i];
        vFinalTexColor += texture(uTerrainTex, vec3(vTexCoord, indices[i])) * weights[i];
    }

//    vec3 normalMap = (texture(uTerrainNormal, vec3(vTexCoord, 0.0)) * SplatmapColor.r  + texture(uTerrainNormal, vec3(vTexCoord, 1.0)) * SplatmapColor.g + texture(uTerrainNormal, vec3(vTexCoord, 2.0)) * SplatmapColor.b + texture(uTerrainNormal, vec3(vTexCoord, 3.0)) * SplatmapColor.a).rgb;

//    vec4 vFinalTexColor = texture(uTerrainTex, vec3(vTexCoord, 0.0)) * SplatmapColor.r + texture(uTerrainTex, vec3(vTexCoord, 1.0)) * SplatmapColor.g + texture(uTerrainTex, vec3(vTexCoord, 2.0)) * SplatmapColor.b + texture(uTerrainTex, vec3(vTexCoord, 3.0)) * SplatmapColor.a;

    if(uDrawDebugRain)
    {
        float val = SplatmapColor.a;
        if(val >= 0.95f)
            vFinalTexColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
        else
            vFinalTexColor = texture(uTerrainTex, vec3(vTexCoord, 0.0)) * SplatmapColor.r  + texture(uTerrainTex, vec3(vTexCoord, 1.0)) * SplatmapColor.g + texture(uTerrainTex, vec3(vTexCoord, 2.0)) * SplatmapColor.b + texture(uTerrainTex, vec3(vTexCoord, 3.0)) * SplatmapColor.a;
    }

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

    float ambient = 0.1f * texture(uAmbientOcclusionMap, vHeightCoord).r;

//    fColor = color * (albedo + diffuse + specular);
    fColor = pow(vFinalTexColor * (albedo + diffuse + specular), vec4(1/2.224));
}

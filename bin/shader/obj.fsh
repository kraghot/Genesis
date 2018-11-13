uniform vec3 uLightPos;
uniform vec3 uCamPos;

uniform sampler2D uSplatmapTex;
uniform sampler2D uAmbientOcclusionMap;

uniform sampler2DArray uTerrainTex;
uniform sampler2DArray uTerrainNormal;
uniform sampler2D uIndexMap;

uniform float fRenderHeight;
uniform bool uDrawDebugRain;

uniform sampler2DRect uShadowMap;
uniform mat4 uShadowViewProjMatrix;
uniform mat4 uBiasMatrix;


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
    vec2 poissonDisk[4] = vec2[](
      vec2( -0.94201624, -0.39906216 ),
      vec2( 0.94558609, -0.76890725 ),
      vec2( -0.094184101, -0.92938870 ),
      vec2( 0.34495938, 0.29387760 )
    );

    vec4 SplatmapColor = texture(uSplatmapTex, vHeightCoord);
    vec4 indexMap = texture(uIndexMap, vHeightCoord);
    float weights[4] = {SplatmapColor.r, SplatmapColor.g, SplatmapColor.b, SplatmapColor.a};
    float indices[4] = {indexMap.r, indexMap.g, indexMap.b, indexMap.a};

    vec3 normalMap = {0, 0, 0};
    vec4 vFinalTexColor = {0, 0, 0, 0};

    for(int i = 0; i < 4; i++)
    {
        // magic number that means no texture
        if(indices[i] == 255)
        {
            vFinalTexColor = vec4(1.0, 1.0, 1.0, 1.0);
            continue;
        }

        normalMap += texture(uTerrainNormal, vec3(vTexCoord, indices[i])).rgb * weights[i];
        vFinalTexColor += texture(uTerrainTex, vec3(vTexCoord, indices[i])) * weights[i];
    }

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

    //mat4 depthBiasMVP = uBiasMatrix * uShadowViewProjMatrix;

    float visibility = 1.0f;

    float bias = 0.005*tan(acos(dot(N, L))); // cosTheta is dot( n,l ), clamped between 0 and 1
    bias = clamp(bias, 0.f, 0.01f);

    vec4 shadowPos = uShadowViewProjMatrix * vec4(vWorldPosition, 1.0);
    shadowPos.xyz /= shadowPos.w;
    float shadowDepth = texture(uShadowMap, (shadowPos.xy * .5 + .5) * 4096.f).r;
    float refDepth = shadowPos.z * .5 + .5;
    //float inShadow = refDepth > shadowDepth? 1.0f : 0.0f;
    float shadowFactor = float(refDepth <= shadowDepth + 0);

    for (int i=0;i<4;i++){
      if ( texture(uShadowMap, ((shadowPos.xy * .5 + .5) * 4096.f) + poissonDisk[i]/700.0 ).r  <  refDepth - bias ){
        visibility-=0.2;
      }
    }

//    if (shadowPos.w < 0.0f) // fix "behind-the-light"
//        shadowFactor = 1;

//    float visibility = 1.0f;
//    if(texture(uShadowMap, shadowPos.xy).z < shadowPos.z)
//        visibility = 0.f;



    fColor = pow(vFinalTexColor * (albedo + (diffuse * visibility) + (specular * visibility) + ambient), vec4(1/2.224));

    if(vWorldPosition.y < 8)
    {
        float mixFactor = vWorldPosition.y / 8;
        fColor.a = mix(0, 1, mixFactor);
    }
    else
        fColor.a = 1.0;
}

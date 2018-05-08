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
in vec2 vHeightCoord;

out vec4 fColor;

void main()
{
    vec4 SplatmapColor = texture(uSplatmapTex, vHeightCoord);

    vec3 normalMap = (texture(uTerrainNormal, vec3(vTexCoord, 0.0)) * SplatmapColor.r  + texture(uTerrainNormal, vec3(vTexCoord, 1.0)) * SplatmapColor.g + texture(uTerrainNormal, vec3(vTexCoord, 2.0)) * SplatmapColor.b).rgb;

    vec4 vFinalTexColor;
    float val = SplatmapColor.a;
    if(val >= 0.01f)
    {
        vec3 color;
        if(val < 0.5f)
            color = mix(vec3(1, 0, 0), vec3(0, 1, 0), val * 2);
        else
            color = mix(vec3(0, 1, 0), vec3(0, 0, 1), (val - 0.5) * 2);
        vFinalTexColor = vec4(color, 1.0f);
    }
    else
        vFinalTexColor = texture(uTerrainTex, vec3(vTexCoord, 0.0)) * SplatmapColor.r  + texture(uTerrainTex, vec3(vTexCoord, 1.0)) * SplatmapColor.g + texture(uTerrainTex, vec3(vTexCoord, 2.0)) * SplatmapColor.b;

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

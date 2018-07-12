uniform mat4 uView;
uniform bool uDrawFlowMap;

uniform samplerCube uCubeMap;
uniform sampler2D uNormalMap1;
uniform sampler2D uNormalMap2;
uniform sampler2D uFlowMap;

uniform float uElapsedTime1;
uniform float uElapsedTime2;
uniform float uLerpFactor;

in vec4 vLightViewPos;
in vec4 vPosition;
in vec2 vTexCoords;
in float vAlpha;

out vec4 fragCol;

void main(){
    vec2 flowmap = texture(uFlowMap, vTexCoords).rg * 2.0f - 1.0f;

    // Slow down the flow
    flowmap /= 100.0f;

    vec2 scaledTexCoords = vTexCoords * 15.0f;

    vec2 flownTexCoords1 = flowmap * uElapsedTime1 + scaledTexCoords;
    vec2 flownTexCoords2 = flowmap * uElapsedTime2 + scaledTexCoords;
    //    vec2 flownTexCoords = vTexCoords;
    vec4 normal1 = texture(uNormalMap1, flownTexCoords1);
    vec4 normal2 = texture(uNormalMap2, flownTexCoords2);

    vec4 normal = mix(normal2, normal1, uLerpFactor);

    // Get reflection from cubemap
    vec4 incidentVector = vPosition;
    vec4 reflectedVec = reflect(incidentVector, normal);
    vec4 sampledCube = texture(uCubeMap, reflectedVec.rgb);

    vec4 lightVec = vLightViewPos - vPosition;
    lightVec = normalize(lightVec);
    float lambertian = dot(normal, lightVec);

    if(uDrawFlowMap)
        fragCol = texture(uFlowMap, vTexCoords);
    else
        fragCol = sampledCube /** lambertian*/;
    fragCol.a = vAlpha;
}

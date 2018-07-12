uniform sampler2D uNormalMap1;
uniform samplerCube uCubeMap;

in vec4 vPosition;
in vec2 vTexCoords;

out vec4 fOutput;

void main()
{
    vec4 normal = texture(uNormalMap1, vTexCoords);

    vec4 incidentVector = vPosition;
    vec4 reflectedVec = reflect(incidentVector, normal);
    vec4 sampledCube = texture(uCubeMap, reflectedVec.rgb);

    sampledCube.a = 0.7;

    fOutput = sampledCube;
}

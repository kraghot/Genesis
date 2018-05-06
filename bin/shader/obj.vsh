uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel; // assuming uniform scaling

in vec3 aPosition;
in vec3 aNormal;
in vec4 aColor;
in vec3 aTangent;
in vec2 aTexCoord;
in float aSlopeY;
in vec2 aHeightCoord;

out vec3 vWorldPosition;
out vec3 vNormal;
out vec4 vColor;
out vec3 vTangent;
out vec2 vTexCoord;
out float vSlopeY;
out vec2 vHeightCoord;

void main()
{
    vWorldPosition = vec3(uModel * vec4(aPosition, 1));
    vNormal = mat3(uModel) * aNormal;
    vColor = aColor;
    vTangent = mat3(uModel) * aTangent;
    vTexCoord = aTexCoord;
    vSlopeY = aSlopeY;
    vHeightCoord = aHeightCoord;

    gl_Position = uProj * uView * vec4(vWorldPosition, 1);
}

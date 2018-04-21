uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel; // assuming uniform scaling
uniform sampler2D uTexDisplacement;

in vec3 aPosition;
in vec3 aNormal;
in vec4 aColor;
in vec3 aTangent;
in vec2 aTexCoord;

out vec3 vWorldPosition;
out vec3 vNormal;
out vec4 vColor;
out vec3 vTangent;
out vec2 vTexCoord;

void main()
{
    float heightOffset = texture2D(uTexDisplacement, aTexCoord).x * 4.0f;
    vec4 hPosition = vec4(aPosition, 1.0f);
    hPosition.y += heightOffset;
    vWorldPosition = vec3(uModel * hPosition);
    vNormal = mat3(uModel) * aNormal;
    vColor = aColor;
    vTangent = mat3(uModel) * aTangent;
    vTexCoord = aTexCoord;

    gl_Position = uProj * uView * vec4(vWorldPosition, 1);
}

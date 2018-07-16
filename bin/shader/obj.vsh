uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel; // assuming uniform scaling
uniform sampler2D uTexDisplacement;

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
    // Example implementation of perturbation. Unused
    // vec2 perturbedTexCoords = aTexCoord + (0.2f * noise2(aPosition.x * 1000 + aPosition.y));
    float heightOffset = texture(uTexDisplacement, aHeightCoord).r;
    vec4 hPosition = vec4(aPosition, 1.0f);
    hPosition.y = heightOffset;

    // Make the extra vertices of a terrain "disappear" (just don't look from below :P)
    if(heightOffset < 0.02)
        vWorldPosition = vec3(0, 0, 0);
    else
        vWorldPosition = vec3(uModel * hPosition);
    vNormal = mat3(uModel) * aNormal;
    vColor = aColor;
    vTangent = mat3(uModel) * aTangent;
    vTexCoord = aTexCoord;
    vSlopeY = aSlopeY;
    vHeightCoord = aHeightCoord;

    gl_Position = uProj * uView * vec4(vWorldPosition, 1);
}

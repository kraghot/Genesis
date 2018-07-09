uniform mat4 uView;
uniform mat4 uProj;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;
uniform int uHeightmapDim;

in vec2 aPosition;

out vec4 vLightViewPos;
out vec4 vPosition;
out vec2 vTexCoords;

void main()
{
    vLightViewPos = uView * vec4(uLightPos, 1.0f);
    vTexCoords = aPosition * 15;
    vec2 extent = aPosition -  0.5;
    extent *= (1.5 * uHeightmapDim);
    vec4 worldPos = vec4(extent.x, 10, extent.y, 1.0);
    vPosition = uView * worldPos;
    gl_Position = uProj * uView * worldPos;
}

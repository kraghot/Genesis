uniform mat4 uView;
uniform mat4 uProj;
uniform vec3 uLightPos;

in vec3 aPosition;
in vec2 aTexCoords;

out vec4 vLightViewPos;
out vec4 vPosition;
out vec2 vTexCoords;

void main()
{
    vLightViewPos = uView * vec4(uLightPos, 1.0f);
    vTexCoords = aTexCoords;
    vec4 worldPos = vec4(aPosition, 1.0);
    vPosition = uView * worldPos;
    gl_Position = uProj * uView * worldPos;
}

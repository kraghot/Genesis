uniform mat4 uView;
uniform mat4 uProj;

in vec2 aPosition;
in vec4 vLightViewPos;

out vec4 vPosition;
out vec2 vTexCoords;

void main()
{
    vec2 extent = aPosition -  0.5;
    extent *= (10000);
    vTexCoords = aPosition * 100;
    vec4 worldPos = vec4(extent.x, 9, extent.y, 1.0);
    vPosition = uView * worldPos;
    gl_Position = uProj * uView * worldPos;
}

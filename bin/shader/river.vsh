uniform mat4 uView;
uniform mat4 uProj;
uniform vec3 uLightPos;
uniform sampler2D uTexDisplacement;
uniform sampler2D uTexRainFlow;

in vec3 aPosition;
in vec2 aHeightCoord;

out vec4 vLightViewPos;
out vec4 vPosition;
out vec2 vTexCoords;

void main()
{
    vLightViewPos = uView * vec4(uLightPos, 1.0f);
    vTexCoords = aHeightCoord;
    float displacement = texture(uTexDisplacement, vTexCoords).r;
    vec4 worldPos;
    float flow = texture(uTexRainFlow, vTexCoords).r;
    if(flow > 0.9)
        flow = (flow - 0.9) * 10.0;
    else
        flow = 0;

    float offset = mix(-0.1, 0.1, flow);
    worldPos = vec4(aPosition.x, displacement + offset, aPosition.z, 1.0);
    vPosition = uView * worldPos;
    gl_Position = uProj * uView * worldPos;
}

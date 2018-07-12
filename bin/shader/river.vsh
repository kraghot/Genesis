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
out float vAlpha;

void main()
{
    vLightViewPos = uView * vec4(uLightPos, 1.0f);
    vTexCoords = aHeightCoord;

    float displacement;
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 4; j++)
        {
            vec2 offsetCoords = vTexCoords + vec2(i, j);
            displacement += texture(uTexDisplacement, offsetCoords).r;
        }
    displacement /= 16;
    vec4 worldPos;
    float flow = texture(uTexRainFlow, vTexCoords).r;
    if(flow > 0.8)
        flow = (flow - 0.8) * 20.0;
    else
        flow = 0;

    float offset = mix(-0.1, 0.1, flow);
    vAlpha = (offset + 0.1) * 4;
    worldPos = vec4(aPosition.x, displacement + offset, aPosition.z, 1.0);
    vPosition = uView * worldPos;
    gl_Position = uProj * uView * worldPos;
}

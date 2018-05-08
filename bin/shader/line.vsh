uniform mat4 uView;
uniform mat4 uProj;

in vec3 aPosition;

void main()
{
    gl_Position = uProj * uView * vec4(aPosition, 1);
}

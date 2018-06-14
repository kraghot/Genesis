uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel;

in vec3 aPosition;

void main()
{
    gl_Position = uProj * uView * uModel * vec4(aPosition, 1);
}

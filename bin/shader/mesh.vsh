in vec3 aPosition;
in vec4 uM1;
in vec4 uM2;
in vec4 uM3;
in vec4 uM4;

out vec3 vPosition;

uniform mat4 uView;
uniform mat4 uProj;


void main() {
    mat4 model;
    model[0][0] = uM1.x;
    model[0][1] = uM1.y;
    model[0][2] = uM1.z;
    model[0][3] = uM1.w;

    model[1][0] = uM2.x;
    model[1][1] = uM2.y;
    model[1][2] = uM2.z;
    model[1][3] = uM2.w;

    model[2][0] = uM3.x;
    model[2][1] = uM3.y;
    model[2][2] = uM3.z;
    model[2][3] = uM3.w;

    model[3][0] = uM4.x;
    model[3][1] = uM4.y;
    model[3][2] = uM4.z;
    model[3][3] = uM4.w;


    vPosition = aPosition;
    gl_Position = uProj * uView * model * vec4(aPosition, 1.0);
}

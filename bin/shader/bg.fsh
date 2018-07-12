in vec2 vPosition;

uniform samplerCube uTexture;

uniform mat4 uInvProj;
uniform mat4 uInvView;

out vec3 fColor;

void main() {

    vec4 near = uInvProj * vec4(vPosition * 2 - 1, 0, 1);
    vec4 far = uInvProj * vec4(vPosition * 2 - 1, 0.5, 1);

    near /= near.w;
    far /= far.w;

    near = uInvView * near;
    far = uInvView * far;

    fColor = pow(texture(uTexture, (far - near).xyz), vec4(1/2.224)).rgb;
}

in vec3 vPosition;

out vec3 fColor;

void main() {
    fColor = vPosition * .5 + .5;
}

//in vec3 vPosition;

//out vec3 fColor;

//void main() {
//    fColor = vPosition * .5 + .5;
//}

uniform vec3 uLightPos;
uniform vec3 uCamPos;

uniform sampler2D uTexColor;
uniform sampler2D uTexNormal;

in vec3 vWorldPosition;
in vec3 vNormal;
in vec3 vTangent;
in vec2 vTexCoord;

out vec3 fColor;

void main()
{
    // material settings
    float shininess = 8;
    float ka = 0.1;
    float kd = 0.7;
    float ks = 0.3;

    // read textures
    vec3 color = texture(uTexColor, vTexCoord).rgb;
    vec3 normalMap = texture(uTexNormal, vTexCoord).rgb;

    // build local coordinate system
    vec3 L = normalize(uLightPos - vWorldPosition);
    vec3 V = normalize(uCamPos - vWorldPosition);
    vec3 H = normalize(V + L);
    vec3 N = normalize(vNormal);
    vec3 T = normalize(vTangent);
    vec3 B = normalize(cross(T, N));
    mat3 tbn = mat3(T, B, N);

    // apply normal mapping
    normalMap.xy = normalMap.xy * 2 - 1;
    N = normalize(tbn * normalMap);

    // blinn-phong model
    float albedo = ka;
    float diffuse = kd * max(0, dot(N, L));
    float specular = ks * pow(max(0, dot(N, H)), shininess);
    fColor = pow(color * (albedo + diffuse + specular), vec3(1/2.224));
}

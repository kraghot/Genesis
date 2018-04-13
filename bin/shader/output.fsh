uniform sampler2DRect uTexture;

out vec3 fColor;

// pseudo-random generator
uint wang_hash(uint seed)
{
    seed = (seed ^ 61u) ^ (seed >> 16u);
    seed *= 9u;
    seed = seed ^ (seed >> 4u);
    seed *= 0x27D4EB2Du;
    seed = seed ^ (seed >> 15u);
    return seed;
}

float wang_float(uint hash)
{
    return hash / float(0x7FFFFFFF) / 2.0;
}

void main()
{
    vec3 color = texture(uTexture, gl_FragCoord.xy).rgb;

    // gamma correction
    color = pow(color, vec3(1 / 2.224));

    // dithering
    uint seed = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * 8096u;
    float r = wang_float(wang_hash(seed * 3u + 0u));
    float g = wang_float(wang_hash(seed * 3u + 1u));
    float b = wang_float(wang_hash(seed * 3u + 2u));
    vec3 random = vec3(r, g, b);
    color += (random - 0.5) / 256.0;

    // output
    fColor = color;
}

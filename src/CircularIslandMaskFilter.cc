#include "CircularIslandMaskFilter.hh"

CircularIslandMaskFilter::CircularIslandMaskFilter(float innerRadius, float outerRadius, PerlinNoiseGenerator &perlin):
    mPerlin(perlin),
    mInnerRadius(innerRadius / 2.0f),
    mOuterRadius(outerRadius / 2.0f)
{

}

double CircularIslandMaskFilter::filter(double x, double y, double val)
{
    glm::vec2 center(0.5, 0.5);
    glm::vec2 directionVector = glm::vec2(x, y) - center;
    glm::vec2 normalizedDir = glm::normalize(directionVector);

    double perlinSample = mPerlin.noise(normalizedDir.x, normalizedDir.y, 0.6);

    float length = glm::length(directionVector);
    if(length < mInnerRadius)
        return val;
    else if(length < mOuterRadius)
    {
        float diffLength = length - mInnerRadius;
        const float lerpSpace = mOuterRadius - mInnerRadius;
        return val * (1 - (diffLength / lerpSpace));
    }
    else
        return 0;

}

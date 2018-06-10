#include "IslandMaskGenerator.hh"

IslandMaskGenerator::IslandMaskGenerator(glm::vec2 innerSquare, glm::vec2 outerSquare, unsigned seed):
    mPerlin(seed),
    mInnerSquare(innerSquare),
    mOuterSquare(outerSquare)
{
    mRanges = glm::vec2(outerSquare - innerSquare);
    mRelRanges = mRanges / mOuterSquare;
    mRelRanges /= 2.0;
}

IslandMaskGenerator::IslandMaskGenerator(glm::vec2 innerSquare, glm::vec2 outerSquare, PerlinNoiseGenerator &perlin):
    mPerlin(perlin),
    mInnerSquare(innerSquare),
    mOuterSquare(outerSquare)
{
    mRanges = glm::vec2(outerSquare - innerSquare);
    mRelRanges = mRanges / mOuterSquare;
    mRelRanges /= 2.0;
}

double IslandMaskGenerator::noise(double x, double y, double z)
{
    double val = 0.0;
    if(x < mRelRanges.x && y < mRelRanges.y)
    {
        double xNoise = GetRandomFactor(0, y * 10.0, z);
        double yNoise = GetRandomFactor(x * 10.0, 0, z);
        double currentRelPosX = x / mRelRanges.x;
        double currentRelPosY = y / mRelRanges.y;

        val = Get2DLerp(currentRelPosX, xNoise, currentRelPosY, yNoise);
    }
    else if(x < mRelRanges.x && y > (1.0 - mRelRanges.y))
    {
        double xNoise = GetRandomFactor(0, y * 10.0, z);
        double yNoise = GetRandomFactor(x * 10.0, 1, z);
        double currentRelPosX = x / mRelRanges.x;
        double currentRelPosY = (1.0 - y) / mRelRanges.y;

        val = Get2DLerp(currentRelPosX, xNoise, currentRelPosY, yNoise);
    }
    else if(x > (1.0 - mRelRanges.x) && y < mRelRanges.y)
    {
        double xNoise = GetRandomFactor(1, y * 10.0, z);
        double yNoise = GetRandomFactor(x * 10.0, 0, z);
        double currentRelPosX = (1.0 - x) / mRelRanges.x;
        double currentRelPosY = y / mRelRanges.y;

        val = Get2DLerp(currentRelPosX, xNoise, currentRelPosY, yNoise);
    }
    else if(x > (1.0 - mRelRanges.x) && y > (1.0 - mRelRanges.y))
    {
        double xNoise = GetRandomFactor(1, y * 10.0, z);
        double yNoise = GetRandomFactor(x * 10.0, 1, z);
        double currentRelPosX = (1.0 - x) / mRelRanges.x;
        double currentRelPosY = (1.0 - y) / mRelRanges.y;

        val = Get2DLerp(currentRelPosX, xNoise, currentRelPosY, yNoise);
    }
    else if(x < mRelRanges.x)
    {
        double noise = GetRandomFactor(0, y * 10, z);

        double currentRelPos = x / mRelRanges.x;

        val = Get1DLerp(currentRelPos, noise);
    }
    else if(x > (1.0 - mRelRanges.x))
    {
        double noise = GetRandomFactor(1.0, y * 10.0, z);

        double currentRelPos = (1.0 - x) / mRelRanges.x;

        val = Get1DLerp(currentRelPos, noise);
    }
    else if(y < mRelRanges.y)
    {
        double noise = GetRandomFactor(x * 10.0, 0, z);

        double currentRelPos = y / mRelRanges.y;

        val = Get1DLerp(currentRelPos, noise);
    }
    else if(y > (1.0 - mRelRanges.y))
    {
        double noise = GetRandomFactor(x * 10.0, 1.0, z);

        double currentRelPos = (1.0 - y) / mRelRanges.y;

        val = Get1DLerp(currentRelPos, noise);
    }

    return val;
}

#ifndef ISLANDMASKGENERATOR_H
#define ISLANDMASKGENERATOR_H

#include "FilterGenerator.hh"
#include "PerlinNoiseGenerator.hh"
#include <glm/common.hpp>

class IslandMaskGenerator : public FilterGenerator
{
public:
    IslandMaskGenerator(glm::vec2 innerSquare, glm::vec2 outerSquare, unsigned seed);
    IslandMaskGenerator(glm::vec2 innerSquare, glm::vec2 outerSquare, PerlinNoiseGenerator& perlin);

    double filter(double x, double y, double input);

private:
    inline double GetRandomFactor(double x, double y, double z)
    {
        double noise = mPerlin.noise(x, y, z);
        noise /= 2.0;
        noise += 0.5;
        return noise;
    }

    inline double Get1DLerp(double currPos, double noise)
    {
        if(currPos > noise)
            return 1.0;

        double scaled = currPos / noise;
        return scaled;
    }

    inline double Get2DLerp(double currPosX, double xNoise, double currPosY, double yNoise)
    {
        if(currPosX > xNoise && currPosY > yNoise)
            return 1.0;

        double scaledX = currPosX / xNoise;
        double scaledY = currPosY / yNoise;
        if(scaledX > 1.0) scaledX = 1.0;
        if(scaledY > 1.0) scaledY = 1.0;

//        double flippedX = 1.0 - scaledX;
//        double flippedY = 1.0 - scaledY;

//        double average = (flippedX * flippedY);
//        average = 1.0 - average;
//        return average - 1.0;
//        return ((scaledX + scaledY) / 2.0f) - 1.0;
        return scaledX * scaledY;
    }

    PerlinNoiseGenerator mPerlin;
    glm::vec2 mInnerSquare;
    glm::vec2 mOuterSquare;
    glm::vec2 mRanges;
    glm::vec2 mRelRanges;
};

#endif // ISLANDMASKGENERATOR_H

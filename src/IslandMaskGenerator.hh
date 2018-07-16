#ifndef ISLANDMASKGENERATOR_H
#define ISLANDMASKGENERATOR_H

#include "FilterGenerator.hh"
#include "PerlinNoiseGenerator.hh"
#include <glm/common.hpp>

/**
 * @brief The SquareIslandMaskGenerator class is a filter that generates square shaped islands
 */
class SquareIslandMaskGenerator : public FilterGenerator
{
public:
    /**
     * @param innerSquare is the size of the square form the center of the heightmap where there should be no fall-off
     * @param outerSquare is the size of the square from the center where all point should reach 0
     * @param seed is the seed for the random component using PerlinNoiseGenerator
     */
    SquareIslandMaskGenerator(glm::vec2 innerSquare, glm::vec2 outerSquare, unsigned seed);
    SquareIslandMaskGenerator(glm::vec2 innerSquare, glm::vec2 outerSquare, PerlinNoiseGenerator& perlin);

    /**
     * Filters the height of the terrain by returning 1 when in InnerSquare and falling of between
     * inner Square and OuterSquare according to the random factor given by Perlin. 0 Otherwise.
     */
    double filter(double x, double y, double input);

private:
    inline double GetRandomFactor(double x, double y, double z)
    {
        double noise = mPerlin.noise(x, y, z);
        noise /= 2.0;
        noise += 0.5;
        return noise;
    }

    /**
     * @brief Get1DLerp returns 1D interpolation used for sides
     */
    inline double Get1DLerp(double currPos, double noise)
    {
        if(currPos > noise)
            return 1.0;

        double scaled = currPos / noise;
        return scaled;
    }

    /**
     * @brief Get2DLerp returns 2d interpolation used for corners
     */
    inline double Get2DLerp(double currPosX, double xNoise, double currPosY, double yNoise)
    {
        if(currPosX > xNoise && currPosY > yNoise)
            return 1.0;

        double scaledX = currPosX / xNoise;
        double scaledY = currPosY / yNoise;
        if(scaledX > 1.0) scaledX = 1.0;
        if(scaledY > 1.0) scaledY = 1.0;
        return scaledX * scaledY;
    }

    PerlinNoiseGenerator mPerlin;
    glm::vec2 mInnerSquare;
    glm::vec2 mOuterSquare;
    glm::vec2 mRanges;
    glm::vec2 mRelRanges;
};

#endif // ISLANDMASKGENERATOR_H

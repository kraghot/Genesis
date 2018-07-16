#ifndef CIRCULARISLANDMASKFILTER_HH
#define CIRCULARISLANDMASKFILTER_HH

#include "FilterGenerator.hh"
#include "PerlinNoiseGenerator.hh"
#include <glm/glm.hpp>

/**
 * @brief The CircularIslandMaskFilter class will filter the island so that is has a circular fall-off
 */
class CircularIslandMaskFilter : public FilterGenerator
{
public:
    /**
     * @param innerRadius is the radius until which there should be no fall-off [0, 1]
     * @param outerRadius is the radius until which the house should be 0 [0, 1]
     * @param perlin is a perlin generator for a random element
     */
    CircularIslandMaskFilter(float innerRadius, float outerRadius, PerlinNoiseGenerator& perlin);
    double filter(double x, double y, double val);

private:
    /**
     * @brief vectorAngle gets an angle in radians from a 2D vector
     * @warning unused
     */
    float vectorAngle(glm::vec2 vec) {
        if (vec.x == 0) // special cases
            return (vec.y > 0)? 90
                : (vec.y == 0)? 0
                : 270;
        else if (vec.y == 0) // special cases
            return (vec.x >= 0)? 0
                : 180;
        float ret = atanf((float)vec.y/vec.x);
        if (vec.x < 0 && vec.y < 0) // quadrant Ⅲ
            ret = M_PI + ret;
        else if (vec.x < 0) // quadrant Ⅱ
            ret = M_PI + ret; // it actually substracts
        else if (vec.y < 0) // quadrant Ⅳ
            ret = (1.5 * M_PI) + ((0.5 * M_PI) + ret); // it actually substracts
        return ret;
    }

    PerlinNoiseGenerator& mPerlin;
    float mInnerRadius;
    float mOuterRadius;
};

#endif // CIRCULARISLANDMASKFILTER_HH

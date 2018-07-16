#ifndef DIAMONDSQUARENOISEGENERATOR_HH
#define DIAMONDSQUARENOISEGENERATOR_HH

#include "NoiseGenerator.hh"
#include <vector>

class DiamondSquareNoiseGenerator : public NoiseGenerator
{
public:
    DiamondSquareNoiseGenerator(const unsigned seed, const unsigned dimX, const unsigned dimY, const unsigned featureSize);
    /**
     * @brief DiamondSquareNoiseGenerator initializes and precalculates Diamond-Square noise
     * @param dimX is the X dimension of the heightmap
     * @param dimY is the Y dimension of the heightmap
     * @param featureSize is the width between the initial random values
     */
    DiamondSquareNoiseGenerator(const unsigned dimX, const unsigned dimY, const unsigned featureSize);
    /**
     * @brief noise returns Diamond-Square noise at location
     * @param x is limited to [0, 1]
     * @param y is limited to [0, 1]
     * @param z is ignored
     */
    double noise(double x, double y, double z);

private:
    void DiamondSquare(int stepsize, double scale);
    void DiamondStep(unsigned x, unsigned y, unsigned size, float value);
    void SquareStep(unsigned x, unsigned y, unsigned size, float value);

    /**
     * @brief GetValue retrieves the height, but checks the bounds
     */
    float GetValue(unsigned x, unsigned y);
    /// Set with wrapping
    void SetValue(unsigned x, unsigned y, float value);
    /// @warning Returns clamped between zero and X only
    unsigned TerrainWrap(unsigned value);
    /// @brief Returns a random value between [0, 1]
    float fRand();

    unsigned mDimX;
    unsigned mDimY;
    std::vector<float> mData;
    unsigned mSeed;
};

#endif // DIAMONDSQUARENOISEGENERATOR_HH

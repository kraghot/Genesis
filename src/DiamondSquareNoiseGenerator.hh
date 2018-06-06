#ifndef DIAMONDSQUARENOISEGENERATOR_HH
#define DIAMONDSQUARENOISEGENERATOR_HH

#include "NoiseGenerator.hh"
#include <vector>

class DiamondSquareNoiseGenerator : public NoiseGenerator
{
public:
    DiamondSquareNoiseGenerator(const unsigned seed, const unsigned dimX, const unsigned dimY, const unsigned featureSize);
    DiamondSquareNoiseGenerator(const unsigned dimX, const unsigned dimY, const unsigned featureSize);
    double noise(double x, double y, double z);

private:
    void DiamondSquare(int stepsize, double scale);
    void DiamondStep(unsigned x, unsigned y, unsigned size, float value);
    void SquareStep(unsigned x, unsigned y, unsigned size, float value);

    // Get with wrapping
    float GetValue(unsigned x, unsigned y);
    // Set with wrapping
    void SetValue(unsigned x, unsigned y, float value);
    /// @warning Returns clamped between zero and X only
    unsigned TerrainWrap(unsigned value);
    float fRand();
    unsigned mDimX;
    unsigned mDimY;
    std::vector<float> mData;
    unsigned mSeed;
};

#endif // DIAMONDSQUARENOISEGENERATOR_HH

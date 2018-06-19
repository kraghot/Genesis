#include "DiamondSquareNoiseGenerator.hh"
#include <stdexcept>
#include <cmath>

#define LOC(a, b) b * mDimY + a

DiamondSquareNoiseGenerator::DiamondSquareNoiseGenerator(const unsigned seed, const unsigned dimX, const unsigned dimY, const unsigned featureSize)
    : mSeed(seed)
{
    srand(seed);
    // Forward to the other constructor
    DiamondSquareNoiseGenerator(dimX, dimY, featureSize);
}

DiamondSquareNoiseGenerator::DiamondSquareNoiseGenerator(const unsigned dimX, const unsigned dimY, const unsigned featureSize):
    mDimX(dimX),
    mDimY(dimY)
{
    // Check if feature size is power of 2
    if((featureSize & (featureSize - 1)) != 0)
        throw std::runtime_error("Terrain must be power of 2 and square");

    mData.resize(dimX * dimY);


    for(auto y = 0u; y < dimY; y+=featureSize)
    {
        for(auto x = 0u; x < dimX; x+=featureSize)
        {
            SetValue(x, y, fRand());
        }
    }

    int sampleSize = featureSize;

    double scale = 1.0;

    while (sampleSize > 1)
    {

        DiamondSquare(sampleSize, scale);

        sampleSize /= 2;
        scale /= 2.0f;
    }
}

double DiamondSquareNoiseGenerator::noise(double x, double y, double z)
{
    unsigned locX, locY;
    locX = std::floor(x * mDimX);
    locY = std::floor(y * mDimY);

    return mData.at(LOC(locX, locY));
}

void DiamondSquareNoiseGenerator::DiamondSquare(int stepsize, double scale)
{

    int halfstep = stepsize / 2;

    for (int y = halfstep; y < mDimY + halfstep; y += stepsize)
    {
        for (int x = halfstep; x < mDimX + halfstep; x += stepsize)
        {
            SquareStep(x, y, stepsize, fRand() * scale);
        }
    }

    for (int y = 0; y < mDimY; y += stepsize)
    {
        for (int x = 0; x < mDimX; x += stepsize)
        {
            DiamondStep(x + halfstep, y, stepsize, fRand() * scale);
            DiamondStep(x, y + halfstep, stepsize, fRand() * scale);
        }
    }

}

void DiamondSquareNoiseGenerator::DiamondStep(unsigned x, unsigned y, unsigned size, float value)
{
    unsigned hSize = size / 2;
    float s1 = GetValue(x - hSize,  y);
    float s2 = GetValue(x + hSize,  y);
    float s3 = GetValue(x,          y - hSize);
    float s4 = GetValue(x,          y + hSize);
    SetValue(x, y, value + ((s1 + s2 + s3 + s4) / 4.0f));
}

void DiamondSquareNoiseGenerator::SquareStep(unsigned x, unsigned y, unsigned size, float value)
{
    unsigned hSize = size / 2;
    float s1 = GetValue(x - hSize, y - hSize);
    float s2 = GetValue(x + hSize, y - hSize);
    float s3 = GetValue(x - hSize, y + hSize);
    float s4 = GetValue(x + hSize, y + hSize);
    SetValue(x, y, value + ((s1 + s2 + s3 + s4) / 4.0f));
}

float DiamondSquareNoiseGenerator::GetValue(unsigned x, unsigned y)
{
    unsigned sX = TerrainWrap(x);
    unsigned sY = TerrainWrap(y);
    return mData.at(LOC(sX, sY));
}

void DiamondSquareNoiseGenerator::SetValue(unsigned x, unsigned y, float value)
{
    unsigned sX = TerrainWrap(x);
    unsigned sY = TerrainWrap(y);
    mData.at(LOC(sX, sY)) = value;
}

unsigned DiamondSquareNoiseGenerator::TerrainWrap(unsigned value)
{
    return value % mDimX;
}

float DiamondSquareNoiseGenerator::fRand()
{
    // Return uniform between -1 and 1
    return (double) rand() / RAND_MAX ;
}

#undef LOC

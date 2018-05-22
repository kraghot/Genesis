#include "DiamondSquareNoiseGenerator.hh"
#include <stdexcept>
#include <cmath>

#define LOC(a, b) b * mDimY + a

DiamondSquareNoiseGenerator::DiamondSquareNoiseGenerator(const unsigned dimX, const unsigned dimY, const unsigned featureSize):
    mDimX(dimX),
    mDimY(dimY)
{
    // Check if terrain power of 2 and square
//    if(((dimX & (dimX - 1)) != 0) || (dimX != dimY))
//        throw std::runtime_error("Terrain must be power of 2 and square");

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
    unsigned sX = TerrainClamp(x);
    unsigned sY = TerrainClamp(y);
    return mData.at(LOC(sX, sY));
}

void DiamondSquareNoiseGenerator::SetValue(unsigned x, unsigned y, float value)
{
    unsigned sX = TerrainClamp(x);
    unsigned sY = TerrainClamp(y);
    mData.at(LOC(sX, sY)) = value;
}

float DiamondSquareNoiseGenerator::TerrainClamp(float value)
{
    if(value < 0)
        return 0;
    if(value > mDimX -1)
        return mDimX - 1;
    return value;
}

float DiamondSquareNoiseGenerator::fRand()
{
    return (double) rand() / RAND_MAX;
}

#undef LOC

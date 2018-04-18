#ifndef PERLINNOISE_HH
#define PERLINNOISE_HH

#include <vector>
#include "NoiseGenerator.hh"

class PerlinNoiseGenerator : public NoiseGenerator
{
public:
    PerlinNoiseGenerator();
    PerlinNoiseGenerator(unsigned int seed);
    double noise(double x, double y, double z) override;

private:
    double fade(double t);
    double lerp(double t, double a, double b);
    double grad(int hash, double x, double y, double z);

    std::vector<int> permutations;
};

#endif // PERLINNOISE_HH

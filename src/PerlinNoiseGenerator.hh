#ifndef PERLINNOISE_HH
#define PERLINNOISE_HH

#include <vector>
#include "NoiseGenerator.hh"

/**
 * @brief The PerlinNoiseGenerator class is used for generating Perlin noise. The data is not precalculated.
 */
class PerlinNoiseGenerator : public NoiseGenerator
{
public:
    /**
     * @brief Initialize the generator with the seed given from the original paper
     */
    PerlinNoiseGenerator();
    /**
     * @brief Initialize with custom seed
     */
    PerlinNoiseGenerator(unsigned int seed);
    /**
     * @brief noise returns perlin noise at location
     * @param x is not limited to [0, 1]
     * @param y is not limited to [0, 1]
     * @param z has an effect the final result
     */
    double noise(double x, double y, double z) override;

private:
    double fade(double t);
    double lerp(double t, double a, double b);
    double grad(int hash, double x, double y, double z);

    std::vector<int> permutations;
};

#endif // PERLINNOISE_HH

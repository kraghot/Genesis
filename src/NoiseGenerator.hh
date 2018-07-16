#ifndef NOISEGENERATOR_H
#define NOISEGENERATOR_H


class NoiseGenerator
{
public:
    /**
     * @brief noise returns random noise at certain location
     * @param x is between 0 and 1. Some generators do not have this limitation
     * @param y is between 0 and 1. Some generators do not have this limitation
     * @param z sometimes unused
     * @return noise at location
     */
    virtual double noise(double x, double y, double z)=0;

};

#endif // NOISEGENERATOR_H

#ifndef NOISEGENERATOR_H
#define NOISEGENERATOR_H


class NoiseGenerator
{
public:
    virtual double noise(double x, double y, double z)=0;

};

#endif // NOISEGENERATOR_H

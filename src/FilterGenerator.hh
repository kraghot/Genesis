#ifndef FILTERGENERATOR_HH
#define FILTERGENERATOR_HH


class FilterGenerator
{
public:
    virtual double filter(double x, double y, double val) = 0;
};

#endif // FILTERGENERATOR_HH

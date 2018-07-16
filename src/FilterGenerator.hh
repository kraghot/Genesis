#ifndef FILTERGENERATOR_HH
#define FILTERGENERATOR_HH


class FilterGenerator
{
public:
    /**
     * @brief filter filter the current position
     * @param x is the normalized x coordinate [0, 1]
     * @param y is the normalized y coordinate [0, 1]
     * @param val is the current height
     * @return filtered height at location
     */
    virtual double filter(double x, double y, double val) = 0;
};

#endif // FILTERGENERATOR_HH

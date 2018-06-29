#ifndef LINEARBLUR_HH
#define LINEARBLUR_HH

#include <vector>

#define LOCV(i, j) (j % height) * width + (i % width)

template<typename T>
void LinearBlur(std::vector<T>& data, unsigned width, int r)
{
    std::vector<T> blurred(data.size());
    unsigned height = data.size() / width;
    for(auto j = 0u; j < height; j++)
    {
        for(auto i = 0u; i < width; i++)
        {
            T sum = {};
            unsigned samples = 0;
            for(int offsetX = -r; offsetX <= r; offsetX++)
            {
                for(int offsetY = -r; offsetY <= r; offsetY++)
                {
                    sum += data[LOCV(i + offsetX, j + offsetY)];
                    samples++;
                }
            }
            sum /= samples;
            blurred[LOCV(i, j)] = sum;
        }
    }

    std::swap(data, blurred);
}

#undef LOCV

#endif // LINEARBLUR_HH

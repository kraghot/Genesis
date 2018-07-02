#ifndef LINEARBLUR_HH
#define LINEARBLUR_HH

#include <vector>

#define LOCV(i, j) j * width + i

template<typename T>
void LinearBlur(std::vector<T>& data, int width, int r)
{
    std::vector<T> blurred(data.size());
    int height = data.size() / width;
    for(auto j = r; j < height - r; j++)
    {
        for(auto i = r; i < width - r; i++)
        {
            T sum = T(0);
            float samples = 0;
            for(int offsetX = -r; offsetX <= r; offsetX++)
            {
                for(int offsetY = -r; offsetY <= r; offsetY++)
                {
                    auto index = LOCV((i + offsetX), (j + offsetY));
                    sum += data[index];
                    samples += 1.0;
                }
            }
            sum /= samples;
            auto index = LOCV(i, j);
            blurred[index] = sum;
        }
    }

    std::swap(data, blurred);
}

#undef LOCV

#endif // LINEARBLUR_HH

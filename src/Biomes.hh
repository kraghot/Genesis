#ifndef BIOMES_H
#define BIOMES_H

#include "MultiLayeredHeightmap.hh"


class Biomes
{
public:
    Biomes(MultiLayeredHeightmap *h);
    void randomWindDirection();
    //glm::mat4 ScanlineProjection();

    std::vector<glm::vec3> mRainMap;

private:
    //glm::vec3 randomWindDirection();
    MultiLayeredHeightmap *mHeightmap;



};

#endif //BIOMES_H

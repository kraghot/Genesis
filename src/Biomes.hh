#ifndef BIOMES_H
#define BIOMES_H

#include "MultiLayeredHeightmap.hh"


class Biomes
{
public:
    Biomes(MultiLayeredHeightmap *h);
    void scanLineConversion();
    void randomWindDirection();
    glm::mat4 ScanlineProjection();

    glm::vec3 mWindDir = {0, 0, 0};
    glm::vec3 mWindUp = {0, 1, 0};
    glm::vec3 mWindPos = {0, 0, 0};

    glm::vec3 viewVector = {0, 0, 0};
    glm::vec3 rightVector = {0, 0, 0};
    glm::vec3 upVector = {0, 0, 0};

private:
    //glm::vec3 randomWindDirection();
    MultiLayeredHeightmap *mHeightmap;

};

#endif //BIOMES_H

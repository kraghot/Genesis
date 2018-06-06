#include "Biomes.hh"

#include <random>

Biomes::Biomes(MultiLayeredHeightmap *h){
    mHeightmap = h;
}


void Biomes::randomWindDirection(){
    glm::vec3 possibleValuesX = {-mHeightmap->halfTerrainWidth/2, mHeightmap->halfTerrainWidth/2, 0}; //because x and z have the same dimensions, leaves room for otherwise

    int randIndex;

    randIndex = std::rand() % 3;
    mWindDir.x = possibleValuesX[2];

    randIndex = std::rand() % 3;
    mWindDir.z = possibleValuesX[2];

    glm::normalize(mWindDir);

    std::cout << "winddir(" << mWindDir.x << ", " << mWindDir.y << ", " << mWindDir.z << ")" << std::endl;
//    std::cout << "random Index = " << randIndex << std::endl;

   // return windDir;
    glm::mat4 ScanlineLookAt = ScanlineProjection();


}

glm::mat4 Biomes::ScanlineProjection(){

    mWindDir = {0, 0, -mHeightmap->halfTerrainWidth/2};

    mWindPos = {0, 0, mHeightmap->halfTerrainWidth/2};

    viewVector = mWindDir - mWindPos;
    float mWindLookAtDistance = glm::length(forwardVector);

    viewVector = viewVector / (float)mWindLookAtDistance;
    rightVector = glm::normalize(glm::cross(viewVector, mWindUp));
    upVector = glm::cross(rightVector, viewVector);


    glm::mat4 lookAt = glm::lookAt(viewVector,
                                   rightVector,
                                   upVector);

    return lookAt;
}

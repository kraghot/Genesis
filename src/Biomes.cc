#include "Biomes.hh"

#include <random>

Biomes::Biomes(MultiLayeredHeightmap *h){
    mHeightmap = h;
}


void Biomes::randomWindDirection(){
//    glm::vec3 possibleValuesX = {-mHeightmap->halfTerrainWidth/2, mHeightmap->halfTerrainWidth/2, 0}; //because x and z have the same dimensions, leaves room for otherwise

//    mWindDir.x = possibleValuesX[(std::rand() % 3)];

//    mWindDir.z = possibleValuesX[(std::rand() % 3)];

//    if(mWindDir.x == 0 && mWindDir.z == 0) // we do not want to look from one corner to its vertical/horizontal neighbour corner, only diagonal
//        mWindDir.z = possibleValuesX[(std::rand() % 2)];

//    glm::normalize(mWindDir);

//    std::cout << "winddir(" << mWindDir.x << ", " << mWindDir.y << ", " << mWindDir.z << ")" << std::endl;

//    glm::mat4 ScanlineLookAt = ScanlineProjection();

//    std::vector<unsigned int> possibleValuesX = {(mHeightmap->mHeightmapDimensions.x -1)/2, (mHeightmap->mHeightmapDimensions.y - 1)/2, mHeightmap->mHeightmapDimensions.y, mHeightmap->mHeightmapDimensions.x, 0};

//    mWindDir.x = possibleValuesX[(std::rand() % 5)];

//    mWindDir.z = possibleValuesX[(std::rand() % 5)];

   // mRainMap.resize(mHeightmap->mNumberOfVertices);

    unsigned int randomWindDir = rand() % 4; //0 = N->S, 1 = S->N, 2 = W->E, 3 = E->W

    double maxY = 3;
    bool higher = false;
    bool noFirstMountain = true;

    //todo: for cases where dimX != dimY we need to check that winddir hasnt 2 dimXs or dimYs
    //#define CURRPOS i*mHeightmap->mHeightmapDimensions.x + j // S->N, N->S
    #define CURRPOS i-j // E->W, W->E

    //for(unsigned int i = 0; i < mHeightmap->mHeightmapDimensions.x; i++){ // S->N
    //for(int i = mHeightmap->mHeightmapDimensions.x - 1; i >= 0; i--){ // N->S
    //for(int i = mHeightmap->mHeightmapDimensions.x * (mHeightmap->mHeightmapDimensions.x -1); i < mHeightmap->mHeightmapDimensions.x * mHeightmap->mHeightmapDimensions.x; i++){ // E->W
    for(int i = mHeightmap->mHeightmapDimensions.x * mHeightmap->mHeightmapDimensions.x - 1; i >= mHeightmap->mHeightmapDimensions.x * (mHeightmap->mHeightmapDimensions.x -1); i--){ // W->E
        if(!higher && !noFirstMountain)
            continue;

        higher = false;

        //for(unsigned int j = 0; j < mHeightmap->mHeightmapDimensions.y; j++){ // S->N
        //for(unsigned int j = 0; j < mHeightmap->mHeightmapDimensions.y; j++){ // N->S
        //for(unsigned int j = 0; j < mHeightmap->mHeightmapDimensions.y * mHeightmap->mHeightmapDimensions.y; j+=mHeightmap->mHeightmapDimensions.y){ // E->W
        for(unsigned int j = 0; j < mHeightmap->mHeightmapDimensions.y * mHeightmap->mHeightmapDimensions.y; j+= mHeightmap->mHeightmapDimensions.y){ // W->E
            //mRainMap.at(CURRPOS) = {1.f, 0.f, 0.f};

            mHeightmap->mSplatmap.at(CURRPOS) = {1.f, 0.f, 0.f};


            std::cout << "Y: " << mHeightmap->mDisplacement.at(CURRPOS) << std::endl;

            if(mHeightmap->mDisplacement.at(CURRPOS) > maxY){
                maxY = mHeightmap->mPositions.at(CURRPOS).y;
                higher = true;
                noFirstMountain = false;
                //break;
            }

//            if(mHeightmap->mPositions.at(CURRPOS) >= 30){
//                mRainLoss = 1.f;
//                goto writeRainMap;
//            }

//            else if(mHeightmap->mPositions.at(CURRPOS) >= 20)
//                mRainLoss += .75f;

//            else if(mHeightmap->mPositions.at(CURRPOS) >= 20)
//                mRainLoss += .25f;

//            if(mRainLoss <= 0)
//                goto writeRainMap;
        }
    }

   // writeRainMap:

    mHeightmap->mSplatmapTexture->bind().setData(GL_RGB, mHeightmap->mHeightmapDimensions.x, mHeightmap->mHeightmapDimensions.y, mHeightmap->mSplatmap);
    mHeightmap->mSplatmapTexture->bind().generateMipmaps();
}

glm::mat4 Biomes::ScanlineProjection(){


    mWindPos = -mWindDir;

    viewVector = mWindDir - mWindPos;
    float mWindLookAtDistance = glm::length(viewVector);

    viewVector = viewVector / (float)mWindLookAtDistance;
    rightVector = glm::normalize(glm::cross(viewVector, mWindUp));
    upVector = glm::cross(rightVector, viewVector);


    glm::mat4 lookAt = glm::lookAt(viewVector,
                                   rightVector,
                                   upVector);

    return lookAt;
}

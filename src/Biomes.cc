#include "Biomes.hh"

#include <random>

#define CURRPOS_NS i*mHeightmap->mHeightmapDimensions.x + j // S->N, N->S
#define CURRPOS_WE i-j // E->W, W->E

Biomes::Biomes(MultiLayeredHeightmap *h){
    mHeightmap = h;
}


void Biomes::randomWindDirection(){

   // mRainMap.resize(mHeightmap->mNumberOfVertices);

    std::vector<std::vector<double>> rainAmount(mHeightmap->mHeightmapDimensions.x,std::vector<double>(mHeightmap->mHeightmapDimensions.y,0));
    unsigned int x = 0, y = 0, i, j;
    bool firstIteration = true;
    unsigned int randomWindDir = rand() % 4; //0 = N->S, 1 = S->N, 2 = W->E, 3 = E->W

    switch(randomWindDir){
        case 0: goto NS;
        case 1: goto SN;
        case 2: goto WE;
        case 3: goto EW;

    }


NS:
    for(i = mHeightmap->mHeightmapDimensions.x - 1; i > 0; i--){
        for(j = 0; j < mHeightmap->mHeightmapDimensions.y; j++){

            if(firstIteration)
                    rainAmount[y][x] = 1.f - (mHeightmap->mDisplacement.at(CURRPOS_NS)/100);

            else if(rainAmount[y-1][x] > 0)
                rainAmount[y][x] = rainAmount[y-1][x] - ((mHeightmap->mDisplacement.at(CURRPOS_NS)/100));

            rainAmount[y][x] = rainAmount[y][x] < 0.0 ? 0.0 : rainAmount[y][x];

            mHeightmap->mSplatmap.at(CURRPOS_NS) ={rainAmount[y][x], 1 - rainAmount[y][x], 0.f};

            x++;
        }

        firstIteration = false;
        x = 0;
        y++;
    }

    goto bindRainMap;

SN:
    for(i = 0; i < mHeightmap->mHeightmapDimensions.x; i++){
        for(j = 0; j < mHeightmap->mHeightmapDimensions.y; j++){

            if(firstIteration)
                    rainAmount[y][x] = 1.f - (mHeightmap->mDisplacement.at(CURRPOS_NS)/100);

            else if(rainAmount[y-1][x] > 0)
                rainAmount[y][x] = rainAmount[y-1][x] - ((mHeightmap->mDisplacement.at(CURRPOS_NS)/100));

            rainAmount[y][x] = rainAmount[y][x] < 0.0 ? 0.0 : rainAmount[y][x];

            mHeightmap->mSplatmap.at(CURRPOS_NS) ={rainAmount[y][x], 1 - rainAmount[y][x], 0.f};

            x++;
        }

        firstIteration = false;
        x = 0;
        y++;
    }

    goto bindRainMap;

WE:
    for(i = mHeightmap->mHeightmapDimensions.x * mHeightmap->mHeightmapDimensions.x - 1; i >= mHeightmap->mHeightmapDimensions.x * (mHeightmap->mHeightmapDimensions.x -1); i--){
        for(j = 0; j < mHeightmap->mHeightmapDimensions.y * mHeightmap->mHeightmapDimensions.y; j+= mHeightmap->mHeightmapDimensions.y){

            if(firstIteration)
                    rainAmount[y][x] = 1.f - (mHeightmap->mDisplacement.at(CURRPOS_WE)/100);

            else if(rainAmount[y-1][x] > 0)
                rainAmount[y][x] = rainAmount[y-1][x] - ((mHeightmap->mDisplacement.at(CURRPOS_WE)/100));

            rainAmount[y][x] = rainAmount[y][x] < 0.0 ? 0.0 : rainAmount[y][x];

            mHeightmap->mSplatmap.at(CURRPOS_WE) ={rainAmount[y][x], 1 - rainAmount[y][x], 0.f};

            x++;
        }

        firstIteration = false;
        x = 0;
        y++;
    }

    goto bindRainMap;

EW:
    for(i = mHeightmap->mHeightmapDimensions.x * (mHeightmap->mHeightmapDimensions.x -1); i < mHeightmap->mHeightmapDimensions.x * mHeightmap->mHeightmapDimensions.x; i++){
        for(j = 0; j < mHeightmap->mHeightmapDimensions.y * mHeightmap->mHeightmapDimensions.y; j+=mHeightmap->mHeightmapDimensions.y){

            if(firstIteration)
                    rainAmount[y][x] = 1.f - (mHeightmap->mDisplacement.at(CURRPOS_WE)/100);

            else if(rainAmount[y-1][x] > 0)
                rainAmount[y][x] = rainAmount[y-1][x] - ((mHeightmap->mDisplacement.at(CURRPOS_WE)/100));

            rainAmount[y][x] = rainAmount[y][x] < 0.0 ? 0.0 : rainAmount[y][x];

            mHeightmap->mSplatmap.at(CURRPOS_WE) ={rainAmount[y][x], 1 - rainAmount[y][x], 0.f};

            x++;
        }

        firstIteration = false;
        x = 0;
        y++;
    }

bindRainMap:

    mHeightmap->mSplatmapTexture->bind().setData(GL_RGB, mHeightmap->mHeightmapDimensions.x, mHeightmap->mHeightmapDimensions.y, mHeightmap->mSplatmap);
    mHeightmap->mSplatmapTexture->bind().generateMipmaps();

}


//glm::mat4 Biomes::ScanlineProjection(){


//    mWindPos = -mWindDir;

//    viewVector = mWindDir - mWindPos;
//    float mWindLookAtDistance = glm::length(viewVector);

//    viewVector = viewVector / (float)mWindLookAtDistance;
//    rightVector = glm::normalize(glm::cross(viewVector, mWindUp));
//    upVector = glm::cross(rightVector, viewVector);


//    glm::mat4 lookAt = glm::lookAt(viewVector,
//                                   rightVector,
//                                   upVector);

//    return lookAt;
//}

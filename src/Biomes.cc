#include "Biomes.hh"
#include <random>

#define CURRPOS_NS i*mHeightmap->mHeightmapDimensions.x + j // S->N, N->S
#define CURRPOS_WE i-j // E->W, W->E

Biomes::Biomes(MultiLayeredHeightmap *h){
    mHeightmap = h;
}


void Biomes::generateRainMap(unsigned int randomWindDir){

    std::vector<std::vector<double>> rainAmount(mHeightmap->mHeightmapDimensions.x,std::vector<double>(mHeightmap->mHeightmapDimensions.y,0));
    unsigned int x = 0, y = 0, i, j;
    bool firstIteration = true;

    mRainMap.resize(mHeightmap->mNumberOfVertices);

    mLastWindDir = randomWindDir;

    switch(randomWindDir){ //0 = N->S, 1 = S->N, 2 = W->E, 3 = E->W
        case 0: goto NS;
        case 1: goto SN;
        case 2: goto WE;
        case 3: goto EW;

    }

NS:
    for(i = mHeightmap->mHeightmapDimensions.x - 1; i > 0; i--){
        for(j = 0; j < mHeightmap->mHeightmapDimensions.y; j++){

            if(firstIteration)
                    rainAmount[y][x] = 1.f - ((mHeightmap->mDisplacement.at(CURRPOS_NS) + 100)/10000.f); // default: /100

            else if(rainAmount[y-1][x] > 0)
                rainAmount[y][x] = rainAmount[y-1][x] - ((mHeightmap->mDisplacement.at(CURRPOS_NS) + 100)/10000.f);

            rainAmount[y][x] = rainAmount[y][x] < 0.0 ? 0.0 : rainAmount[y][x];

            mRainMap.at(CURRPOS_NS) ={rainAmount[y][x], 1 - rainAmount[y][x], 0.f};

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
                    rainAmount[y][x] = 1.f - ((mHeightmap->mDisplacement.at(CURRPOS_NS) + 100)/10000.f);

            else if(rainAmount[y-1][x] > 0)
                rainAmount[y][x] = rainAmount[y-1][x] - ((mHeightmap->mDisplacement.at(CURRPOS_NS) + 100)/10000.f);

            rainAmount[y][x] = rainAmount[y][x] < 0.0 ? 0.0 : rainAmount[y][x];

            mRainMap.at(CURRPOS_NS) ={rainAmount[y][x], 1 - rainAmount[y][x], 0.f};

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
                    rainAmount[y][x] = 1.f - ((mHeightmap->mDisplacement.at(CURRPOS_WE) + 100)/10000.f);

            else if(rainAmount[y-1][x] > 0)
                rainAmount[y][x] = rainAmount[y-1][x] - ((mHeightmap->mDisplacement.at(CURRPOS_WE) + 100)/10000.f);

            rainAmount[y][x] = rainAmount[y][x] < 0.0 ? 0.0 : rainAmount[y][x];

            mRainMap.at(CURRPOS_WE) ={rainAmount[y][x], 1 - rainAmount[y][x], 0.f};

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
                    rainAmount[y][x] = 1.f - ((mHeightmap->mDisplacement.at(CURRPOS_WE) + 100)/10000.f);

            else if(rainAmount[y-1][x] > 0)
                rainAmount[y][x] = rainAmount[y-1][x] - ((mHeightmap->mDisplacement.at(CURRPOS_WE) + 100)/10000.f);

            rainAmount[y][x] = rainAmount[y][x] < 0.0 ? 0.0 : rainAmount[y][x];

            mRainMap.at(CURRPOS_WE) ={rainAmount[y][x], 1 - rainAmount[y][x], 0.f};

            x++;
        }

        firstIteration = false;
        x = 0;
        y++;
    }

bindRainMap:

    mRainTexture = glow::Texture2D::create(mHeightmap->mHeightmapDimensions.x, mHeightmap->mHeightmapDimensions.y, GL_RGB);
    mRainTexture->bind().setData(GL_RGB, mHeightmap->mHeightmapDimensions.x, mHeightmap->mHeightmapDimensions.y, mRainMap);
    mRainTexture->bind().generateMipmaps();

}

glow::SharedTexture2D Biomes::getRainTexture() const
{
    return mRainTexture;
}

void Biomes::randomWindDirection(){
    unsigned int randomWindDir = rand() % 4;
    generateRainMap(randomWindDir);
}

glm::vec2 Biomes::GetWindDirection()
{
    switch(mLastWindDir){ //0 = N->S, 1 = S->N, 2 = W->E, 3 = E->W
        case 0: return glm::vec2( 0, -1);
        case 1: return glm::vec2( 0,  1);
        case 2: return glm::vec2(-1,  0);
        case 3: return glm::vec2( 1,  0);
    }
    return glm::vec2(0, 0);
}

#include "Biomes.hh"
#include <random>
#include <list>

#define CURRPOS_NS i*mHeightmap->mHeightmapDimensions.x + j // S->N, N->S
#define CURRPOS_WE i-j // E->W, W->E

Biomes::Biomes(MultiLayeredHeightmap *h){
    mHeightmap = h;
}


void Biomes::generateRainMap(unsigned int randomWindDir){

    std::vector<std::vector<double>> rainAmount(mHeightmap->mHeightmapDimensions.x,std::vector<double>(mHeightmap->mHeightmapDimensions.y,0));
    unsigned int x = 0, y = 0, i, j;
    bool firstIteration = true;
    float initRainValue = 0.f;
    std::vector<std::vector<double>> biomesMerge(mHeightmap->mHeightmapDimensions.x,std::vector<double>(mHeightmap->mHeightmapDimensions.y,0));

    float sum;

    mRainMap.resize(mHeightmap->GetNumberOfVertices());
    mBiomeMap.resize(mHeightmap->GetNumberOfVertices());

    mLastWindDir = randomWindDir;

    switch(randomWindDir){ //0 = N->S, 1 = S->N, 2 = W->E, 3 = E->W
        case 0: goto NS;
        case 1: goto SN;
        case 2: goto WE;
        case 3: goto EW;

    }

NS:
    rain_start = {mHeightmap->mHeightmapDimensions.x, mHeightmap->mHeightmapDimensions.y};
    rain_end = {0,0};

    for(i = mHeightmap->mHeightmapDimensions.x - 1; i > 0; i--){
        for(j = 0; j < mHeightmap->mHeightmapDimensions.y; j++){

            if(firstIteration){
                    rainAmount[y][x] = 1.f - ((mHeightmap->mDisplacement.at(CURRPOS_NS) + 10)/(mHeightmap->mHeightmapDimensions.x * 40)); // default: /0.0171f
                    initRainValue = rainAmount[y][x];
                    biomesMerge[x][y] = 1.f;
            }

            else if(rainAmount[y-1][x] > 0)
                rainAmount[y][x] = rainAmount[y-1][x] - ((mHeightmap->mDisplacement.at(CURRPOS_NS) + 10)/(mHeightmap->mHeightmapDimensions.x * 40));

            rainAmount[y][x] = rainAmount[y][x] < 0.0 ? 0.0 : rainAmount[y][x];

            if(rainAmount[y][x] > initRainValue * 0.7f){
                mRainMap.at(CURRPOS_NS) ={1.f, 0.f, 0.f, 0.f};
                biomesMerge[x][y] = 1.f;
            }

            else{
                biomesMerge[x][y] = biomesMerge[x][y-1] - ((mHeightmap->mDisplacement.at(CURRPOS_NS) + 100)/(mHeightmap->mHeightmapDimensions.x /0.05));
                biomesMerge[x][y] = biomesMerge[x][y] < 0.0 ? 0.0 : biomesMerge[x][y];
                mRainMap.at(CURRPOS_NS) ={biomesMerge[x][y], 1.f - biomesMerge[x][y], 0.f, 0.f};
            }

            if(rainAmount[y][x] > initRainValue * 0.8777f){
                rain_end.x = (mHeightmap->mHeightmapDimensions.x) - 1;
                rain_end.y = (mHeightmap->mHeightmapDimensions.y) - 1;
                rain_start.x = 0;
                rain_start.y = i;
            }

            mBiomeMap.at(CURRPOS_NS) = {mRainMap.at(CURRPOS_NS).r, mRainMap.at(CURRPOS_NS).g , mHeightmap->mSplatmap.at(CURRPOS_NS).b * 5, mHeightmap->mSplatmap.at(CURRPOS_NS).a * 8};
            sum = mBiomeMap.at(CURRPOS_NS).x + mBiomeMap.at(CURRPOS_NS).y + mBiomeMap.at(CURRPOS_NS).z + mBiomeMap.at(CURRPOS_NS).w;

            mBiomeMap.at(CURRPOS_NS).x /= sum;
            mBiomeMap.at(CURRPOS_NS).y /= sum;
            mBiomeMap.at(CURRPOS_NS).z /= sum;
            mBiomeMap.at(CURRPOS_NS).w /= sum;

            x++;
        }

        firstIteration = false;
        x = 0;
        y++;
    }

    NoRain_start = {0, 0};
    NoRain_end = {rain_end.x, rain_start.y};

    goto bindRainMap;

SN:
    for(i = 0; i < mHeightmap->mHeightmapDimensions.x; i++){
        for(j = 0; j < mHeightmap->mHeightmapDimensions.y; j++){

            if(firstIteration){
                rainAmount[y][x] = 1.f - ((mHeightmap->mDisplacement.at(CURRPOS_NS) + 10)/(mHeightmap->mHeightmapDimensions.x * 40)); // default: /0.0171f
                    initRainValue = rainAmount[y][x];
                    biomesMerge[x][y] = 1.f;
            }

            else if(rainAmount[y-1][x] > 0)
                rainAmount[y][x] = rainAmount[y-1][x] - ((mHeightmap->mDisplacement.at(CURRPOS_NS) + 10)/(mHeightmap->mHeightmapDimensions.x * 40));

            rainAmount[y][x] = rainAmount[y][x] < 0.0 ? 0.0 : rainAmount[y][x];

            if(rainAmount[y][x] > initRainValue * 0.7f){
                mRainMap.at(CURRPOS_NS) ={1.f, 0.f, 0.f, 0.f};
                biomesMerge[x][y] = 1.f;
            }
            else{
                biomesMerge[x][y] = biomesMerge[x][y-1] - ((mHeightmap->mDisplacement.at(CURRPOS_NS) + 100)/(mHeightmap->mHeightmapDimensions.x/0.05));
                biomesMerge[x][y] = biomesMerge[x][y] < 0.0 ? 0.0 : biomesMerge[x][y];
                mRainMap.at(CURRPOS_NS) ={biomesMerge[x][y], 1.f - biomesMerge[x][y], 0.f, 0.f};
            }

            if(rainAmount[y][x] > initRainValue * 0.8777f){
                rain_end.x = mHeightmap->mHeightmapDimensions.x-1;
                rain_end.y = i;
                rain_start.x = 0;
                rain_start.y = 0;
            }

            mBiomeMap.at(CURRPOS_NS) = {mRainMap.at(CURRPOS_NS).r, mRainMap.at(CURRPOS_NS).g , mHeightmap->mSplatmap.at(CURRPOS_NS).b * 5, mHeightmap->mSplatmap.at(CURRPOS_NS).a* 8};
            sum = mBiomeMap.at(CURRPOS_NS).x + mBiomeMap.at(CURRPOS_NS).y + mBiomeMap.at(CURRPOS_NS).z + mBiomeMap.at(CURRPOS_NS).w;

            mBiomeMap.at(CURRPOS_NS).x /= sum;
            mBiomeMap.at(CURRPOS_NS).y /= sum;
            mBiomeMap.at(CURRPOS_NS).z /= sum;
            mBiomeMap.at(CURRPOS_NS).w /= sum;

            x++;

         }

        firstIteration = false;
        x = 0;
        y++;
        
    }

    NoRain_start = {0, rain_end.y};
    NoRain_end = {mHeightmap->mHeightmapDimensions.x- 1, mHeightmap->mHeightmapDimensions.y - 1};

    goto bindRainMap;

WE:
    for(i = mHeightmap->mHeightmapDimensions.x * mHeightmap->mHeightmapDimensions.x - 1; i >= mHeightmap->mHeightmapDimensions.x * (mHeightmap->mHeightmapDimensions.x -1); i--){
        for(j = 0; j < mHeightmap->mHeightmapDimensions.y * mHeightmap->mHeightmapDimensions.y; j+= mHeightmap->mHeightmapDimensions.y){

            if(firstIteration){
                rainAmount[y][x] = 1.f - ((mHeightmap->mDisplacement.at(CURRPOS_WE) + 10)/(mHeightmap->mHeightmapDimensions.x * 40)); // default: /0.0171f
                    initRainValue = rainAmount[y][x];
                    biomesMerge[x][y] = 1.f;
            }

            else if(rainAmount[y-1][x] > 0)
                rainAmount[y][x] = rainAmount[y-1][x] - ((mHeightmap->mDisplacement.at(CURRPOS_WE) + 10)/(mHeightmap->mHeightmapDimensions.x * 40));

            rainAmount[y][x] = rainAmount[y][x] < 0.0 ? 0.0 : rainAmount[y][x];

            if(rainAmount[y][x] > initRainValue * 0.7f){
                mRainMap.at(CURRPOS_WE) ={1.f, 0.f, 0.f, 0.f};
                biomesMerge[x][y] = 1.f;
            }

            else{
                biomesMerge[x][y] = biomesMerge[x][y-1] - ((mHeightmap->mDisplacement.at(CURRPOS_WE) + 100)/(mHeightmap->mHeightmapDimensions.x /0.05));
                biomesMerge[x][y] = biomesMerge[x][y] < 0.0 ? 0.0 : biomesMerge[x][y];
                mRainMap.at(CURRPOS_WE) ={biomesMerge[x][y], 1.f - biomesMerge[x][y], 0.f, 0.f};
            }

            if(rainAmount[y][x] > initRainValue * 0.8777f){
                NoRain_start.x = 0;
                NoRain_start.y = 0;
                NoRain_end.x = mHeightmap->WorldToLocalCoordinates({mHeightmap->mPositions.at(CURRPOS_WE).x, mHeightmap->mPositions.at(CURRPOS_WE).z}).x;
                NoRain_end.y = mHeightmap->mHeightmapDimensions.y-1;
            }

            mBiomeMap.at(CURRPOS_WE) = {mRainMap.at(CURRPOS_WE).r, mRainMap.at(CURRPOS_WE).g , mHeightmap->mSplatmap.at(CURRPOS_WE).b * 5, mHeightmap->mSplatmap.at(CURRPOS_WE).a* 8};
            sum = mBiomeMap.at(CURRPOS_WE).x + mBiomeMap.at(CURRPOS_WE).y + mBiomeMap.at(CURRPOS_WE).z + mBiomeMap.at(CURRPOS_WE).w;

            mBiomeMap.at(CURRPOS_WE).x /= sum;
            mBiomeMap.at(CURRPOS_WE).y /= sum;
            mBiomeMap.at(CURRPOS_WE).z /= sum;
            mBiomeMap.at(CURRPOS_WE).w /= sum;

            x++;
        }

        firstIteration = false;
        x = 0;
        y++;
    }

//    rain_end.x /= mHeightmap->mHeightmapDimensions.x;
//    rain_end.y /= mHeightmap->mHeightmapDimensions.y;

    rain_start = {NoRain_end.x, 0};
    rain_end = {mHeightmap->mHeightmapDimensions.x-1, mHeightmap->mHeightmapDimensions.y-1};


    goto bindRainMap;

EW:


    for(i = mHeightmap->mHeightmapDimensions.x * (mHeightmap->mHeightmapDimensions.x -1); i < mHeightmap->mHeightmapDimensions.x * mHeightmap->mHeightmapDimensions.x; i++){
        for(j = 0; j < mHeightmap->mHeightmapDimensions.y * mHeightmap->mHeightmapDimensions.y; j+=mHeightmap->mHeightmapDimensions.y){

            if(firstIteration){
                rainAmount[y][x] = 1.f - ((mHeightmap->mDisplacement.at(CURRPOS_WE) + 10)/(mHeightmap->mHeightmapDimensions.x * 40)); // default: /0.0171f
                    initRainValue = rainAmount[y][x];
                    biomesMerge[x][y] = 1.f;
            }

            else if(rainAmount[y-1][x] > 0)
                rainAmount[y][x] = rainAmount[y-1][x] - ((mHeightmap->mDisplacement.at(CURRPOS_WE) + 10)/(mHeightmap->mHeightmapDimensions.x * 40));

            rainAmount[y][x] = rainAmount[y][x] < 0.0 ? 0.0 : rainAmount[y][x];

            if(rainAmount[y][x] > initRainValue * 0.7f){
                mRainMap.at(CURRPOS_WE) ={1.f, 0.f, 0.f, 0.f};
                biomesMerge[x][y] = 1.f;
            }
            else{
                biomesMerge[x][y] = biomesMerge[x][y-1] - ((mHeightmap->mDisplacement.at(CURRPOS_WE) + 100)/(mHeightmap->mHeightmapDimensions.x /0.05));
                biomesMerge[x][y] = biomesMerge[x][y] < 0.0 ? 0.0 : biomesMerge[x][y];
                mRainMap.at(CURRPOS_WE) ={biomesMerge[x][y], 1.f - biomesMerge[x][y], 0.f, 0.f};
            }

            if(rainAmount[y][x] > initRainValue * 0.8777f){
                NoRain_start.x = mHeightmap->WorldToLocalCoordinates({mHeightmap->mPositions.at(CURRPOS_WE).x, mHeightmap->mPositions.at(CURRPOS_WE).z}).x;
                NoRain_start.y = 0;
                NoRain_end.x = mHeightmap->mHeightmapDimensions.x-1;
                NoRain_end.y = mHeightmap->mHeightmapDimensions.y-1;

            }

            mBiomeMap.at(CURRPOS_WE) = {mRainMap.at(CURRPOS_WE).r, mRainMap.at(CURRPOS_WE).g , mHeightmap->mSplatmap.at(CURRPOS_WE).b * 5, mHeightmap->mSplatmap.at(CURRPOS_WE).a* 8};

            sum = mBiomeMap.at(CURRPOS_WE).x + mBiomeMap.at(CURRPOS_WE).y + mBiomeMap.at(CURRPOS_WE).z + mBiomeMap.at(CURRPOS_WE).w;

            mBiomeMap.at(CURRPOS_WE).x /= sum;
            mBiomeMap.at(CURRPOS_WE).y /= sum;
            mBiomeMap.at(CURRPOS_WE).z /= sum;
            mBiomeMap.at(CURRPOS_WE).w /= sum;

            x++;
        }


        firstIteration = false;
        x = 0;
        y++;
    }

    rain_start = {0, 0};
    rain_end = {NoRain_start.x, mHeightmap->mHeightmapDimensions.y-1};


bindRainMap:

    mRainTexture = glow::Texture2D::create(mHeightmap->mHeightmapDimensions.x, mHeightmap->mHeightmapDimensions.y, GL_RGBA);
    mRainTexture->bind().setData(GL_RGBA, mHeightmap->mHeightmapDimensions.x, mHeightmap->mHeightmapDimensions.y, mRainMap);
    mRainTexture->bind().generateMipmaps();

    mBiomesTexture = glow::Texture2D::create(mHeightmap->mHeightmapDimensions.x, mHeightmap->mHeightmapDimensions.y, GL_RGBA);
    mBiomesTexture->bind().setData(GL_RGBA, mHeightmap->mHeightmapDimensions.x, mHeightmap->mHeightmapDimensions.y, mBiomeMap);
    mBiomesTexture->bind().generateMipmaps();

}

glow::SharedTexture2D Biomes::getRainTexture() const
{
    return mRainTexture;
}

glow::SharedTexture2D Biomes::getBiomesTexture() const
{
    return mBiomesTexture;
}

void Biomes::randomWindDirection(){
    unsigned int randomWindDir = rand() % 4;
    generateRainMap(randomWindDir);
}

glm::vec2 Biomes::GetWindDirection()
{
    switch(mLastWindDir){ //0 = N->S, 1 = S->N, 2 = W->E, 3 = E->W
        case 0: return glm::vec2( 0, 1);
        case 1: return glm::vec2( 0, -1);
        case 2: return glm::vec2( 1,  0);
        case 3: return glm::vec2(-1,  0);
    }
    return glm::vec2(0, 0);
}

std::vector<glm::vec2> Biomes::poissonDiskSampling(float radius, int k, glm::vec2 startpos, glm::vec2 endpos, std::vector<glm::vec2> takenPoints, bool rainy){

    std::vector<glm::vec2> points;
    std::list<glm::vec2> active;
    glm::vec2 p0;

    int N = 2;
    int attempts = 0;
    glm::vec2 start_end = {endpos.x - startpos.x, endpos.y - startpos.y};



    do{
        p0 = {rand() % (unsigned int)start_end.x, rand() % (unsigned int)start_end.y};
        p0 = {startpos.x + p0.x, startpos.y + p0.y};
        attempts++;
    }while((mHeightmap->mPositions.at(p0.y * mHeightmap->mHeightmapDimensions.x + p0.x).y > 23 || mHeightmap->mPositions.at(p0.y * mHeightmap->mHeightmapDimensions.x + p0.x).y <= 14) && attempts <= k);

    if(attempts >= k)
        return takenPoints;

    float cellSize = std::floor(radius/sqrt(N));

    int ncells_width = ceil(endpos.x/cellSize) + 1;
    int ncells_height = ceil(endpos.y/cellSize) + 1;

    std::vector<std::vector<glm::vec2>> grid(ncells_width,std::vector<glm::vec2>(ncells_height,glm::vec2(-1, -1)));

    insertPoint(grid, cellSize, p0);

    points.push_back(p0);
    active.push_back(p0);

    while(active.size() > 0){
        size_t random_index = rand() % (active.size());
        auto it2 = active.begin();
        std::advance(it2, random_index);
        glm::vec2 p = *it2;


        bool found = false;

        for(int tries = 0; tries < k; tries++){
            float theta = rand() % (360);

            int rad_2rad = (2*radius) - radius;

            float new_radius = rand() % rad_2rad;
            new_radius += radius;

            float pnewx = p.x + new_radius * cos(glm::radians(theta));
            float pnewy = p.y + new_radius * sin(glm::radians(theta));
            glm::vec2 pnew = {pnewx, pnewy};

            if (!isValidPoint(grid, cellSize, ncells_width, ncells_height, pnew, radius, startpos, endpos, rainy))
                continue;

            points.push_back(pnew);
            insertPoint(grid, cellSize, pnew);
            active.push_back(pnew);
            found = true;
            break;

        }

        if(!found)
            active.erase(it2);
    }

    return points;
}

void Biomes::insertPoint(std::vector<std::vector<glm::vec2>>& grid, float cellsize, glm::vec2 point) {
    int xindex = floor((float) point.x / cellsize);
    int yindex = floor((float) point.y / cellsize);
    grid[xindex][yindex] = point;
}

bool Biomes::isValidPoint(std::vector<std::vector<glm::vec2>>& grid, float cellsize, int gwidth, int gheight, glm::vec2 p, float radius, glm::vec2 startpos, glm::vec2 endpos, bool rainy) {
    float rain;
    float noRain;


    if (p.x < startpos.x || p.x >= endpos.x || p.y < startpos.y || p.y >= endpos.y)
        return false;

    else if (mHeightmap->mPositions.at((int)p.y * mHeightmap->mHeightmapDimensions.x + (int)p.x).y > 23 || mHeightmap->mPositions.at((int)p.y * mHeightmap->mHeightmapDimensions.x + (int)p.x).y <= 14)
        return false;

    if(rainy){
        rain = mRainMap.at((unsigned int)p.y * mHeightmap->mHeightmapDimensions.y + (unsigned int)p.x).x;
        noRain = 0;
    }
    else{
        rain = 1;
        noRain = mRainMap.at(p.y * mHeightmap->mHeightmapDimensions.y + p.x).x;
    }

    if(rain < 0.7 && noRain > 0.00000005)
        return false;


    int xindex = floor((float)p.x / cellsize);
    int yindex = floor((float)p.y / cellsize);
    int i0 = std::max(xindex - 1, 0);
    int i1 = std::min(xindex + 1, gwidth - 1);
    int j0 = std::max(yindex - 1, 0);
    int j1 = std::min(yindex + 1, gheight - 1);

    for (int i = i0; i <= i1; i++){
        for (int j = j0; j <= j1; j++){
          if (grid[i][j].x != (-1)){

              float xdiff = grid[i][j].x - p.x;
              float zdiff = grid[i][j].y - p.y;

              float distance2 = (xdiff * xdiff) - (zdiff * zdiff);
              float radius2 = radius * radius;

              if (distance2 < radius2)
                return false;
          }
        }
    }

    return true;
}

#define LOC i * mHeightmap->mHeightmapDimensions.y + j
void Biomes::loadBiomesMap(){
    float sum;

    for (unsigned int i = 0; i < mHeightmap->mHeightmapDimensions.y; i++){
        for (unsigned int j = 0; j < mHeightmap->mHeightmapDimensions.x; j++){
            mBiomeMap.at(LOC) = {mRainMap.at(LOC).r, mRainMap.at(LOC).g , mHeightmap->mSplatmap.at(LOC).b * 5, mHeightmap->mSplatmap.at(LOC).a * 8};
            sum = mBiomeMap.at(LOC).r + mBiomeMap.at(LOC).g + mBiomeMap.at(LOC).b + mBiomeMap.at(LOC).a;

            mBiomeMap.at(LOC).r /= sum;
            mBiomeMap.at(LOC).g /= sum;
            mBiomeMap.at(LOC).b /= sum;
            mBiomeMap.at(LOC).a /= sum;
        }
    }

    mBiomesTexture = glow::Texture2D::create(mHeightmap->mHeightmapDimensions.x, mHeightmap->mHeightmapDimensions.y, GL_RGBA);
    mBiomesTexture->bind().setData(GL_RGBA, mHeightmap->mHeightmapDimensions.x, mHeightmap->mHeightmapDimensions.y, mBiomeMap);
    mBiomesTexture->bind().generateMipmaps();

}


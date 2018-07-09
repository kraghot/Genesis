#ifndef BIOMES_H
#define BIOMES_H

#include "MultiLayeredHeightmap.hh"
#include<glow/data/TextureData.hh>



class Biomes
{
public:
    Biomes(MultiLayeredHeightmap *h);
    void generateRainMap(unsigned int randomWindDir);
    glow::SharedTexture2D getRainTexture() const;
    glow::SharedTexture2D getBiomesTexture() const;
    void randomWindDirection();
    glm::vec2 GetWindDirection();
    //void generateBiomes();
    std::vector<glm::vec2> poissonDiskSampling(float radius, int k, glm::vec2 startpos, glm::vec2 endpos);
    std::vector<glm::vec4> mBiomeMap;

    glm::vec2 rain_start;
    glm::vec2 rain_end;

private:
    //glm::vec3 randomWindDirection();
    void insertPoint(std::vector<std::vector<glm::vec2> > &grid, float cellsize, glm::vec2 point);
    bool isValidPoint(std::vector<std::vector<glm::vec2>>& grid, float cellsize, int gwidth, int gheight, glm::vec2 p, float radius, glm::vec2 startpos, glm::vec2 endpos);

    MultiLayeredHeightmap *mHeightmap;

    glow::SharedTexture2D mRainTexture;
    std::vector<glm::vec4> mRainMap;

    glow::SharedTexture2D mBiomesTexture;

    unsigned mLastWindDir;
};

#endif //BIOMES_H

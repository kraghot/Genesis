#ifndef BIOMES_H
#define BIOMES_H

#include "MultiLayeredHeightmap.hh"
#include<glow/data/TextureData.hh>



class Biomes
{
public:
    enum TextureIndices
    {
        LightGrass = 0,
        DarkGrass = 1,
        Rock = 2,
        Beach = 3,
        Underwater = 4
    };

    Biomes(MultiLayeredHeightmap *h);
    void generateRainMap(unsigned int randomWindDir);
    glow::SharedTexture2D getRainTexture() const;
    glow::SharedTexture2D getBiomesTexture() const;
    void randomWindDirection();
    glm::vec2 GetWindDirection();
    //void generateBiomes();
    std::vector<glm::vec2> poissonDiskSampling(float radius, int k, glm::vec2 startpos, glm::vec2 endpos, std::vector<glm::vec2> takenPoints, bool rainy);
    std::vector<glm::vec4> mBiomeMap;
    std::vector<glm::vec4> mIndicesMap;

    void LoadBiomesMap();

    glm::vec2 rain_start;
    glm::vec2 rain_end;

    glm::vec2 NoRain_start;
    glm::vec2 NoRain_end;

private:
    //glm::vec3 randomWindDirection();
    void CalculateBiomeAtLocation(size_t pos);
    void insertPoint(std::vector<std::vector<glm::vec2> > &grid, float cellsize, glm::vec2 point);
    bool isValidPoint(std::vector<std::vector<glm::vec2>>& grid, float cellsize, int gwidth, int gheight, glm::vec2 p, float radius, glm::vec2 startpos, glm::vec2 endpos, bool rainy);

    MultiLayeredHeightmap *mHeightmap;

    glow::SharedTexture2D mRainTexture;
    std::vector<glm::vec4> mRainMap;

    glow::SharedTexture2D mBiomesTexture;
    glow::SharedTexture2D mIndicesTexture;

    unsigned mLastWindDir;
};

#endif //BIOMES_H

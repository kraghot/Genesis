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

private:
    //glm::vec3 randomWindDirection();
    MultiLayeredHeightmap *mHeightmap;

    glow::SharedTexture2D mRainTexture;
    std::vector<glm::vec4> mRainMap;

    glow::SharedTexture2D mBiomesTexture;
    std::vector<glm::vec4> mBiomeMap;

    unsigned mLastWindDir;
};

#endif //BIOMES_H

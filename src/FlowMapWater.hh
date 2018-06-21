#ifndef FLOWMAPWATER_HH
#define FLOWMAPWATER_HH

#include <glow/objects/Texture2D.hh>

class MultiLayeredHeightmap;

class FlowMapWater
{
public:
    FlowMapWater(unsigned width, unsigned height, MultiLayeredHeightmap* heightmap);
    glow::SharedTexture2D GetFlowTexture() {return mFlowTexture;}
    void SetWindDirection(glm::vec2 windDirection);

private:
    void GenerateFlowTexture();
    int incDec(int value, bool increment);

    unsigned mWidth, mHeight;
    glow::SharedTexture2D mFlowTexture;
    std::vector<glm::vec2> mFlowData;
    MultiLayeredHeightmap* mHeightmap;
    glm::vec2 mWindDirection;
};

#endif // FLOWMAPWATER_HH

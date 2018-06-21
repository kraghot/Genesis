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
    void FlowParticle(glm::vec2& particle, const glm::vec2& direction);
    int incDec(int value, bool increment);
    bool IsInBounds(glm::vec2& particle);
    glm::vec2 GetNextPosition(glm::vec2 currPos, glm::vec2 direction);

    /**
     * @brief ApplyFlow applies flow in a brush like pattern
     * @param coords is the location of the center of change
     * @param flow is the direction of the flow which should be applied
     * @param radius is the radius of the brush
     */
    void ApplyFlow(glm::uvec2& coords, glm::vec2 flow, float radius);

    unsigned mWidth, mHeight;
    glow::SharedTexture2D mFlowTexture;
    std::vector<glm::vec2> mFlowData;
    MultiLayeredHeightmap* mHeightmap;
    glm::vec2 mWindDirection;
    float mWaterLevel = 10.0;
    glm::uvec2 mOffsetToTerrain;
};

#endif // FLOWMAPWATER_HH

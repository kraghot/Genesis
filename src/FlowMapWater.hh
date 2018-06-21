#ifndef FLOWMAPWATER_HH
#define FLOWMAPWATER_HH

#include <glow/objects/Texture2D.hh>

class MultiLayeredHeightmap;

class FlowMapWater
{
public:
    /**
     * @brief FlowMapWater initializes the dimensions and heightmap pointer
     * It will automatically spread itself to a 1.5 times larger surface than the heightmap
     * @param width is the width of the heightmap
     * @param height is the height of the heightmap
     * @param heightmap is the pointer to the MultiLayeredHeightmap
     */
    FlowMapWater(unsigned width, unsigned height, MultiLayeredHeightmap* heightmap);

    /**
     * @brief GetFlowTexture returns the OpenGL texture containing the flow data
     * @warning SetWindDirection() has to be called at least once before getting the texture!
     */
    glow::SharedTexture2D GetFlowTexture() {return mFlowTexture;}

    /**
     * @brief SetWindDirection recalculates and prepares a texture with the flow data
     */
    void SetWindDirection(glm::vec2 windDirection);

private:
    /**
     * @brief GenerateFlowTexture prepares an OpenGL texture with the flow data
     */
    void GenerateFlowTexture();

    /**
     * @brief FlowParticle simulates the travel of a particle according to wind and terrain
     * @param particle is the particle to flow
     * @param direction is the wind direction
     */
    void FlowParticle(glm::vec2& particle, const glm::vec2& direction);

    /**
     * @brief IsInBounds checks if the particle is in the bound of the heightmap
     * @param particle is the particle to check with
     * @return true if inside bounds of heightmap
     */
    bool IsInBounds(glm::vec2& particle);

    /**
     * @brief GetNextPosition returns the next position according to the wind and terrain
     * @param currPos is the position to move from
     * @param direction is the wind direction
     */
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

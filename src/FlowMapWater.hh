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

    /**
     * @brief SpawnRiver attmepts to create a river in the given coordinates
     * @param worldCoordinates is where the river starts
     * @param flowVolume is the magnitude of the river
     */
    void SpawnRiver(glm::vec3 worldCoordinates, float flowVolume);

    void SetFlowAt(glm::uvec2 heightmapCoordinates, glm::vec2 flow);

    /**
     * @brief GenerateFlowTexture prepares an OpenGL texture with the flow data
     */
    void GenerateFlowTexture();

    float GetWaterLevel() {return mWaterLevel;}

    /**
     * @brief ResetMap zeroes the flowmap out
     */
    void ResetMap();
private:
    friend class MultiLayeredHeightmap;

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

    /**
     * @brief GetTriangleArea calculates the surface area of a triangle using Heron's formula
     * @param A, B, C are points of a triangle
     * @return Surface area of a triangle
     */\
    float GetTriangleArea(glm::vec3 A, glm::vec3 B, glm::vec3 C);

    /**
     * @brief InitializeOcean initializes all locations where the terrain height is less than the water level
     * @param value is the direction to use in initialization
     */
    void InitializeOcean(glm::vec2 value);

    /**
     * @brief ConvertToHeightmapCoords converts the flowmap coords to the heightmap coords
     */
    glm::vec2 ConvertToHeightmapCoords(glm::vec2 flowCoords);

    /**
     * @brief ClampTo1 clamps a vector to a max length of 1
     */
    glm::vec2 ClampTo1(glm::vec2& vector);

    unsigned mWidth, mHeight;
    glow::SharedTexture2D mFlowTexture;
    std::vector<glm::vec2> mFlowData;
    MultiLayeredHeightmap* mHeightmap;
    glm::vec2 mWindDirection;
    float mWaterLevel = 10.0;
    glm::uvec2 mOffsetToTerrain;
};

#endif // FLOWMAPWATER_HH

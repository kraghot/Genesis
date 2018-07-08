#ifndef MLHEIGHT
#define MLHEIGHT

#include<string>
#include<istream>

#include <algorithm>
#include <ctime>
#include <fstream>
#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <vector>

#include<glm/common.hpp>
#include<glm/glm.hpp>
#include <glm/ext.hpp>

#include <glow/common/scoped_gl.hh>
#include <glow/gl.hh>
#include <glow/objects/Framebuffer.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/TextureRectangle.hh>
#include <glow/objects/VertexArray.hh>
#include<glow/objects/Texture2DArray.hh>
#include<glow/objects/ArrayBuffer.hh>
#include<glow/objects/ElementArrayBuffer.hh>
#include<glow/data/TextureData.hh>
#include<glow/data/SurfaceData.hh>

#include <AntTweakBar.h>

#include "NoiseGenerator.hh"
#include "FilterGenerator.hh"


inline long getFileSize(FILE *file)
    {
        long lCurPos, lEndPos;
        lCurPos = ftell(file);
        fseek(file, 0, 2);
        lEndPos = ftell(file);
        fseek(file, lCurPos, 0);
        return lEndPos;
    }

inline float getPercentage(float value, const float min, const float max){
    value = glm::clamp(value, min, max);
    return (value-min)/(max-min);
}

inline int getFileLength(std::istream& file){
    int pos = file.tellg();
    file.seekg(0, std::ios::end );
    int length = file.tellg();
    // Restore the position of the get pointer
    file.seekg(pos);

    return length;
}

class FlowMapWater;

class MultiLayeredHeightmap
{
public:
    enum NeighSide {
        Left = 0,
        Right = 1,
        Up = 2,
        Down = 3
    };

    struct GeneratorProperties
    {
        NoiseGenerator& generator;
        unsigned numOctaves;
        float freq;
        float freqScale;
        float amplitude;
        float amplitudeScale;

        /// Constructor for usage with emplace_back
        GeneratorProperties(NoiseGenerator& inGenerator, unsigned inNumOctaves, float inFreq,
                            float inFreqScale, float inAmplitude, float inAmplitudeScale):
            generator(inGenerator)
          , numOctaves(inNumOctaves)
          , freq(inFreq)
          , freqScale(inFreqScale)
          , amplitude(inAmplitude)
          , amplitudeScale(inAmplitudeScale) {}
    };

    unsigned int mNumberOfVertices;

    // float heightScale: determines the maximum height of the terrain in world units.
    // float blockScale: determines the space between terrain vertices in world units for both the X and Z axes.
    MultiLayeredHeightmap(float heightScale, float blockScale);
    virtual ~MultiLayeredHeightmap();
    glow::SharedVertexArray LoadHeightmap(const char *filename, unsigned char bitsPerPixel);
    glow::SharedVertexArray GenerateTerrain(std::vector<GeneratorProperties>& properties, std::vector<FilterGenerator*> filters,
                                            unsigned int dimX, unsigned int dimY);

    // Get the height of the terrain at a position in world space, position = world space position
    float GetHeightAt(const glm::vec3& position);

    glow::SharedTexture2DArray LoadTexture(std::vector<std::string> textureName);
    glow::SharedTexture2DArray LoadNormal(std::vector<std::string> normalName);

    glm::vec4 CalculateRGBA(float fRange1, float fRange2, float fRange3, float fRange4, float fScale);

    void LoadSplatmap();

    /**
     * @brief DumpHeightmapToFile writes the position data to a RAW file
     */
    void DumpHeightmapToFile();
    void DumpSplatmapToFile();
    void ThermalErodeTerrain();
    void HydraulicErodeTerrain();
    void DropletErodeTerrain(glm::vec2 coordinates, float strength=1.0f);
    float getMfHeightScale() const;
    float GetDisplacementAt(glm::uvec2 pos);
    void SetDisplacementAt(glm::uvec2 pos, float value);
    /// @todo Add brushlike softening and deposition
    void AddDisplacementAt(glm::uvec2 pos, float addition);
    void AddClampedDisplacementAt(glm::uvec2 pos, float addition, float min);
    void AddSoftDisplacement(glm::uvec2 pos, float addition);
    void IterateDroplet(int num);
    std::vector<glm::uvec2> GetNeighborhood(unsigned int i, unsigned int j);

    /**
     * @brief GetNeighborhood returns the Von Neumann neighborhood of a given point
     * @param coord is the point for which to generate neighbours
     * @return list of neighbours
     */
    std::vector<glm::uvec2> GetNeighborhood(glm::uvec2 coord);

    /**
     * @brief GetMooreNeighborhood returns the Moore neighborhood of a given point
     * @param coord is the point for which to generate neighbours
     * @return list of neighbours
     */
    std::vector<glm::uvec2> GetMooreNeighborhood(glm::uvec2 coord);

    /**
     * @brief GetLowestNeigh returns the point with the lowest height
     * @param neigh is a list of of points which to check
     * @return the coordinates of the lowest point
     */
    glm::uvec2 GetLowestNeigh(std::vector<glm::uvec2>& neigh);

    /**
     * @brief WorldToLocalCoordinates calculates the local position on the heightmap from world coords
     * @param position are the XZ world coords
     * @return coords on the heightmap
     */
    glm::uvec2 WorldToLocalCoordinates(glm::vec2 position);

    /**
     * @brief LocalToWorldCoordinates calculates the world coordinates from local coordinates
     * @param position is the XZ local coord
     * @return XZ world coords
     */
    glm::vec3 LocalToWorldCoordinates(glm::uvec2 position);

    glm::vec3 LocalToWorldCoordinates(glm::vec3 pos);

    /**
     * @brief IsWaterMass checks if the coordinate is a river of a lake
     */
    bool IsWaterMass(glm::uvec2 pos);

    glow::SharedVertexArray GetRainMesh() {return mRainMesh;}

    glow::SharedTexture2D GetDisplacementTexture() const;

    glow::SharedTexture2D getSplatmapTexture() const;
    void CalculateNormalsTangents(int dimX, int dimY);
    glow::SharedVertexArray getVao() const;
    std::vector<glm::vec3> mPositions;
    std::vector<float> mDisplacement;

    void MakeVertexArray();

    glow::SharedTexture2D mSplatmapTexture;
    // The dimensions of the heightmap texture
    glm::uvec2 mHeightmapDimensions;
    std::vector<glm::vec4> mSplatmap;

    std::vector<glm::vec4> mTemperatureMap;
    std::vector<glm::vec4> mSlopeMap;

    float mfBlockScale;
    float halfTerrainWidth;

    float GetMfBlockScale() const;
    FlowMapWater* mFlowMap;
    glow::SharedTexture2D mRainFlowMapTexture;

protected:

    void FillData(std::vector<float>& heights);

    float mfHeightScale;
    float mHeightValue;

    std::vector<glm::vec3> mNormals;
    std::vector<glm::vec4> mColors;
    std::vector<uint32_t> mIndices;
    std::vector<glm::vec2> mTexCoords;
    std::vector<glm::vec2> mHeightCoords;
    std::vector<float> mWaterLevel;

    std::vector<glm::vec3> mNormals1;
    std::vector<glm::vec3> mNormals2;
    std::vector<glm::vec3> mNormalsFinal;

    std::vector<glm::vec3> mTangents1;
    std::vector<glm::vec3> mTangents2;
    std::vector<glm::vec3> mTangentsFinal;

    std::vector<float> mSlopeY;

    std::vector<glow::SharedTextureData> mTexture;
    std::vector<glow::SharedSurfaceData> mSurface;

    std::vector<glow::SharedTextureData> mTextureNormal;
    std::vector<glow::SharedSurfaceData> mNormalSurface;

    std::vector<glow::SharedArrayBuffer> mAbs;
    glow::SharedElementArrayBuffer mEab;
    glow::SharedVertexArray mVao;
    glow::SharedTexture2D mDisplacementTexture;

    glm::mat4x4 mLocalToWorldMatrix;

    std::vector<float> mRainFlowMap;
    glow::SharedVertexArray mRainMesh;

};

#endif

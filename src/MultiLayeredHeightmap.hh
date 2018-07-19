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

    /**
     * @brief The GeneratorProperties struct is used to pass all the properties of a generator
     * to the generation engine
     */
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

    /**
     * @param heightScale determines the maximum height of the terrain in world units
     * @param blockScale determines the space between terrain vertices in world units for both the X and Z axes
     */
    MultiLayeredHeightmap(float heightScale, float blockScale);
    virtual ~MultiLayeredHeightmap();

    /**
     * @brief LoadHeightmap loads the heightmap from RAW format
     * @param filename is the name of the dumped file
     * @param bitsPerPixel is usually specified in the filename of the raw file
     * @return Vertex Array to the loaded heightmap
     */
    glow::SharedVertexArray LoadHeightmap(const char *filename, unsigned char bitsPerPixel);

    /**
     * @brief GenerateTerrain generates a heightmap from a multitude of noise generators and filters
     * @param properties are the generator properties to use in generation @see GeneratorProperties
     * @param filters are the filters used in the generation @see FilterGenerator
     * @param dimX is the X dimension of the heightmap
     * @param dimY is the Y dimension of the heightmap
     * @return Vertex array of the heightmap
     */
    glow::SharedVertexArray GenerateTerrain(std::vector<GeneratorProperties>& properties, std::vector<FilterGenerator*> filters,
                                            unsigned int dimX, unsigned int dimY);

    // Get the height of the terrain at a position in world space, position = world space position
    /**
     * @brief GetHeightAt returns the height of the terrain at a position in world space
     * @param position in world-space
     */
    float GetHeightAt(const glm::vec3& position);

    glow::SharedTexture2DArray LoadTexture(std::vector<std::string> textureName);
    glow::SharedTexture2DArray LoadNormal(std::vector<std::string> normalName);

    glm::vec4 CalculateRGBA(float fRange1, float fRange2, float fRange3, float fRange4, float fScale);

    void LoadSplatmap();

    /**
     * @brief DumpHeightmapToFile writes the position data to a RAW file
     */
    void DumpHeightmapToFile();
    /**
     * @brief DumpSplatmapToFile dumps raw splatmap data to a RAW file
     */
    void DumpSplatmapToFile();
    /**
     * @brief ThermalErodeTerrain applies thermal erosion to the whole terrain
     */
    void ThermalErodeTerrain();
    /**
     * @brief HydraulicErodeTerrain applies hydraulic erosion to the terrain
     * @warning abandoned @see DropletErodeTerrain
     */
    void HydraulicErodeTerrain();
    /**
     * @brief DropletErodeTerrain applies terrain erosion using the droplet model
     * @param coordinates is the coordinate where to apply the erosion
     * @param strength is the amount of water to drop at that point
     */
    void DropletErodeTerrain(glm::vec2 coordinates, float strength=1.0f);
    float getMfHeightScale() const;
    /**
     * @brief GetDisplacementAt returns the height at location
     * @param pos position in local coordinates
     * @return height at positions
     */
    float GetDisplacementAt(glm::ivec2 pos);
    /**
     * @brief SetDisplacementAt sets the position at the local coordinates
     * @param pos is the local position on the heightmap
     * @param value is the value to set
     */
    void SetDisplacementAt(glm::ivec2 pos, float value);
    /**
     * @brief AddDisplacementAt is the same as GetDisplacementAt() except that it adds to the height
     */
    void AddDisplacementAt(glm::ivec2 pos, float addition);
    /**
     * @brief AddClampedDisplacementAt adds displacement while clamping to the minimum
     * @param pos local position
     * @param addition how much to add
     * @param min is the value to clamp against
     */
    void AddClampedDisplacementAt(glm::ivec2 pos, float addition, float min);
    /**
     * @brief AddSoftDisplacement applies displacement to the given local position and the Von-Neumann neighborhood
     * @param pos position in local coordinates
     * @param addition how much height to add
     */
    void AddSoftDisplacement(glm::ivec2 pos, float addition);
    /**
     * @brief IterateDroplet applies droplet erosion num times at random positions on the terrain
     * @param num
     */
    void IterateDroplet(int num);

    /**
     * @brief GetNumberOfVertices returns the total number of vertices in the heighmap (x * y)
     */
    unsigned GetNumberOfVertices() {return mNumberOfVertices;}

    /**
     * @brief GetNeighborhood returns the Von Neumann neighborhood of a given point
     * @param coord is the point for which to generate neighbours
     * @return list of neighbours
     */
    std::vector<glm::ivec2> GetNeighborhood(glm::ivec2 coord);
    std::vector<glm::ivec2> GetNeighborhood(int i, int j);

    /**
     * @brief GetMooreNeighborhood returns the Moore neighborhood of a given point
     * @param coord is the point for which to generate neighbours
     * @return list of neighbours
     */
    std::vector<glm::ivec2> GetMooreNeighborhood(glm::ivec2 coord);

    /**
     * @brief GetLowestNeigh returns the point with the lowest height
     * @param neigh is a list of of points which to check
     * @return the coordinates of the lowest point
     */
    glm::ivec2 GetLowestNeigh(std::vector<glm::ivec2> &neigh);

    /**
     * @brief WorldToLocalCoordinates calculates the local position on the heightmap from world coords
     * @param position are the XZ world coords
     * @return coords on the heightmap
     */
    glm::ivec2 WorldToLocalCoordinates(glm::vec2 position);

    /**
     * @brief LocalToWorldCoordinates calculates the world coordinates from local coordinates
     * @param position is the XZ local coord
     * @return XZ world coords
     */
    glm::vec3 LocalToWorldCoordinates(glm::ivec2 position);
    glm::vec3 LocalToWorldCoordinates(glm::vec3 pos);

    /**
     * @brief IsWaterMass checks if the coordinate is a river of a lake
     */
    bool IsWaterMass(glm::ivec2 pos);

    /**
     * @brief ComputeAmbientOcclusionMap baked an ambient occlusion map using gaussian averages
     */
    void ComputeAmbientOcclusionMap();

    /**
     * @brief GaussianKernel calculates the gaussian kernel for the given radius
     * @return a vector of vectors containing the kernel
     */
    inline std::vector<std::vector<float> > GaussianKernel(int radius)
    {
        double sigma = 1.0;
        double r, s = 2.0 * sigma * sigma;

        int halfrad = radius / 2;
        float sum = 0;

        std::vector<std::vector<float> > kernel(radius, std::vector<float>(radius, 0));

        for(int j = -halfrad; j <= halfrad; j++)
        {
            for(int i = -halfrad; i <= halfrad; i++)
            {
                r = sqrt(i*i + j*j);
                kernel[i + halfrad][j + halfrad] =
                         (std::exp(-(r*r)/s))/(M_PI * s);
                sum += kernel[i + halfrad][j + halfrad];
            }
        }

        return kernel;
    }

    /**
     * @brief GetDisplacementTexture returns the heightmap texture handle in GL_RED32F format
     */
    glow::SharedTexture2D GetDisplacementTexture() const;

    glow::SharedTexture2D GetSplatmapTexture() const;
    void CalculateNormalsTangents(glm::vec2 start_pos, glm::vec2 end_pos);
    glow::SharedVertexArray GetVao() const;

    ///@todo make private
    std::vector<glm::vec3> mPositions;
    std::vector<float> mDisplacement;

    /**
     * @brief MakeVertexArray creates all necessary Array and Index buffers, puts them in a VAO and creates the displacement texture
     */
    void MakeVertexArray();

    /// @todo These members do not belong in public
    glow::SharedTexture2D mSplatmapTexture;
    // The dimensions of the heightmap texture
    glm::ivec2 mHeightmapDimensions;
    std::vector<glm::vec4> mSplatmap;

    std::vector<glm::vec4> mTemperatureMap;
    std::vector<glm::vec4> mSlopeMap;

    float mfBlockScale;
    float halfTerrainWidth;

    float GetMfBlockScale() const;
    FlowMapWater* mFlowMap;
    glow::SharedTexture2D mRainFlowMapTexture;
    std::vector<glow::SharedArrayBuffer> mAbs;
    std::vector<glm::vec3> mNormalsFinal;
    glow::SharedTexture2D mAmbientOcclusionMap;

    std::vector<glm::vec3> mNormals;
    std::vector<glm::vec3> mNormals1;

protected:

    /**
     * @brief FillData initializes all the data, calculates the vector positions and indices.
     * Also calls for generation of all data  ready to be calculated at this time
     * @param heights
     */
    void FillData(std::vector<float>& heights);

    unsigned int mNumberOfVertices;
    float mfHeightScale;
    float mHeightValue;

    std::vector<glm::vec4> mColors;
    std::vector<uint32_t> mIndices;
    std::vector<glm::vec2> mTexCoords;
    std::vector<glm::vec2> mHeightCoords;
    std::vector<float> mWaterLevel;

    std::vector<glm::vec3> mNormals2;

    std::vector<glm::vec3> mTangents1;
    std::vector<glm::vec3> mTangents2;
    std::vector<glm::vec3> mTangentsFinal;

    std::vector<float> mSlopeY;

    std::vector<glow::SharedTextureData> mTexture;
    std::vector<glow::SharedSurfaceData> mSurface;

    std::vector<glow::SharedTextureData> mTextureNormal;
    std::vector<glow::SharedSurfaceData> mNormalSurface;

    glow::SharedElementArrayBuffer mEab;
    glow::SharedVertexArray mVao;
    glow::SharedTexture2D mDisplacementTexture;

    glm::mat4x4 mLocalToWorldMatrix;

    std::vector<float> mRainFlowMap;

};

#endif

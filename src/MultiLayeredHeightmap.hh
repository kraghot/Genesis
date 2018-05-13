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

#include "NoiseGenerator.hh"

enum ETextureFiltering
{
    TEXTURE_FILTER_MAG_NEAREST = 0, // Nearest criterion for magnification
    TEXTURE_FILTER_MAG_BILINEAR, // Bilinear criterion for magnification
    TEXTURE_FILTER_MIN_NEAREST, // Nearest criterion for minification
    TEXTURE_FILTER_MIN_BILINEAR, // Bilinear criterion for minification
    TEXTURE_FILTER_MIN_NEAREST_MIPMAP, // Nearest criterion for minification, but on closest mipmap
    TEXTURE_FILTER_MIN_BILINEAR_MIPMAP, // Bilinear criterion for minification, but on closest mipmap
    TEXTURE_FILTER_MIN_TRILINEAR, // Bilinear criterion for minification on two closest mipmaps, then averaged
};



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


class MultiLayeredHeightmap
{
public:
    enum NeighSide {
        Left = 0,
        Right = 1,
        Up = 2,
        Down = 3
    };

    // float heightScale: determines the maximum height of the terrain in world units.
    // float blockScale: determines the space between terrain vertices in world units for both the X and Z axes.
    MultiLayeredHeightmap(float heightScale, float blockScale);
    virtual ~MultiLayeredHeightmap();
    glow::SharedVertexArray LoadHeightmap(const char *filename, unsigned char bitsPerPixel);
    glow::SharedVertexArray GenerateTerrain(NoiseGenerator* generator, unsigned int dimX, unsigned int dimY, unsigned int octaves = 4, float freqScale = 0.5f, float maxHeight = 1.0f);

    // Get the height of the terrain at a position in world space, position = world space position
    float GetHeightAt(const glm::vec3& position);

    glow::SharedTexture2DArray LoadTexture(std::vector<std::string> textureName);
    glow::SharedTexture2DArray LoadNormal(std::vector<std::string> normalName);

    void LoadSplatmap();

    // translate the incoming char data array into a floating point value in the range [0…1]
    // If you wanted to load height maps that are stored MSB,LSB, you would have to reverse the array indices for values that read more than 1 byte.
    inline float GetHeightValue(const unsigned char* data, unsigned char numBytes){
        switch ( numBytes )
            {
            case 1:
                {
                    return (unsigned char)(data[0]) / (float)0xff;
                }
                break;
            case 2:
                {
                    return (unsigned short)(data[1] << 8 | data[0] ) / (float)0xffff;
                }
                break;
            case 4:
                {
                    return (unsigned int)(data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0] ) / (float)0xffffffff;
                }
                break;
            default:
                {
                    assert(false);  // Height field with non standard pixle sizes
                }
                break;
            }

            return 0.0f;
    }

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
    /// @todo Add brushlike softening and deposition
    void AddDisplacementAt(glm::uvec2 pos, float addition);
    void AddSoftDisplacement(glm::uvec2 pos, float addition);
    void IterateDroplet(int num);

    glow::SharedTexture2D GetDisplacementTexture() const;

    glow::SharedTexture2D getSplatmapTexture() const;

private:
    void MakeVertexArray();
    void FillData(std::vector<float>& heights);
    void CalculateNormalsTangents(int dimX, int dimY);
    std::vector<glm::uvec2> GetNeighborhood(unsigned int i, unsigned int j);
    std::vector<glm::uvec2> GetNeighborhood(glm::uvec2 coord);
    glm::uvec2 GetLowestNeigh(std::vector<glm::uvec2>& neigh);

    float mfHeightScale;
    float mfBlockScale;
    float mHeightValue;

    std::vector<glm::vec3> mPositions;
    std::vector<float> mDisplacement;
    std::vector<glm::vec3> mNormals;
    std::vector<glm::vec4> mColors;
    std::vector<uint32_t> mIndices;
    std::vector<glm::vec2> mTexCoords;
    std::vector<glm::vec2> mHeightCoords;
    std::vector<float> mWaterLevel;

    std::vector<glm::vec3> normals1;
    std::vector<glm::vec3> normals2;
    std::vector<glm::vec3> normals_final;

    std::vector<glm::vec3> tangents1;
    std::vector<glm::vec3> tangents2;
    std::vector<glm::vec3> tangents_final;

    std::vector<float> slope_y;

    std::vector<glm::vec4> mSplatmap;
    glow::SharedTexture2D mSplatmapTexture;


    std::vector<glow::SharedTextureData> mTexture;
    std::vector<glow::SharedSurfaceData> mSurface;

    std::vector<glow::SharedTextureData> mTextureNormal;
    std::vector<glow::SharedSurfaceData> mNormalSurface;

    std::vector<glow::SharedArrayBuffer> mAbs;
    glow::SharedElementArrayBuffer mEab;
    glow::SharedVertexArray mVao;
    glow::SharedTexture2D mDisplacementTexture;

    glm::mat4x4 mLocalToWorldMatrix;
    // The dimensions of the heightmap texture
    glm::uvec2 mHeightmapDimensions;
    unsigned int mNumberOfVertices;
};

#endif

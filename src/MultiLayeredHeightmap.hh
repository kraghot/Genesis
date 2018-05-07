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

struct Ray
{
    glm::vec3 origin = {0, 0, 0};
    glm::vec3 direction = {0, 0, 0};
};

struct Face
{
    glm::vec3 p0;
    glm::vec3 p1;
    glm::vec3 p2;
};


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

    glow::SharedTexture2D GetDisplacementTexture() const;

    glow::SharedTexture2D getSplatmapTexture() const;

    bool bary_coord(const glm::vec3& _p, const glm::vec3& _u, const glm::vec3& _v, const glm::vec3& _w, glm::vec3& _result) const;
    bool intersectTriangle(const Face& _face, const glm::vec3 &_normal, const Ray &_ray);
    void intersect(const Ray& _ray );

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

    std::vector<glm::vec3> mNormals1;
    std::vector<glm::vec3> mNormals2;
    std::vector<glm::vec3> mNormalsFinal;

    std::vector<glm::vec3> mTangents1;
    std::vector<glm::vec3> mTangents2;
    std::vector<glm::vec3> mTangentsFinal;

    std::vector<float> mSlopeY;

    std::vector<glm::vec3> mSplatmap;
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

    glm::dvec3 intersectionPoint;
    float _t;
    double epsilon = 0.01;

};

#endif

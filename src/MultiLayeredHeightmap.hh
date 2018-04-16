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
    // float heightScale: determines the maximum height of the terrain in world units.
    // float blockScale: determines the space between terrain vertices in world units for both the X and Z axes.
    MultiLayeredHeightmap(float heightScale, float blockScale);
    virtual ~MultiLayeredHeightmap();
    glow::SharedVertexArray LoadHeightmap(const char *filename, unsigned char bitsPerPixel, unsigned int width, unsigned int height);

    // Get the height of the terrain at a position in world space, position = world space position
    float getHeightAt(const glm::vec3& position);

    //Load textures into 1 of 3 texture stages (3 stages supported; 0 1 and 2
    void LoadTexture(const char *filename, unsigned int textureStage);

    void BindTerrainTexture(glow::SharedTexture2D uiTexture, GLuint uiSampler, int unit);

   static const unsigned int numLayers = 2; //2+1

   glow::SharedTexture2D terrainColor[numLayers];
   glow::SharedTexture2D terrainNormal[numLayers];

   std::vector<glm::vec3> positions;
   std::vector<glm::vec3> normals;
   std::vector<glm::vec4> colors;
   std::vector<uint32_t> indices;
   std::vector<glm::vec2> tex0buffer;

   GLuint uiSampler;



    // translate the incoming char data array into a floating point value in the range [0…1]
    // If you wanted to load height maps that are stored MSB,LSB, you would have to reverse the array indices for values that read more than 1 byte.
    inline float getHeightValue(const unsigned char* data, unsigned char numBytes){
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

    float m_fHeightScale;
    float m_fBlockScale;




private:
    glm::mat4x4 m_LocalToWorldMatrix;

    // The dimensions of the heightmap texture
    glm::uvec2 m_HeightmapDimensions;





};

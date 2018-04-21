#include "MultiLayeredHeightmap.hh"
#include "GlowApp.hh"

#include<stdio.h>

typedef std::basic_ios<char> ios;
//===========MACROS===========

// Enable mutitexture blending across the terrain
#ifndef ENABLE_MULTITEXTURE
#define ENABLE_MULTITEXTURE 1
#endif

// Enable the blend constants based on the slope of the terrain
#ifndef ENABLE_SLOPE_BASED_BLEND
#define ENABLE_SLOPE_BASED_BLEND 1
#endif

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

//===========MACROS===========

GlowApp GlowAppObject;


MultiLayeredHeightmap::MultiLayeredHeightmap(float heightScale, float blockScale):
    mfHeightScale(heightScale),
    mfBlockScale(blockScale),
    mHeightmapDimensions(0,0)
    {

}

MultiLayeredHeightmap::~MultiLayeredHeightmap(){

}


glow::SharedTexture2DArray MultiLayeredHeightmap::LoadTexture(std::vector<std::string> textureName){

    tex.resize(textureName.size());
    surface.resize(textureName.size());

    std::cout << "texname0 = "<<textureName[0]<< std::endl;

    for(auto i = 0u; i < textureName.size(); i++){
        tex[i] = (glow::TextureData::createFromFile(textureName[i], glow::ColorSpace::sRGB));
        surface[i] = tex[i]->getSurfaces()[0];
    }

    for(auto j = 0u; j < surface.size(); j++){
        surface[j]->setOffsetZ(j);
        tex[0]->addSurface(surface[j]);
    }

    tex[0]->setTarget(GL_TEXTURE_2D_ARRAY);
    tex[0]->setDepth(surface.size());

    return glow::Texture2DArray::createFromData(tex[0]);
}

void MultiLayeredHeightmap::DumpToFile()
{
    std::ostringstream filename;
    filename << "terrain-8bbp-" << mHeightmapDimensions.x << "x" << mHeightmapDimensions.y << ".raw";
    std::ofstream file (filename.str(), std::ios::out | std::ios::binary);
    std::vector<uint8_t> byteField;
    byteField.reserve(mNumberOfVertices);
    for(auto it : mPositions)
    {
        /// @todo Correct for y scaling
        byteField.push_back(it.y * (255));
    }
    file.write((char *)byteField.data(), byteField.size());
}

void MultiLayeredHeightmap::MakeVertexArray()
{
    auto ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aPosition");
    ab->bind().setData(mPositions, GL_DYNAMIC_DRAW);
    mAbs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aNormal");
    ab->bind().setData(mNormals);
    mAbs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec4>("aColor");
    ab->bind().setData(mColors);
    mAbs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec2>("aTexCoord");
    ab->bind().setData(mTexCoords);
    mAbs.push_back(ab);

    for (auto const& ab : mAbs)
        ab->setObjectLabel(ab->getAttributes()[0].name + " of " + "Perlin");

    mEab = glow::ElementArrayBuffer::create(mIndices);
    mEab->setObjectLabel("Heightamp");
    mVao = glow::VertexArray::create(mAbs, mEab, GL_TRIANGLE_STRIP);
    mVao->setObjectLabel("Heightmap");
}

void MultiLayeredHeightmap::FillData(std::vector<float>& heights)
{
    const uint32_t restart = 65535;
    mPositions.resize(mNumberOfVertices);
    mColors.resize(mNumberOfVertices);
    mTexCoords.resize(mNumberOfVertices);
    mIndices.resize(mNumberOfVertices);
    mNormals.resize(mNumberOfVertices);
    int dimX = mHeightmapDimensions.x, dimY = mHeightmapDimensions.y;

#define CURRPOS i*dimY + j
    for(int i = 0; i < dimY; i++)
    {
        for(int j = 0; j < dimX; j++)
        {
            mPositions.at(CURRPOS) = {i, heights.at(i*dimY + j), j};
            mNormals.at(CURRPOS) = {0, 1, 0};

            glm::vec2 normalizedCoord((float)j / dimX, (float)i / dimY);
            mTexCoords.at(CURRPOS) = {normalizedCoord.x, normalizedCoord.y};
            mNormals.at(CURRPOS) = {0, 1, 0};
            mColors.at(CURRPOS) = {1.0f, 1.0f, 1.0f, 1.0f};

            if(i != dimY - 1)
            {
                mIndices.push_back(CURRPOS);
                mIndices.push_back(CURRPOS + dimY);
            }
        }
        mIndices.push_back(restart);
    }
}
#undef CURRPOS

float MultiLayeredHeightmap::getMfHeightScale() const
{
    return mfHeightScale;
}

glow::SharedVertexArray MultiLayeredHeightmap::LoadHeightmap(const char *filename, unsigned char bitsPerPixel){

    //===========verifies the file we are trying to load exists and it is the size we are expecting based on the passed-in parameters===========

    if(!(std::experimental::filesystem::exists(filename)) ){
        std::cerr << "Could not find file: " << filename << std::endl;
        return 0;
    }

    std::FILE *file =NULL;
    if((file = std::fopen(filename, "rb"))==NULL)
        std::cerr << "Could not open specified file" << std::endl;
    else
        std::cerr << "File opened successfully" << std::endl;

    const unsigned int bytesPerPixel = bitsPerPixel / 8;
    const unsigned int fileSize =  getFileSize( file );

    float resolution = fileSize / bytesPerPixel;
    unsigned int width = glm::sqrt(resolution);
    unsigned int height = width;


    if ( width * height != resolution ){
            std::cerr << "Expected quadratic resolution! Resolution: " << resolution << " -> " << width <<"x"<< height << std::endl;
            return NULL;
    }

    mHeightmapDimensions = {width, height};
    mNumberOfVertices = resolution;

    //===========load the height map data from the RAW texture into a float array===========
    unsigned char heightMap[fileSize];
    std::fread(heightMap, fileSize, 1, file);

    std::vector<float> heights;
    heights.resize(mNumberOfVertices);

    for(size_t i = 0; i < heights.size(); i++)
        heights.at(i) = (float) heightMap[i] / 255.0f;

    //===========set up buffers===========
    FillData(heights);
    MakeVertexArray();

    return mVao;

}

glow::SharedVertexArray MultiLayeredHeightmap::GenerateTerrain(NoiseGenerator *generator, unsigned int dimX, unsigned int dimY, unsigned int octaves, float freqScale, float maxHeight)
{
    mHeightmapDimensions = glm::uvec2(dimX, dimY);
    mNumberOfVertices = dimX * dimY;
    std::vector<float> heights;
    heights.reserve(dimX * dimY);

    for(auto i = 0u; i < dimY; i++)
    {
        for(auto j = 0u; j < dimX; j++)
        {
            glm::vec2 normalizedCoord((float)i / dimY, (float)j/dimX);

            float x = 10 * normalizedCoord.x,  y = 10 * normalizedCoord.y;
            heights.push_back(0.0f);
            float amp = maxHeight;
            for(auto oct = 0u; oct < octaves; oct++)
            {
                heights.back() += generator->noise(x, y, 0.8f) * amp;
                x /= freqScale; y /= freqScale; amp *= freqScale;
            }
        }
    }

    FillData(heights);
    MakeVertexArray();

    return mVao;
}



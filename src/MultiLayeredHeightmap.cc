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
#define ENABLE_SLOPE_BASED_BLEND 0
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

    mTexture.resize(textureName.size());
    mSurface.resize(textureName.size());

    //std::cout << "texname0 = "<<textureName[0]<< std::endl;

    for(int i = 0; i < textureName.size(); i++){
        mTexture[i] = (glow::TextureData::createFromFile(textureName[i], glow::ColorSpace::sRGB));
        mSurface[i] = mTexture[i]->getSurfaces()[0];
        mSurface[i]->setOffsetZ(i);
        mTexture[0]->addSurface(mSurface[i]);
    }

    mTexture[0]->setTarget(GL_TEXTURE_2D_ARRAY);
    mTexture[0]->setDepth(mSurface.size());

    return glow::Texture2DArray::createFromData(mTexture[0]);
}

glow::SharedTexture2DArray MultiLayeredHeightmap::LoadNormal(std::vector<std::string> normalName){

    mTextureNormal.resize(normalName.size());
    mNormalSurface.resize(normalName.size());

    //std::cout << "texname0 = "<<normalName[0]<< std::endl;

    for(int i = 0; i < normalName.size(); i++){
        mTextureNormal[i] = (glow::TextureData::createFromFile(normalName[i], glow::ColorSpace::Linear));
        mNormalSurface[i] = mTextureNormal[i]->getSurfaces()[0];
        mNormalSurface[i]->setOffsetZ(i);
        mTextureNormal[0]->addSurface(mNormalSurface[i]);
    }

    mTextureNormal[0]->setTarget(GL_TEXTURE_2D_ARRAY);
    mTextureNormal[0]->setDepth(mNormalSurface.size());

    return glow::Texture2DArray::createFromData(mTextureNormal[0]);
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
    ab->bind().setData(normals_final);
    mAbs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec4>("aColor");
    ab->bind().setData(mColors);
    mAbs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec2>("aTexCoord");
    ab->bind().setData(mTexCoords);
    mAbs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aTangent");
    ab->bind().setData(tangents_final);
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

    normals1.resize(mNumberOfVertices);
    normals2.resize(mNumberOfVertices);
    normals_final.resize(mNumberOfVertices);
    tangents1.resize(mNumberOfVertices);
    tangents2.resize(mNumberOfVertices);
    tangents_final.resize(mNumberOfVertices);

    int dimX = mHeightmapDimensions.x, dimY = mHeightmapDimensions.y;

    float terrainWidth = ( dimX - 1 ) * mfBlockScale;
    float terrainHeight = ( dimY - 1 ) * mfBlockScale;

    float halfTerrainWidth = terrainWidth * 0.5f;
    float halfTerrainHeight = terrainHeight * 0.5f;

    float fTextureU = float(dimX)*0.1f;
    float fTextureV = float(dimY)*0.1f;


#define CURRPOS i*dimX + j

    for(int i = 0; i < dimY; ++i)
    {
        for(int j = 0; j < dimX; ++j)
        {

            //float x = 10 * ((float)i / dimY), y = 10 * ((float)j/dimX);

            float S = ( j / (float)(dimX - 1) );
            float T = ( i / (float)(dimY - 1) );

            float X = ( S * terrainWidth ) - halfTerrainWidth;
            float Y = heights.at(CURRPOS) * 30;
            float Z = ( T * terrainHeight ) - halfTerrainHeight;

            normals_final.at(CURRPOS) = glm::vec3(0);
            mPositions.at(CURRPOS) = glm::vec3(X, Y, Z);
            mTexCoords.at(CURRPOS) = glm::vec2(S*fTextureU, T*fTextureV);

             if(i != dimY - 1)
             {
                 mIndices.push_back(CURRPOS);
                 mIndices.push_back((i+1) * dimY + j);
             }
        }

        mIndices.push_back(restart);
    }
    CalculateNormalsTangents(dimX, dimY);
}
#undef CURRPOS

void MultiLayeredHeightmap::CalculateNormalsTangents(int dimX, int dimY){

    for ( unsigned int j = 0; j < dimY-1; j++ )
    {
        for ( unsigned i = 0; i < dimX-1; i++ )
        {
            unsigned int index = ( j * dimX ) + i;

            glm::vec3 vTriangle0[] =
            {
                mPositions.at((j * dimX ) + i),
                mPositions.at((j+1) * dimX  + i),
                mPositions.at((j+1) * dimX + i+1)
            };

            glm::vec3 vTriangle1[] =
            {
                 mPositions.at((j+1) * dimX + i+1),
                 mPositions.at((j* dimX) + i+1),
                 mPositions.at((j * dimX ) + i)
            };

            glm::vec2 vUV0[] =
            {
                mTexCoords.at((j * dimX ) + i),
                mTexCoords.at((j+1) * dimX  + i),
                mTexCoords.at((j+1) * dimX + i+1)
            };

            glm::vec2 vUV1[] =
            {
                mTexCoords.at((j+1) * dimX + i+1),
                mTexCoords.at((j* dimX) + i+1),
                mTexCoords.at((j * dimX ) + i)
            };

            //normals
            glm::vec3 vTriangleNorm0 = glm::cross(vTriangle0[0]-vTriangle0[1], vTriangle0[1]-vTriangle0[2]);
            glm::vec3 vTriangleNorm1 = glm::cross(vTriangle1[0]-vTriangle1[1], vTriangle1[1]-vTriangle1[2]);

            normals1.at(index) = glm::normalize(vTriangleNorm0);
            normals2.at(index) = glm::normalize(vTriangleNorm1);

            //tangents
            glm::vec3 deltaPos1 = vTriangle0[0]-vTriangle0[1];
            glm::vec3 deltaPos2 = vTriangle0[1]-vTriangle0[2];

            glm::vec3 deltaPos3 = vTriangle1[0]-vTriangle1[1];
            glm::vec3 deltaPos4 = vTriangle1[1]-vTriangle1[2];

            glm::vec2 deltaUV1 = vUV0[0]-vUV0[1];
            glm::vec2 deltaUV2 = vUV0[1]-vUV0[2];

            glm::vec2 deltaUV3 = vUV1[0]-vUV1[1];
            glm::vec2 deltaUV4 = vUV1[1]-vUV1[2];

            float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

            glm::vec3 tangent1;
            glm::vec3 tangent2;

            tangent1 = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
            tangent2 = (deltaPos3 * deltaUV4.y - deltaPos4 * deltaUV3.y)*r;

            tangents1.at(index) = tangent1;
            tangents2.at(index) = tangent2;

        }
    }


    for ( unsigned int i = 0; i < dimY; ++i )
    {
        for ( unsigned j = 0; j < dimX; ++j )
        {
            glm::vec3 tempNormals = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 tempTangents = glm::vec3(0.0f, 0.0f, 0.0f);

            // Look for upper-left triangles
            if(j != 0 && i != 0){
                tempNormals += normals1.at(( (i-1) * dimX ) + j - 1);
                tempNormals += normals2.at(( (i-1) * dimX ) + j - 1);

                tempTangents += tangents1.at(( (i-1) * dimX ) + j - 1);
                tempTangents += tangents2.at(( (i-1) * dimX ) + j - 1);
            }

            // Look for upper-right triangles
            if(i != 0 && j != dimX-1){
                tempNormals += normals1.at(( (i-1) * dimX ) + j);
                tempNormals += normals2.at(( (i-1) * dimX ) + j);

                tempTangents += tangents1.at(( (i-1) * dimX ) + j);
                tempTangents += tangents2.at(( (i-1) * dimX ) + j);
            }

            // Look for bottom-right triangles
            if(i != dimY-1 && j != dimX-1){
                tempNormals += normals1.at(( i * dimX ) + j);
                tempNormals += normals2.at(( i * dimX ) + j);

                tempTangents += tangents1.at(( i * dimX ) + j);
                tempTangents += tangents2.at(( i * dimX ) + j);
            }

            // Look for bottom-left triangles
            if(i != dimY-1 && j != 0){
                tempNormals += normals1.at(( i * dimX ) + j - 1);
                tempNormals += normals2.at(( i * dimX ) + j - 1);

                tempTangents += tangents1.at(( i * dimX ) + j - 1);
                tempTangents += tangents2.at(( i * dimX ) + j - 1);
            }

            tempNormals = glm::normalize(tempNormals);
            normals_final.at(( i * dimX ) + j) = tempNormals; // Store final normal of j-th vertex in i-th row


            tangents_final.at(( i * dimX ) + j) = tempTangents;
        }
    }
}

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
        heights.at(i) = (unsigned char)heightMap[i * bytesPerPixel]/(float)0xff;


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
            float amp = 5;
            float temp = 0.0f;
            for(auto oct = 0u; oct < 4; oct++)
            {
                temp += generator->noise(x, y, 0.8f) * amp;
                x /= freqScale; y /= freqScale; amp *= freqScale;
            }
             heights.back() =  (temp * 0.2f) + 0.85f;
        }
    }

    FillData(heights);

    MakeVertexArray();

    return mVao;
}



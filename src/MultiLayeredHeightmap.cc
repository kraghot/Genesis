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
    m_fHeightScale(heightScale),
    m_fBlockScale(blockScale),
    m_HeightmapDimensions(0,0)
    {

}

MultiLayeredHeightmap::~MultiLayeredHeightmap(){

}


glow::SharedTexture2DArray MultiLayeredHeightmap::LoadTexture(std::vector<std::string> textureName){

    tex.resize(textureName.size());
    surface.resize(textureName.size());

    //std::cout << "texname0 = "<<textureName[0]<< std::endl;

    for(int i = 0; i < textureName.size(); i++){
        tex[i] = (glow::TextureData::createFromFile(textureName[i], glow::ColorSpace::sRGB));
        surface[i] = tex[i]->getSurfaces()[0];
        surface[i]->setOffsetZ(i);
        tex[0]->addSurface(surface[i]);
    }

    tex[0]->setTarget(GL_TEXTURE_2D_ARRAY);
    tex[0]->setDepth(surface.size());

    return glow::Texture2DArray::createFromData(tex[0]);
}

glow::SharedTexture2DArray MultiLayeredHeightmap::LoadNormal(std::vector<std::string> normalName){

    normal.resize(normalName.size());
    nsurface.resize(normalName.size());

    //std::cout << "texname0 = "<<normalName[0]<< std::endl;

    for(int i = 0; i < normalName.size(); i++){
        normal[i] = (glow::TextureData::createFromFile(normalName[i], glow::ColorSpace::Linear));
        nsurface[i] = normal[i]->getSurfaces()[0];
        nsurface[i]->setOffsetZ(i);
        normal[0]->addSurface(nsurface[i]);
    }

    normal[0]->setTarget(GL_TEXTURE_2D_ARRAY);
    normal[0]->setDepth(nsurface.size());

    return glow::Texture2DArray::createFromData(normal[0]);
}

glow::SharedVertexArray MultiLayeredHeightmap::LoadHeightmap(const char *filename, unsigned char bitsPerPixel){

    //===========verifies the file we are trying to load exists and it is the size we are expecting based on the passed-in parameters===========

    if(!(std::experimental::filesystem::exists(filename)) ){
        std::cerr << "Could not find file: " << filename << std::endl;
        return 0;
    }

    std::FILE *file =NULL;
    if((file = std::fopen(filename, "rb"))==NULL)   std::cerr << "Could not open specified file" << std::endl;
            else
                std::cerr << "File opened successfully" << std::endl;

    const unsigned int bytesPerPixel = bitsPerPixel / 8;
    //const unsigned int expectedFileSize = ( bytesPerPixel * width * height );
    const unsigned int fileSize =  getFileSize( file );

    float resolution = fileSize / bytesPerPixel;
    unsigned int width = glm::sqrt(resolution);
    unsigned int height = width;


    if ( width * height != resolution ){
            std::cerr << "Expected quadratic resolution! Resolution: " << resolution << " -> " << width <<"x"<< height << std::endl;
            return NULL;
    }

    //===========load the height map data from the RAW texture into an unsigned char data array===========

    unsigned char* heightMap = new unsigned char[fileSize];

    std::fread(heightMap, fileSize, 1, file);

    //===========set up buffers===========


    unsigned long int numVerts = height * width;

    const uint32_t restart = 65535;

    positions.resize(numVerts);
    colors.resize(numVerts);
    tex0buffer.resize(numVerts);
    indices.resize(numVerts);
    normals1.resize(numVerts);
    normals2.resize(numVerts);
    normals_final.resize(numVerts);
    tangents1.resize(numVerts);
    tangents2.resize(numVerts);
    tangents_final.resize(numVerts);

    m_HeightmapDimensions = glm::uvec2(width, height);

    // Size of the terrain in world units
    float terrainWidth = ( width - 1 ) * m_fBlockScale;
    float terrainHeight = ( height - 1 ) * m_fBlockScale;

    float halfTerrainWidth = terrainWidth * 0.5f;
    float halfTerrainHeight = terrainHeight * 0.5f;

    float fTextureU = float(width)*0.1f;
    float fTextureV = float(height)*0.1f;

    for ( unsigned int j = 0; j < height; ++j )
        {
            for ( unsigned i = 0; i < width; ++i )
            {
                unsigned int index = ( j * width ) + i;
                assert( index * bytesPerPixel < fileSize );
                float heightValue = getHeightValue( &heightMap[index * bytesPerPixel], bytesPerPixel );

                float S = ( i / (float)(width - 1) );
                float T = ( j / (float)(height - 1) );

                float X = ( S * terrainWidth ) - halfTerrainWidth;
                float Y = heightValue * m_fHeightScale ;
                float Z = ( T * terrainHeight ) - halfTerrainHeight;

                // Blend 3 textures depending on the height of the terrain
                float tex0Contribution = 1.0f - getPercentage( heightValue, 0.0f, 0.75f );
                float tex2Contribution = 1.0f - getPercentage( heightValue, 0.75f, 1.0f );

                normals_final.at(index) = glm::vec3(0);
                positions.at(index) = glm::vec3(X, Y, Z);
                tex0buffer.at(index) = glm::vec2(S*fTextureU, T*fTextureV);

#if ENABLE_MULTITEXTURE
                colors.at(index) = glm::vec4(tex0Contribution, tex0Contribution, tex0Contribution, tex2Contribution);
#else
                colors.at(index) = glm::vec4(1.0f);
#endif


                if(j != height - 1)
                {
                    indices.push_back(j* width + i);
                    indices.push_back((j+1) * height + i);
                }


            }
            indices.push_back(restart);
        }

    glm::vec3 tangent1;
    glm::vec3 tangent2;

    //=========Calculate normals and tangents=========//

    for ( unsigned int j = 0; j < height-1; j++ )
        {
            for ( unsigned i = 0; i < width-1; i++ )
            {
                unsigned int index = ( j * width ) + i;


                glm::vec3 vTriangle0[] =
                {
                    positions.at((j * width ) + i),
                    positions.at((j+1) * width  + i),
                    positions.at((j+1) * width + i+1)
                    };
                glm::vec3 vTriangle1[] =
                {
                    positions.at((j+1) * width + i+1),
                    positions.at((j* width) + i+1),
                    positions.at((j * width ) + i)
                    };

                glm::vec2 vUV0[] =
                {
                    tex0buffer.at((j * width ) + i),
                    tex0buffer.at((j+1) * width  + i),
                    tex0buffer.at((j+1) * width + i+1)
                    };

                glm::vec2 vUV1[] =
                {
                    tex0buffer.at((j+1) * width + i+1),
                    tex0buffer.at((j* width) + i+1),
                    tex0buffer.at((j * width ) + i)
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
                tangent1 = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
                tangent2 = (deltaPos3 * deltaUV4.y - deltaPos4 * deltaUV3.y)*r;

                tangents1.at(index) = tangent1;
                tangents2.at(index) = tangent2;


            }
    }


    for ( unsigned int i = 0; i < height; ++i )
        {
            for ( unsigned j = 0; j < width; ++j )
            {
                glm::vec3 tempNormals = glm::vec3(0.0f, 0.0f, 0.0f);
                glm::vec3 tempTangents = glm::vec3(0.0f, 0.0f, 0.0f);

                // Look for upper-left triangles
                if(j != 0 && i != 0){
                    tempNormals += normals1.at(( (i-1) * width ) + j - 1);
                    tempNormals += normals2.at(( (i-1) * width ) + j - 1);

                    tempTangents += tangents1.at(( (i-1) * width ) + j - 1);
                    tempTangents += tangents2.at(( (i-1) * width ) + j - 1);
                }

                // Look for upper-right triangles
                if(i != 0 && j != width-1){
                    tempNormals += normals1.at(( (i-1) * width ) + j);
                    tempNormals += normals2.at(( (i-1) * width ) + j);

                    tempTangents += tangents1.at(( (i-1) * width ) + j);
                    tempTangents += tangents2.at(( (i-1) * width ) + j);
                }

                // Look for bottom-right triangles
                if(i != height-1 && j != width-1){
                    tempNormals += normals1.at(( i * width ) + j);
                    tempNormals += normals2.at(( i * width ) + j);

                    tempTangents += tangents1.at(( i * width ) + j);
                    tempTangents += tangents2.at(( i * width ) + j);
                }

                // Look for bottom-left triangles
                if(i != height-1 && j != 0){
                    tempNormals += normals1.at(( i * width ) + j - 1);
                    tempNormals += normals2.at(( i * width ) + j - 1);

                    tempTangents += tangents1.at(( i * width ) + j - 1);
                    tempTangents += tangents2.at(( i * width ) + j - 1);
                }

                tempNormals = glm::normalize(tempNormals);
                normals_final.at(( i * width ) + j) = tempNormals; // Store final normal of j-th vertex in i-th row


                tangents_final.at(( i * width ) + j) = tempTangents;
            }
    }

    //====================================//

    std::vector<glow::SharedArrayBuffer> abs;

    auto ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aPosition");
    ab->bind().setData(positions);
    abs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aNormal");
    ab->bind().setData(normals_final);
    abs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec4>("aColor");
    ab->bind().setData(colors);
    abs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec2>("aTexCoord");
    ab->bind().setData(tex0buffer);
    abs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aTangent");
    ab->bind().setData(tangents_final);
    abs.push_back(ab);

    for (auto const& ab : abs)
        ab->setObjectLabel(ab->getAttributes()[0].name + " of " + "Heightmap");

    auto eab = glow::ElementArrayBuffer::create(indices);
    eab->setObjectLabel("Heightmap");
    auto va = glow::VertexArray::create(abs, eab, GL_TRIANGLE_STRIP);
    va->setObjectLabel("Heightmap");



    std::cout << "Terrain has been loaded!" << std::endl;
    //delete [] heightMap;

    return va;

}



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
    m_LocalToWorldMatrix(1),
    m_HeightmapDimensions(0,0),
    m_fHeightScale(heightScale),
    m_fBlockScale(blockScale)
    {

}

MultiLayeredHeightmap::~MultiLayeredHeightmap(){

}


bool MultiLayeredHeightmap::LoadTexture(std::string &filename, unsigned int textureStage){


    terrainColor[textureStage] = glow::Texture2DArray::createFromFile(filename, glow::ColorSpace::sRGB);
    terrainNormal[textureStage] = glow::Texture2DArray::createFromFile(filename, glow::ColorSpace::Linear);

    if(terrainColor[textureStage] != 0 && terrainNormal[textureStage] !=0){
        printf("Terrain color successfully loaded.");
        glBindTexture(terrainColor[textureStage]->getTarget(), terrainColor[textureStage]->getObjectName()); //mozda zamjeniti sa
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        //glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
        glBindTexture( GL_TEXTURE_2D, 0 );
        return 1;
    }
    else {
        printf("Error loading texture for texture stage %u",textureStage);
        return 0;
    }
}

glow::SharedVertexArray MultiLayeredHeightmap::LoadHeightmap(const char *filename, unsigned char bitsPerPixel, unsigned int width, unsigned int height){

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
    const unsigned int expectedFileSize = ( bytesPerPixel * width * height );
    const unsigned int fileSize =  getFileSize( file );

    if ( expectedFileSize != fileSize ){
            std::cerr << "Expected file size [" << expectedFileSize << " bytes] differs from actual file size [" << fileSize << " bytes]" << std::endl;
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
    normals.resize(numVerts);

    m_HeightmapDimensions = glm::uvec2(width, height);

    // Size of the terrain in world units
    float terrainWidth = ( width - 1 ) * m_fBlockScale;
    float terrainHeight = ( height - 1 ) * m_fBlockScale;

    float halfTerrainWidth = terrainWidth * 0.5f;
    float halfTerrainHeight = terrainHeight * 0.5f;

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
                float Y = heightValue * m_fHeightScale *(-1);
                float Z = ( T * terrainHeight ) - halfTerrainHeight;

                // Blend 3 textures depending on the height of the terrain
                float tex0Contribution = 1.0f - getPercentage( heightValue, 0.0f, 0.75f );
                float tex2Contribution = 1.0f - getPercentage( heightValue, 0.75f, 1.0f );

                normals.at(index) = glm::vec3(0);
                positions.at(index) = glm::vec3(X, Y, Z);

#if ENABLE_MULTITEXTURE
                colors.at(index) = glm::vec4(tex0Contribution, tex0Contribution, tex0Contribution, tex2Contribution);
#else
                colors.at(index) = glm::vec4(1.0f);
#endif
                tex0buffer.at(index) = glm::vec2(S, T);

                if(j != height - 1)
                {
                    indices.push_back(j* width + i);
                    indices.push_back((j+1) * height + i);
                }


            }
            indices.push_back(restart);
        }

    std::vector<glow::SharedArrayBuffer> abs;

    auto ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aPosition");
    ab->bind().setData(positions);
    abs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aNormal");
    ab->bind().setData(normals);
    abs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec4>("aColor");
    ab->bind().setData(colors);
    abs.push_back(ab);

    for (auto const& ab : abs)
        ab->setObjectLabel(ab->getAttributes()[0].name + " of " + "Perlin");

    auto eab = glow::ElementArrayBuffer::create(indices);
    eab->setObjectLabel("Perlin");
    auto va = glow::VertexArray::create(abs, eab, GL_TRIANGLE_STRIP);
    va->setObjectLabel("Perlin");



    std::cout << "Terrain has been loaded!" << std::endl;
    //delete [] heightMap;

    return va;

}




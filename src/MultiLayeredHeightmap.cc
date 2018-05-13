#include "MultiLayeredHeightmap.hh"
#include "GlowApp.hh"

#include <stdio.h>
#include <experimental/random>

#ifndef ENABLE_SLOPE_BASED_BLEND
#define ENABLE_SLOPE_BASED_BLEND 1
#endif

typedef std::basic_ios<char> ios;

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

    for(auto i = 0u; i < textureName.size(); i++){
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

    for(auto i = 0u; i < normalName.size(); i++){
        mTextureNormal[i] = (glow::TextureData::createFromFile(normalName[i], glow::ColorSpace::Linear));
        mNormalSurface[i] = mTextureNormal[i]->getSurfaces()[0];
        mNormalSurface[i]->setOffsetZ(i);
        mTextureNormal[0]->addSurface(mNormalSurface[i]);
    }

    mTextureNormal[0]->setTarget(GL_TEXTURE_2D_ARRAY);
    mTextureNormal[0]->setDepth(mNormalSurface.size());

    return glow::Texture2DArray::createFromData(mTextureNormal[0]);
}

void MultiLayeredHeightmap::DumpHeightmapToFile()
{
    std::ostringstream filename;
    filename << "terrain-heightmap-8bbp-" << mHeightmapDimensions.x << "x" << mHeightmapDimensions.y << ".raw";
    std::ofstream file (filename.str(), std::ios::out | std::ios::binary);
    std::vector<uint8_t> byteField;
    byteField.reserve(mNumberOfVertices);
    for(auto it : mPositions)
    {
        if(it.y < 0)
            it.y *= (-1);

        /// @todo Correct for y scaling
        byteField.push_back(it.y * (255));
    }
    file.write((char *)byteField.data(), byteField.size());
}

void MultiLayeredHeightmap::DumpSplatmapToFile()
{

    std::ostringstream filename;
    filename << "terrain-splatmap-8bbp-" << mHeightmapDimensions.x << "x" << mHeightmapDimensions.y << ".raw";
    std::ofstream file (filename.str(), std::ios::out | std::ios::binary);
    std::vector<glm::vec4> byteField;
    byteField.reserve(mNumberOfVertices);

    byteField = mSplatmap;

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

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<float>("aSlopeY");
    ab->bind().setData(slope_y);
    mAbs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec2>("aHeightCoord");
    ab->bind().setData(mHeightCoords);
    mAbs.push_back(ab);

    for (auto const& ab : mAbs)
        ab->setObjectLabel(ab->getAttributes()[0].name + " of " + "Perlin");

    mEab = glow::ElementArrayBuffer::create(mIndices);
    mEab->setObjectLabel("Heightamp");
    mVao = glow::VertexArray::create(mAbs, mEab, GL_TRIANGLE_STRIP);
    mVao->setObjectLabel("Heightmap");

    mDisplacementTexture = glow::Texture2D::create(mHeightmapDimensions.x, mHeightmapDimensions.y, GL_R32F);
    mDisplacementTexture->bind().setData(GL_R32F, mHeightmapDimensions.x, mHeightmapDimensions.y, GL_RED, GL_FLOAT, mDisplacement.data());
    mDisplacementTexture->bind().generateMipmaps();

    mSplatmapTexture = glow::Texture2D::create(mHeightmapDimensions.x, mHeightmapDimensions.y, GL_RGBA);
    mSplatmapTexture->bind().setData(GL_RGBA, mHeightmapDimensions.x, mHeightmapDimensions.y, mSplatmap);
    mSplatmapTexture->bind().generateMipmaps();
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
    mDisplacement.resize(mNumberOfVertices);
    mHeightCoords.resize(mNumberOfVertices);

    slope_y.resize(mNumberOfVertices);

    mHeightCoords.resize(mNumberOfVertices);

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
        //    mPositions.at(CURRPOS) = {i, 0.0f, j};
            mNormals.at(CURRPOS) = {0, 1, 0};
            mDisplacement.at(CURRPOS) = heights.at(CURRPOS);

            //float x = 10 * ((float)i / dimY), y = 10 * ((float)j/dimX);

            float S = ( j / (float)(dimX - 1) );
            float T = ( i / (float)(dimY - 1) );

            float X = ( S * terrainWidth ) - halfTerrainWidth;
            //float Y = heights.at(CURRPOS) * 30;
            float Y = 0.0f;
            float Z = ( T * terrainHeight ) - halfTerrainHeight;

            normals_final.at(CURRPOS) = glm::vec3(0);
            mPositions.at(CURRPOS) = glm::vec3(X, Y, Z);
            mTexCoords.at(CURRPOS) = glm::vec2(S * fTextureU, T * fTextureV);
            mHeightCoords.at(CURRPOS) = glm::vec2(S, T);

             if(i != dimY - 1)
             {
                 mIndices.push_back(CURRPOS);
                 mIndices.push_back((i+1) * dimY + j);
             }
        }

        mIndices.push_back(restart);
    }
    CalculateNormalsTangents(dimX, dimY);
    LoadSplatmap();
}

glow::SharedTexture2D MultiLayeredHeightmap::GetDisplacementTexture() const
{
    return mDisplacementTexture;
}

std::vector<glm::uvec2> MultiLayeredHeightmap::GetNeighborhood(unsigned int i, unsigned int j)
{
    std::vector<glm::uvec2> ret;
    ret.push_back(glm::uvec2((i - 1) % mHeightmapDimensions.x, j));
    ret.push_back(glm::uvec2((i + 1) % mHeightmapDimensions.x, j));
    ret.push_back(glm::uvec2(i, (j - 1) % mHeightmapDimensions.y));
    ret.push_back(glm::uvec2(i, (j + 1) % mHeightmapDimensions.y));
    return ret;
}

std::vector<glm::uvec2> MultiLayeredHeightmap::GetNeighborhood(glm::uvec2 coord)
{
    return GetNeighborhood(coord.x, coord.y);
}

glm::uvec2 MultiLayeredHeightmap::GetLowestNeigh(std::vector<glm::uvec2> &neigh)
{
    unsigned int lowestIndex = -1;
    float lowestDepth = std::numeric_limits<float>::max();

    for(auto i = 0u; i < 4; i++)
    {
        auto currDepth = GetDisplacementAt(neigh.at(i));
        if (currDepth < lowestDepth)
        {
            lowestDepth = currDepth;
            lowestIndex = i;
        }
    }

    return neigh.at(lowestIndex);
}

void MultiLayeredHeightmap::ThermalErodeTerrain()
{
#define LOC(a, b) b * mHeightmapDimensions.y + a

    float T = 8.0f / (float) mHeightmapDimensions.x;
    int counter = 0;
#pragma omp for
    for(auto i = 0u; i < mHeightmapDimensions.y; i++)
    {
        for(auto j = 0u; j < mHeightmapDimensions.x; j++)
        {
            float dMax = 0;
            glm::vec2 l;
            auto neigh = GetNeighborhood(j, i);
            for(auto it : neigh)
            {
                float diff = mDisplacement.at(LOC(j, i)) - mDisplacement.at(LOC(it.x, it.y));
                if(diff > dMax)
                {
                    dMax = diff;
                    l = it;
                }
            }

            if(0 < dMax && dMax <= T)
            {
                float deltaH = dMax;
                mDisplacement.at(LOC(j, i)) -= deltaH;
                mDisplacement.at(LOC(l.x, l.y)) += deltaH;
                counter++;
            }
        }
    }
    std::cout << counter << std::endl;
}

void MultiLayeredHeightmap::HydraulicErodeTerrain()
{
    const float rainfall = 0.01f;
    const float sediment = 0.01f * mfHeightScale;
#pragma omp for
    for(auto i = 0u; i < mHeightmapDimensions.y; i++)
    {
        for(auto j = 0u; j < mHeightmapDimensions.x; j++)
        {
            mWaterLevel.at(LOC(j, i)) += rainfall;
            float a = mDisplacement.at(LOC(j, i)) + mWaterLevel.at(LOC(j, i));
            auto neigh = GetNeighborhood(j, i);
            glm::uvec2 lowestNeigh = neigh.at(0);
            float dmax = -1.0f;
            for (auto it: neigh)
            {
                float ai = mDisplacement.at(LOC(it.x, it.y)) + mWaterLevel.at(LOC(it.x, it.y));
                float di = a - ai;
                if (di > 0.0f)
                {
                    if(di > dmax)
                    {
                        dmax = di;
                        lowestNeigh = it;

                    }
                }
            }

            if(dmax <= 0.0f)
                continue;

            float sedimentToTransport = sediment * (std::min(mWaterLevel.at(LOC(lowestNeigh.x, lowestNeigh.y)), dmax));
            mDisplacement.at(LOC(j, i)) -= sedimentToTransport;
            mDisplacement.at(LOC(lowestNeigh.x, lowestNeigh.y)) += sediment;
            mWaterLevel.at(LOC(j, i)) *= 0.5f;
        }
    }


}

void MultiLayeredHeightmap::DropletErodeTerrain(glm::vec2 coordinates, float strength)
{
          // Soil carry capacity
    float Cq=10,
          minSlope=0.05f,
          // Water evaporation speed
          Cw=0.001f,
          // Erosion speed
          Cr=0.9f,
          // Deposition speed
          Cd=0.02f,
          // Direction inertia
          Ci=0.1f,
          // Gravity acceleration
          Cg=20;
    const unsigned maxPathLength = mHeightmapDimensions.x;
    std::vector<float> erosion;
    erosion.resize(mNumberOfVertices);

    // Sediment, velocity, water
    float s=0, v=0, w=strength;


//    // Left, Right, Bottom, Top
//    float hl = GetDisplacementAt(neight.at(NeighSide::Left));
//    float hr = GetDisplacementAt(neight.at(NeighSide::Right));
//    float hu = GetDisplacementAt(neight.at(NeighSide::Up));
//    float hd = GetDisplacementAt(neight.at(NeighSide::Down));

    glm::uvec2 currPos = coordinates;
    for(auto i=0u; i < maxPathLength; i++)
    {
        auto neigh = GetNeighborhood(currPos);
        auto next = GetLowestNeigh(neigh);

        float heightDifference = GetDisplacementAt(currPos) - GetDisplacementAt(next);
        std::cout << "Currently at position " << currPos.x << " " << currPos.y << std::endl;
        std::cout << "Height " << GetDisplacementAt(currPos) << " " << heightDifference << std::endl;

        /// @todo Add handling when the differece is negligible
        /// Move in random direction
//        if(heightDifference <= 0.0001)
//        {
//            int random = std::experimental::randint(0, 3);
//            currPos = neigh.at(random);
//            continue;
//        }

        mSplatmap.at(LOC(currPos.x, currPos.y)).a = 0.1f * (i+1);

        // If this location is the deepest in the neigh try to deposit everything
        if(heightDifference < 0.0001)
        {
            // Deposit everthing up to the height difference in order to prevent it being heigher than the neighs
            if(/*s < heightDifference*/1)
            {
                AddDisplacementAt(currPos, s);
                s = 0;
                break;
            }
            else
            {
                AddDisplacementAt(currPos, heightDifference);
                s -= heightDifference;
                break;
            }
        }
        // It is possible to go downhill, calculate if sediment is accumulated
        else
        {
            float slope = std::max(minSlope, heightDifference);
            float accumulate = slope * v * w * Cq;
            if(accumulate > heightDifference)
                accumulate = heightDifference;

            AddDisplacementAt(currPos, -accumulate);
            s += accumulate;
            v = sqrt(v*v + Cg * heightDifference);
        }

        w *= 1 - Cw;
        currPos = next;

        if(i > maxPathLength)
            break;

    }

}

void MultiLayeredHeightmap::CalculateNormalsTangents(int dimX, int dimY){

    for (int j = 0; j < dimY-1; j++ )
    {
        for (int i = 0; i < dimX-1; i++ )
        {
            unsigned int index = ( j * dimX ) + i;

            glm::vec3 vTriangle0[] =
            {
                mPositions.at((j * dimX ) + i),
                mPositions.at((j+1) * dimX  + i),
                mPositions.at((j+1) * dimX + i+1)
            };

            vTriangle0[0].y = mDisplacement.at((j * dimX ) + i);
            vTriangle0[1].y = mDisplacement.at((j+1) * dimX  + i);
            vTriangle0[2].y = mDisplacement.at((j+1) * dimX + i+1);

            glm::vec3 vTriangle1[] =
            {
                 mPositions.at((j+1) * dimX + i+1),
                 mPositions.at((j* dimX) + i+1),
                 mPositions.at((j * dimX ) + i)
            };

            vTriangle1[0].y = mDisplacement.at((j+1) * dimX + i+1);
            vTriangle1[1].y = mDisplacement.at((j* dimX) + i+1);
            vTriangle1[2].y = mDisplacement.at((j * dimX ) + i);

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


    for (int i = 0; i < dimY; ++i )
    {
        for (int j = 0; j < dimX; ++j )
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

            //in radians
            slope_y.at(( i * dimX ) + j) = glm::acos(tempNormals.y);

        }
    }
}

glow::SharedTexture2D MultiLayeredHeightmap::getSplatmapTexture() const
{
    return mSplatmapTexture;
}

float MultiLayeredHeightmap::getMfHeightScale() const
{
    return mfHeightScale;
}

float MultiLayeredHeightmap::GetDisplacementAt(glm::uvec2 pos)
{
    return mDisplacement.at(LOC(pos.x, pos.y));
}

void MultiLayeredHeightmap::AddDisplacementAt(glm::uvec2 pos, float addition)
{
    mDisplacement.at(LOC(pos.x, pos.y)) += addition;
}

#undef LOC

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
            float temp = 0.0f;
            for(auto oct = 0u; oct < octaves; oct++)
            {
                temp += generator->noise(x, y, 0.8f) * amp;
                x /= freqScale; y /= freqScale; amp *= 0.5f;
            }
             heights.back() =  temp + 0.5f;
        }
    }

    FillData(heights);
    mWaterLevel.resize(mNumberOfVertices);
//    for(int i = 0; i < 100; i++)
//    {
//        std::cout << "Iteration " << i << ". Changed heights: ";
//        HydraulicErodeTerrain();
//        ThermalErodeTerrain();

//    }
    DropletErodeTerrain(glm::uvec2(50, 50), 50);
    MakeVertexArray();

    return mVao;
}

void MultiLayeredHeightmap::LoadSplatmap(){

    mSplatmap.resize(mNumberOfVertices);

    const float fRange1 = 0.01f;
    const float fRange2 = 0.01667f;
    const float fRange3 = 0.02333f;
    const float fRange4 = 0.03f;

    float fScale;

    float r;
    float g;
    float b;

    for(unsigned int i = 0; i<mNumberOfVertices; i++){
        r = 0.0f;
        g = 0.0f;
        b = 0.0f;

#if ENABLE_SLOPE_BASED_BLEND
    fScale = slope_y.at(i);
#else
    fScale = mPositions.at(i).y/mfHeightScale;
#endif
#undef ENABLE_SLOPE_BASED_BLEND

        if(fScale >= 0.0 && fScale <= fRange1){
            r = 1.f;
        }

        else if(fScale <= fRange2){
            fScale -= fRange1;
            fScale /= (fRange2-fRange1);

           float fScale2 = fScale;
           fScale = 1.0-fScale;

           r = fScale;
           g = fScale2;

        }

        else if(fScale <= fRange3){
            g = 1.f;
        }

        else if(fScale <= fRange4)
        {
                fScale -= fRange3;
                fScale /= (fRange4-fRange3);

                float fScale2 = fScale;
                fScale = 1.0-fScale;

                g = fScale;
                b = fScale2;
        }

        else{
            b = 1.f;
           }

        mSplatmap.at(i) = {r,g,b,0.0f};
    }


}

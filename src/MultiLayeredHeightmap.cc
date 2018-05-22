#include "MultiLayeredHeightmap.hh"
#include "GlowApp.hh"

#include <stdio.h>
#include <experimental/random>
#include <glm/gtc/quaternion.hpp>
#include<glm/gtx/quaternion.hpp>

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
    std::vector<glm::vec3> byteField;
    byteField.reserve(mNumberOfVertices);

    byteField = mSplatmap;

    file.write((char *)byteField.data(), byteField.size());
}


void MultiLayeredHeightmap::MakeVertexArray()
{

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aPosition");
    ab->bind().setData(mPositions, GL_DYNAMIC_DRAW);
    mAbs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aNormal");
    ab->bind().setData(mNormalsFinal);
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
    ab->bind().setData(mTangentsFinal);
    mAbs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<float>("aSlopeY");
    ab->bind().setData(mSlopeY);
    mAbs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec2>("aHeightCoord");
    ab->bind().setData(mHeightCoords);
    mAbs.push_back(ab);

    ab = glow::ArrayBuffer::create();
    ab->defineAttribute<float>("aHeightBrush");
    ab->bind().setData(mHeightBrush);
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

    mSplatmapTexture = glow::Texture2D::create(mHeightmapDimensions.x, mHeightmapDimensions.y, GL_RGB);
    mSplatmapTexture->bind().setData(GL_RGB, mHeightmapDimensions.x, mHeightmapDimensions.y, mSplatmap);
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

    mNormals1.resize(mNumberOfVertices);
    mNormals2.resize(mNumberOfVertices);
    mNormalsFinal.resize(mNumberOfVertices);
    mTangents1.resize(mNumberOfVertices);
    mTangents2.resize(mNumberOfVertices);
    mTangentsFinal.resize(mNumberOfVertices);
    mDisplacement.resize(mNumberOfVertices);
    mHeightCoords.resize(mNumberOfVertices);

    mSlopeY.resize(mNumberOfVertices);

    mHeightBrush.resize(mNumberOfVertices);

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
            float Y = heights.at(CURRPOS) * 30;
            //float Y = 0.0f;
            float Z = ( T * terrainHeight ) - halfTerrainHeight;

            mNormalsFinal.at(CURRPOS) = glm::vec3(0);
            mPositions.at(CURRPOS) = glm::vec3(X, Y, Z);
            mTexCoords.at(CURRPOS) = glm::vec2(S * fTextureU, T * fTextureV);
            mHeightCoords.at(CURRPOS) = glm::vec2(S, T);
            mHeightBrush.at(CURRPOS) = 0.f;

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
        if (GetDisplacementAt(neigh.at(i)) < lowestDepth)
        {
            lowestDepth = GetDisplacementAt(neigh.at(i));
            lowestIndex = i;
        }
    }

    return neigh.at(lowestIndex);
}

glow::SharedVertexArray MultiLayeredHeightmap::getVao() const
{
    return mVao;
}

glow::SharedVertexArray MultiLayeredHeightmap::getCircleVao() const
{
    return mCircleVao;
}

glm::mat4 MultiLayeredHeightmap::GetCircleRotation()
{
    glm::vec3 upVector(0, 1, 0);
    glm::vec3 xVector = glm::cross(upVector, mIntersectionTriangle.normal);
    glm::mat4 rot = glm::lookAt(intersectionPoint,
                                intersectionPoint + xVector,
                                mIntersectionTriangle.normal);

    return inverse(rot);

}

void MultiLayeredHeightmap::SetTextureBrush(int seletedTexture){

    float sum;

    float Radius2 = mIntersectionRadius * mIntersectionRadius;

        for (unsigned int j = mIntersectionHeight - mIntersectionRadius; j < mIntersectionHeight + mIntersectionRadius; j++){ // 2m world = 2 u heightmapu
            for (unsigned int i = mIntersectionWidth - mIntersectionRadius; i < mIntersectionWidth + mIntersectionRadius; i++){


                float pointPositionx = glm::pow(mPositions.at((j * mHeightmapDimensions.x) + i).x - mPositions.at((mIntersectionHeight * mHeightmapDimensions.x) + mIntersectionWidth).x,2);
                float pointPositiony  = glm::pow(mPositions.at((j * mHeightmapDimensions.x) + i).y -mPositions.at((mIntersectionHeight * mHeightmapDimensions.x) + mIntersectionWidth).y,2);
                float pointPositionz = glm::pow(mPositions.at((j * mHeightmapDimensions.x) + i).z -mPositions.at((mIntersectionHeight * mHeightmapDimensions.x) + mIntersectionWidth).z,2);

                float distance = pointPositionx + pointPositiony + pointPositionz;

                    if(distance < Radius2 && distance > (0.7 * Radius2)){
                        mSplatmap.at(j*mHeightmapDimensions.x + i)[seletedTexture] += 0.2;
                    }

                    else if(distance < (0.7 * Radius2) && distance > (0.5 * Radius2)){
                        mSplatmap.at(j*mHeightmapDimensions.x + i)[seletedTexture] += 0.4;
                    }
                    else if(distance < (0.5 * Radius2)){
                        mSplatmap.at(j*mHeightmapDimensions.x + i)[seletedTexture] += 0.8;
                    }

                    sum = mSplatmap.at(j*mHeightmapDimensions.x + i).x + mSplatmap.at(j*mHeightmapDimensions.x + i).y + mSplatmap.at(j*mHeightmapDimensions.x + i).z;
                    mSplatmap.at(j*mHeightmapDimensions.x + i).y /= sum;
                    mSplatmap.at(j*mHeightmapDimensions.x + i).x /= sum;
                    mSplatmap.at(j*mHeightmapDimensions.x + i).z /= sum;

                }
            }


        mSplatmapTexture->bind().setData(GL_RGB, mHeightmapDimensions.x, mHeightmapDimensions.y, mSplatmap);
        mSplatmapTexture->bind().generateMipmaps();
}

void MultiLayeredHeightmap::SetHeightBrush(float factor){
    float Radius2 = mIntersectionRadius * mIntersectionRadius;

    for (unsigned int j = mIntersectionHeight - mIntersectionRadius; j < mIntersectionHeight + mIntersectionRadius; j++){ // 2m world = 2 u heightmapu
        for (unsigned int i = mIntersectionWidth - mIntersectionRadius; i < mIntersectionWidth + mIntersectionRadius; i++){


            float pointPositionx = glm::pow(mPositions.at((j * mHeightmapDimensions.x) + i).x - mPositions.at((mIntersectionHeight * mHeightmapDimensions.x) + mIntersectionWidth).x,2);
            float pointPositiony  = glm::pow(mPositions.at((j * mHeightmapDimensions.x) + i).y -mPositions.at((mIntersectionHeight * mHeightmapDimensions.x) + mIntersectionWidth).y,2);
            float pointPositionz = glm::pow(mPositions.at((j * mHeightmapDimensions.x) + i).z -mPositions.at((mIntersectionHeight * mHeightmapDimensions.x) + mIntersectionWidth).z,2);

            float distance = pointPositionx + pointPositiony + pointPositionz;

                if(distance < Radius2 && distance > (0.7 * Radius2)){
                    mPositions.at(j*mHeightmapDimensions.x + i).y += 0.1f;
                    mDisplacement.at(j*mHeightmapDimensions.x + i) += factor < 0.004f? 0.f : factor - 0.004f;
                }

                else if(distance < (0.7 * Radius2) && distance > (0.5 * Radius2)){
                    mPositions.at(j*mHeightmapDimensions.x + i).y += 0.1f;
                    mDisplacement.at(j*mHeightmapDimensions.x + i) += factor < 0.002f? 0.f : factor - 0.002f;
                }
                else if(distance < (0.5 * Radius2)){
                     mPositions.at(j*mHeightmapDimensions.x + i).y += 0.1f;
                     mDisplacement.at(j*mHeightmapDimensions.x + i) += factor;
                }

            }
        }

    CalculateNormalsTangents(mHeightmapDimensions.x, mHeightmapDimensions.y);
    LoadSplatmap();
    MakeVertexArray();
}

glm::dvec3 MultiLayeredHeightmap::getIntersectionPoint() const
{
    return intersectionPoint;
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
#undef LOC
    std::cout << counter << std::endl;
}

void MultiLayeredHeightmap::HydraulicErodeTerrain()
{
#define LOC(a, b) b * mHeightmapDimensions.y + a

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
        std::cout << "Currently at position " << currPos.x << " " << currPos.y << std::endl;
        auto neigh = GetNeighborhood(currPos);
        auto next = GetLowestNeigh(neigh);

        float heightDifference = GetDisplacementAt(currPos) - GetDisplacementAt(next);

        /// @todo Add handling when the differece is negligible
        /// Move in random direction
        if(heightDifference <= 0.001)
        {
            int random = std::experimental::randint(0, 3);
            currPos = neigh.at(random);
            continue;
        }

        // If this location is the deepest in the neigh try to deposit everything
        if(heightDifference < 0)
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

            mNormals1.at(index) = glm::normalize(vTriangleNorm0);
            mNormals2.at(index) = glm::normalize(vTriangleNorm1);

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

            mTangents1.at(index) = tangent1;
            mTangents2.at(index) = tangent2;

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
                tempNormals += mNormals1.at(( (i-1) * dimX ) + j - 1);
                tempNormals += mNormals2.at(( (i-1) * dimX ) + j - 1);

                tempTangents += mTangents1.at(( (i-1) * dimX ) + j - 1);
                tempTangents += mTangents2.at(( (i-1) * dimX ) + j - 1);
            }

            // Look for upper-right triangles
            if(i != 0 && j != dimX-1){
                tempNormals += mNormals1.at(( (i-1) * dimX ) + j);
                tempNormals += mNormals2.at(( (i-1) * dimX ) + j);

                tempTangents += mTangents1.at(( (i-1) * dimX ) + j);
                tempTangents += mTangents2.at(( (i-1) * dimX ) + j);
            }

            // Look for bottom-right triangles
            if(i != dimY-1 && j != dimX-1){
                tempNormals += mNormals1.at(( i * dimX ) + j);
                tempNormals += mNormals2.at(( i * dimX ) + j);

                tempTangents += mTangents1.at(( i * dimX ) + j);
                tempTangents += mTangents2.at(( i * dimX ) + j);
            }

            // Look for bottom-left triangles
            if(i != dimY-1 && j != 0){
                tempNormals += mNormals1.at(( i * dimX ) + j - 1);
                tempNormals += mNormals2.at(( i * dimX ) + j - 1);

                tempTangents += mTangents1.at(( i * dimX ) + j - 1);
                tempTangents += mTangents2.at(( i * dimX ) + j - 1);
            }

            tempNormals = glm::normalize(tempNormals);
            mNormalsFinal.at(( i * dimX ) + j) = tempNormals; // Store final normal of j-th vertex in i-th row


            mTangentsFinal.at(( i * dimX ) + j) = tempTangents;

            //in radians
            mSlopeY.at(( i * dimX ) + j) = glm::acos(tempNormals.y);

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
    return mDisplacement.at(LOC(pos.y, pos.x));
}

void MultiLayeredHeightmap::AddDisplacementAt(glm::uvec2 pos, float addition)
{
    mDisplacement.at(LOC(pos.y, pos.x)) += addition;
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
//    DropletErodeTerrain(glm::uvec2(50, 50), 50);
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
    fScale = mSlopeY.at(i);
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

        mSplatmap.at(i) = {r,g,b};
    }
}

bool MultiLayeredHeightmap::intersectTriangle(const Face& _face, const glm::vec3& _normal, const Ray& _ray)
{

    glm::vec3 bary;
    auto temp_t = _t;
    auto temp_intersection = intersectionPoint;

    float dotRN = glm::dot(_ray.direction, _normal);

    float planeDist = glm::dot((_face.p0 - _ray.origin), _normal);

    _t = planeDist / dotRN;


    intersectionPoint = _ray.origin + _t * _ray.direction;

    bary_coord(intersectionPoint, _face.p0, _face.p1, _face.p2, bary);


    if(bary[0] >= 0 && bary[1] >= 0 && bary[2] >= 0 && bary[0] <= 1 && bary[1] <= 1 && bary[2] <= 1 && _t > epsilon){

        return true;
    }

    _t = temp_t;
    intersectionPoint = temp_intersection;

    return false;
}

bool MultiLayeredHeightmap::bary_coord(const glm::vec3& _p, const glm::vec3& _u, const glm::vec3& _v, const glm::vec3& _w, glm::vec3& _result) const
{
    glm::vec3 vu = _v - _u;
    glm::vec3 wu = _w - _u;
    glm::vec3 pu = _p - _u;

    double nx = vu[1] * wu[2] - vu[2] * wu[1];
    double ny = vu[2] * wu[0] - vu[0] * wu[2];
    double nz = vu[0] * wu[1] - vu[1] * wu[0];
    double ax = fabs(nx);
    double ay = fabs(ny);
    double az = fabs(nz);

    unsigned char max_coord;

    if (ax > ay)
    {
        if (ax > az)
        {
            max_coord = 0;
        }
        else
        {
            max_coord = 2;
        }
    }
    else
    {
        if (ay > az)
        {
            max_coord = 1;
        }
        else
        {
            max_coord = 2;
        }
    }

    switch (max_coord)
    {
    case 0:
        if (1.0 + ax == 1.0)
            return false;
        _result[1] = 1.0 + (pu[1] * wu[2] - pu[2] * wu[1]) / nx - 1.0;
        _result[2] = 1.0 + (vu[1] * pu[2] - vu[2] * pu[1]) / nx - 1.0;
        _result[0] = 1.0 - _result[1] - _result[2];
        break;

    case 1:
        if (1.0 + ay == 1.0)
            return false;
        _result[1] = 1.0 + (pu[2] * wu[0] - pu[0] * wu[2]) / ny - 1.0;
        _result[2] = 1.0 + (vu[2] * pu[0] - vu[0] * pu[2]) / ny - 1.0;
        _result[0] = 1.0 - _result[1] - _result[2];
        break;

    case 2:
        if (1.0 + az == 1.0)
            return false;
        _result[1] = 1.0 + (pu[0] * wu[1] - pu[1] * wu[0]) / nz - 1.0;
        _result[2] = 1.0 + (vu[0] * pu[1] - vu[1] * pu[0]) / nz - 1.0;
        _result[0] = 1.0 - _result[1] - _result[2];
        break;
    }

    return true;
}

void MultiLayeredHeightmap::intersect(const Ray& _ray )
{

    int dimX = mHeightmapDimensions.x, dimY = mHeightmapDimensions.y;
    Face Triangle1, Triangle2;
    glm::vec3 Normal1, Normal2;
    float temp_t = 1000000.f;

    for (int j = 0; j < dimY-1; j++ )
    {
        for (int i = 0; i < dimX-1; i++ )
        {
           // unsigned int index = ( j * dimX ) + i;

            Triangle1.p0 = mPositions.at((j * dimX ) + i);
            Triangle1.p1 = mPositions.at((j+1) * dimX  + i);
            Triangle1.p2 = mPositions.at((j+1) * dimX + i+1);

//            Triangle1.p0.y = mDisplacement.at((j * dimX ) + i);
//            Triangle1.p1.y = mDisplacement.at((j+1) * dimX  + i);
//            Triangle1.p2.y = mDisplacement.at((j+1) * dimX + i+1);

            Triangle2.p0 = mPositions.at((j+1) * dimX + i+1);
            Triangle2.p1 = mPositions.at((j* dimX) + i+1);
            Triangle2.p2 = mPositions.at((j * dimX ) + i);

//            Triangle2.p0.y = mDisplacement.at((j+1) * dimX + i+1);
//            Triangle2.p1.y = mDisplacement.at((j* dimX) + i+1);
//            Triangle2.p2.y = mDisplacement.at((j * dimX ) + i);

            Normal1 = glm::normalize(glm::cross(Triangle1.p0-Triangle1.p1, Triangle1.p1-Triangle1.p2));
            Normal2 = glm::normalize(glm::cross(Triangle2.p0-Triangle2.p1, Triangle2.p1-Triangle2.p2));

            if(intersectTriangle(Triangle1, Normal1, _ray) && _t < temp_t){
                temp_t = _t;
                Triangle1.normal = Normal1;  
                mIntersectionTriangle = Triangle1;
                mIntersectionHeight = j;
                mIntersectionWidth = i;

                intersection = true;
            }

            else if(intersectTriangle(Triangle2, Normal2, _ray) && _t < temp_t){
                temp_t = _t;
                Triangle2.normal = Normal2;
                mIntersectionTriangle = Triangle2;
                mIntersectionHeight = j;
                mIntersectionWidth = i;
                intersection = true;
            }

        }

    }

    if(intersection)
        intersectionPoint = _ray.origin + temp_t * _ray.direction;

}



void MultiLayeredHeightmap::GenerateArc(float r)
{
    mIntersectionRadius = r;

    std::vector<glm::vec3> circlePoints;
    for(auto i=0.0f; i < 2.0f * M_PI; i+= 0.1f)
    {
        circlePoints.push_back(glm::vec3(sin(i) * r, 0.0f, cos(i) * r));
    }

    auto ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aPosition");
    ab->bind().setData(circlePoints);
    ab->setObjectLabel(ab->getAttributes()[0].name + " of " + "Circle");
    mCircleVao = glow::VertexArray::create(ab, GL_LINES);

}



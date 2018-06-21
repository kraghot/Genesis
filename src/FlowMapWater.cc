#include "FlowMapWater.hh"
#include <MultiLayeredHeightmap.hh>
#include <math.h>

FlowMapWater::FlowMapWater(unsigned width, unsigned height, MultiLayeredHeightmap* heightmap):
    mWidth(width * 1.5),
    mHeight(height * 1.5),
    mFlowData(width * height, glm::vec2(0, 0)),
    mHeightmap(heightmap)
{
    mOffsetToTerrain.x = (mWidth - width) / 2.0;
    mOffsetToTerrain.y = (mHeight - height) / 2.0;
}

void FlowMapWater::SetWindDirection(glm::vec2 windDirection)
{
    mWindDirection = windDirection;
    glm::vec2 rescaledWindDirection = mWindDirection / 2.0F + 0.5f;
    mFlowData = std::vector<glm::vec2>(mWidth * mHeight, rescaledWindDirection);

    int frontArrayBegin, frontArrayEnd, particleBegin, particleEnd, variableCoordinate;

    if(mWindDirection == glm::vec2(0, -1))
    {
        frontArrayBegin = mOffsetToTerrain.x;
        frontArrayEnd = mWidth - mOffsetToTerrain.x;
        particleBegin = mHeight - 1 - mOffsetToTerrain.y;
        particleEnd = mOffsetToTerrain.y;
        variableCoordinate = 0;
    }
    else if(mWindDirection == glm::vec2(0, 1))
    {
        frontArrayBegin = mOffsetToTerrain.x;
        frontArrayEnd = mWidth - mOffsetToTerrain.x;
        particleBegin = mOffsetToTerrain.y;
        particleEnd = mHeight - mOffsetToTerrain.y;
        variableCoordinate = 0;
    }
    else if(mWindDirection == glm::vec2(-1, 0))
    {
        frontArrayBegin = mOffsetToTerrain.y;
        frontArrayEnd = mHeight - mOffsetToTerrain.y;
        particleBegin = mWidth - 1 - mOffsetToTerrain.x;
        particleEnd = mOffsetToTerrain.x;
        variableCoordinate = 1;
    }
    else
    {
        frontArrayBegin = mOffsetToTerrain.y;
        frontArrayEnd = mHeight - mOffsetToTerrain.y;
        particleBegin = mOffsetToTerrain.x;
        particleEnd = mWidth - mOffsetToTerrain.x;
        variableCoordinate = 1;
    }

    for(int fi = frontArrayBegin; fi < frontArrayEnd; fi++)
    {
        glm::vec2 particleStart;
        variableCoordinate ? particleStart = {particleBegin, fi} : particleStart = {fi, particleBegin};
        FlowParticle(particleStart, windDirection);
    }

    GenerateFlowTexture();
}

void FlowMapWater::GenerateFlowTexture()
{
    mFlowTexture = glow::Texture2D::create(mWidth, mHeight, GL_RG);
    mFlowTexture->bind().setData(GL_RG, mWidth, mHeight, mFlowData);
    mFlowTexture->bind().generateMipmaps();
}

void FlowMapWater::FlowParticle(glm::vec2 &particle, const glm::vec2 &direction)
{

//    if(direction.x == 0)
    glm::vec2 lastPos = particle - direction;
    glm::vec2 nextPos = particle;
    while(IsInBounds(nextPos))
    {
        glm::vec2 flowToAdd = nextPos - lastPos;
        flowToAdd = flowToAdd / 2.0F + 0.5f;
        glm::uvec2 arrayCoords = {round(nextPos.x), round(nextPos.y)};
        size_t arrayIndex = arrayCoords.y * mWidth + arrayCoords.x;

        mFlowData[arrayIndex] =/* 0.9f * mFlowData[arrayIndex]  + 0.1f **/ flowToAdd;

        lastPos = nextPos;
        nextPos = GetNextPosition(nextPos, direction);
    }
}

int FlowMapWater::incDec(int value, bool increment)
{
    if(increment)
        return value++;
    else
        return value--;
}

bool FlowMapWater::IsInBounds(glm::vec2 &particle)
{
    if(particle.x < mOffsetToTerrain.x || particle.x >= mWidth - mOffsetToTerrain.x)
        return false;
    if(particle.y < mOffsetToTerrain.y || particle.y >= mHeight - mOffsetToTerrain.y)
        return false;
    return true;
}

glm::vec2 FlowMapWater::GetNextPosition(glm::vec2 currPos, glm::vec2 direction)
{
    glm::vec2 forwardPos = currPos + direction;
    glm::uvec2 arrayCoords(forwardPos);
    arrayCoords -= mOffsetToTerrain;

    if(!IsInBounds(forwardPos))
        return forwardPos;

    if(mHeightmap->GetDisplacementAt(arrayCoords) > mWaterLevel)
    {
        glm::vec2 leftDirection = rotate(direction, 1.5708f);
        glm::vec2 rightDirection = rotate(direction, 4.71239f);

        leftDirection += currPos;
        rightDirection += currPos;

        glm::uvec2 unsignedLeftDirection(leftDirection);
        glm::uvec2 unsignedRightDirection(rightDirection);
        glm::vec2 runDirection;

        if(!IsInBounds(leftDirection) || !IsInBounds(rightDirection))
            return forwardPos;

        if(mHeightmap->GetDisplacementAt(unsignedLeftDirection - mOffsetToTerrain) <
                mHeightmap->GetDisplacementAt(unsignedRightDirection - mOffsetToTerrain))
            runDirection = leftDirection;
        else
            runDirection = rightDirection;
        return runDirection + direction;
    }

    return forwardPos;
}

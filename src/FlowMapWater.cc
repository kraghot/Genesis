#include "FlowMapWater.hh"
#include <MultiLayeredHeightmap.hh>
#include <LinearBlur.hh>
#include <math.h>

FlowMapWater::FlowMapWater(unsigned width, unsigned height, MultiLayeredHeightmap* heightmap):
    mWidth(width * 1.5),
    mHeight(height * 1.5),
    mFlowData(mWidth * mHeight, glm::vec2(0.5, 0.5)),
    mHeightmap(heightmap)
{
    mOffsetToTerrain.x = (mWidth - width) / 2.0;
    mOffsetToTerrain.y = (mHeight - height) / 2.0;
}

void FlowMapWater::SetWindDirection(glm::vec2 windDirection)
{
    mWindDirection = windDirection;
    glm::vec2 rescaledWindDirection = mWindDirection / 2.0F + 0.5f;
    InitializeOcean(rescaledWindDirection);

    // Bypass because no effect
    GenerateFlowTexture();
    return;

#ifdef GENESIS_BUILD_ALTERNATIVE_FLOWMAP

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

#endif
}

void FlowMapWater::SpawnRiver(glm::vec3 worldCoordinates, float flowVolume)
{
    glm::uvec2 currentCoords = mHeightmap->WorldToLocalCoordinates({worldCoordinates.x, worldCoordinates.z});

    // Add strong droplets to generate a better rainmap
    mHeightmap->DropletErodeTerrain(currentCoords, 10.0f);
}

void FlowMapWater::SetFlowAt(glm::uvec2 heightmapCoordinates, glm::vec2 flow)
{
    glm::uvec2 flowCoords = {heightmapCoordinates.x + mOffsetToTerrain.x,
                            heightmapCoordinates.y + mOffsetToTerrain.y};

    size_t arrayIndex = flowCoords.x + flowCoords.y * mWidth;
    auto existingFlow = mFlowData[arrayIndex];

    mFlowData[arrayIndex] = 0.8 * existingFlow + 0.2 * flow;
}

void FlowMapWater::GenerateFlowTexture()
{
    LinearBlur(mFlowData, mWidth, 1);

    mFlowTexture = glow::Texture2D::create(mWidth, mHeight, GL_RG);
    mFlowTexture->bind().setData(GL_RG, mWidth, mHeight, mFlowData);
    mFlowTexture->bind().generateMipmaps();
}

void FlowMapWater::ResetMap()
{
    mFlowData = std::vector<glm::vec2>(mWidth * mHeight, glm::vec2(0.5, 0.5));
}

void FlowMapWater::FlowParticle(glm::vec2 &particle, const glm::vec2 &direction)
{
    glm::vec2 lastPos = particle - direction;
    glm::vec2 nextPos = particle;
    while(IsInBounds(nextPos))
    {
        glm::vec2 flowToAdd = nextPos - lastPos;
        glm::uvec2 arrayCoords = {round(nextPos.x), round(nextPos.y)};

        if(flowToAdd != direction)
            ApplyFlow(arrayCoords, flowToAdd / 2.0F + 0.5f, 10);

        lastPos = nextPos;
        nextPos = GetNextPosition(nextPos, direction);
    }
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

void FlowMapWater::ApplyFlow(glm::uvec2 &coords, glm::vec2 flow, float radius)
{
    glm::vec2 floatCoords = glm::vec2(coords);
    for(int y = coords.y - radius; y < coords.y + radius; y++)
    {
        for(int x = coords.x - radius; x < coords.x + radius; x++)
        {
            float length = distance(glm::vec2(x, y), floatCoords);
            length = fabs(length);

            if(length > radius)
                continue;

            // Closer to center should be stronger
//            float lerpFactor = (radius - length) / radius;
            float lerpFactor = 1.0f;

            // Get Field to write to
            size_t arrayIndex = coords.y * mWidth + coords.x;

            auto dataToAdd = (1 - lerpFactor) * mFlowData[arrayIndex] + lerpFactor * flow;
            mFlowData[arrayIndex] = 0.5f * mFlowData[arrayIndex] + 0.5f * dataToAdd;
        }
    }

}

float FlowMapWater::GetTriangleArea(glm::vec3 A, glm::vec3 B, glm::vec3 C)
{
    glm::vec3 sideA, sideB, sideC;
    float lenA, lenB, lenC;

    // Get Sides from points
    sideA = B - C;
    sideB = A - C;
    sideC = A - B;

    lenA = sideA.length();
    lenB = sideB.length();
    lenC = sideC.length();

    float s = (lenA + lenB + lenC) / 2.0;
    float area = sqrt(s * (s - lenA) * (s - lenB) * (s - lenC));
    return area;
}

void FlowMapWater::InitializeOcean(glm::vec2 value)
{
    for(auto i = 0u; i < mWidth; i++)
    {
        for(auto j = 0u; j < mHeight; j++)
        {
            glm::vec2 flowPos = {i, j};
            if(IsInBounds(flowPos))
            {
                glm::vec2 heightmapPos = ConvertToHeightmapCoords(flowPos);
                if(mHeightmap->GetDisplacementAt(heightmapPos) < mWaterLevel)
                    mFlowData[i + j * mWidth] = value;
            }
            else
                mFlowData[i + j * mWidth] = value;
        }
    }
}

glm::vec2 FlowMapWater::ConvertToHeightmapCoords(glm::vec2 flowCoords)
{
    assert(IsInBounds(flowCoords));

    return {flowCoords.x - mOffsetToTerrain.x, flowCoords.y - mOffsetToTerrain.y};
}

glm::vec2 FlowMapWater::ClampTo1(glm::vec2 &vector)
{
    if(vector.length() > 1)
        return normalize(vector);
    return vector;
}

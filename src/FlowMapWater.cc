#include "FlowMapWater.hh"


FlowMapWater::FlowMapWater(unsigned width, unsigned height, MultiLayeredHeightmap* heightmap):
    mWidth(width),
    mHeight(height),
    mFlowData(width * height, glm::vec2(0, 0)),
    mHeightmap(heightmap)
{

}

void FlowMapWater::SetWindDirection(glm::vec2 windDirection)
{
    mWindDirection = windDirection;
    glm::vec2 rescaledWindDirection = mWindDirection / 2.0F + 0.5f;
    mFlowData = std::vector<glm::vec2>(mWidth * mHeight, rescaledWindDirection);

//    int beginX, beginY, endX, endY, directionX, directionY;

//    for(int y = beginY; y < endY; y = incDec(y, directionX))
//    {
//        for(int x = beginX; x < endX; x = incDec(x, directionY))
//        {

//        }
//    }

    GenerateFlowTexture();
}

void FlowMapWater::GenerateFlowTexture()
{
    mFlowTexture = glow::Texture2D::create(mWidth, mHeight, GL_RG);
    mFlowTexture->bind().setData(GL_RG, mWidth, mHeight, mFlowData);
    mFlowTexture->bind().generateMipmaps();
}

int FlowMapWater::incDec(int value, bool increment)
{
    if(increment)
        return value++;
    else
        return value--;
}

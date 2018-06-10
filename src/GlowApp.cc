#include "GlowApp.hh"
#include "PerlinNoiseGenerator.hh"
#include "IslandMaskGenerator.hh"

#include <AntTweakBar.h>

#include <fstream>

#include <glm/ext.hpp>

#include <glow/common/scoped_gl.hh>
#include <glow/common/str_utils.hh>
#include <glow/gl.hh>
#include <glow/objects/Framebuffer.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/TextureRectangle.hh>
#include <glow/objects/VertexArray.hh>

#include <glow-extras/assimp/Importer.hh>
#include <glow-extras/camera/GenericCamera.hh>
#include <glow-extras/geometry/Quad.hh>
#include <glow/objects/ElementArrayBuffer.hh>

unsigned int seed;
bool button;

using namespace glow;
const int heightMapDim = 256;

GlowApp::GlowApp():
    mHeightmap(20.0f, 3.0f)
{

}

void GlowApp::init()
{
    GlfwApp::init(); // call to base!

    // check correct working dir
    if (!std::ifstream("mesh/cube.obj").good())
    {
        glow::error() << "Working directory must be set to `bin/`!";
        exit(0);
    }

    // configure GlfwApp
    setTitle("Genesis");

    // set up tweakbar
    TwAddVarRW(tweakbar(), "bg color", TW_TYPE_COLOR3F, &mClearColor, "group=rendering");
    TwAddVarRW(tweakbar(), "light direction", TW_TYPE_DIR3F, &mLightDir, "group=scene");
    TwAddVarRW(tweakbar(), "light distance", TW_TYPE_FLOAT, &mLightDis, "group=scene step=0.1 min=1 max=100");
    TwAddVarRW(tweakbar(), "rotation speed", TW_TYPE_FLOAT, &mSpeed, "group=scene step=0.1");
    TwAddVarCB(tweakbar(), "seed", TW_TYPE_UINT16, GlowApp::setSeedTerrain, GlowApp::getSeedTerrain, &seed, "group=scene step=1");
    TwAddButton(tweakbar(), "terrain", GlowApp::randomTerrain, this, " label='Generate random terrain '");
    TwAddVarRW(tweakbar(), "Number of Iterations", TW_TYPE_UINT16, &mNumIterations, "group=scene step=1");
    TwAddButton(tweakbar(), "Erode Terrain", GlowApp::dropletErode, this, "label='Erode Terrain'");

    // load object
    //mMeshCube = assimp::Importer().load("mesh/cube.obj");
    mShaderObj = Program::createFromFile("shader/obj");
    mTextureColor = Texture2D::createFromFile("texture/rock-albedo.png", ColorSpace::sRGB);
    mTextureNormal = Texture2D::createFromFile("texture/rock-normal.png", ColorSpace::Linear);

    //define textures for terrain
    std::vector<std::string> mTerrainTextures = {"texture/snow009.jpg", "texture/grass007.jpg", "texture/rock007.jpg"};

    //define normals of textures for terrain (in the same order as the textures)
    std::vector<std::string> mTerrainNormals = {"texture/snow009_normal.png", "texture/grass007_normal9.png", "texture/rock007_normal9.png"};

    //load textures for terrain
    mTexture = mHeightmap.LoadTexture(mTerrainTextures);

    //load normals of textures for terrain
    mTexNormal = mHeightmap.LoadNormal(mTerrainNormals);

    //generate first random seed for terrain
    std::srand(std::time(0));
    seed = std::rand();
    GlowApp::initTerrain();
    std::cout << "seed: " << seed << std::endl;

    // set up framebuffer and output
    mShaderOutput = Program::createFromFile("shader/output");
    mMeshQuad = geometry::Quad<>().generate();
    mTargetColor = TextureRectangle::create(1, 1, GL_RGB16F);
    mTargetDepth = TextureRectangle::create(1, 1, GL_DEPTH_COMPONENT32);
    mFramebuffer = Framebuffer::create("fColor", mTargetColor, mTargetDepth);

    //setup background
    auto pbt = util::pathOf(__FILE__) + "/../bin/cubemap/mountain/";
    mBackgroundTexture = glow::TextureCubeMap::createFromData(
                glow::TextureData::createFromFileCube(pbt + "posx.jpg",
                                                      pbt + "negx.jpg",
                                                      pbt + "posy.jpg",
                                                      pbt + "negy.jpg",
                                                      pbt + "posz.jpg",
                                                      pbt + "negz.jpg",
                                                      glow::ColorSpace::sRGB));
    mShaderBg = glow::Program::createFromFile("shader/bg");
}

void GlowApp::onResize(int w, int h)
{
    GlfwApp::onResize(w, h); // call to base!

    mTargetColor->bind().resize(w, h);
    mTargetDepth->bind().resize(w, h);
}

void GlowApp::update(float elapsedSeconds)
{
    GlfwApp::update(elapsedSeconds); // call to base!

    // rotate obj
    mAngle += mSpeed * elapsedSeconds;
    mAngle = glm::mod(mAngle, (2 * glm::pi<float>()));
}

void GlowApp::render(float elapsedSeconds)
{
    if(button){
        GlowApp::initTerrain();
        button = false;
    }

    GlfwApp::render(elapsedSeconds); // call to base!

    auto cam = getCamera(); // internal camera from GlfwApp with some default input handling
    auto view = cam->getViewMatrix();
    auto proj = cam->getProjectionMatrix();
    auto camPos = cam->getPosition();

    auto lightPos = normalize(mLightDir) * mLightDis;

    // render to framebuffer
    {
        //auto fb = mFramebuffer->bind();
//        GLOW_SCOPED(enable, GL_CULL_FACE);  // use backface culling
        //GLOW_SCOPED(enable, GL_DEPTH_TEST); // use z-Buffer
        GLOW_SCOPED(enable, GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(65535);

        // clear buffer (color + depth)
        {
            auto c = pow(mClearColor, glm::vec3(2.224f)); // inverse gamma correction
            GLOW_SCOPED(clearColor, c.r, c.g, c.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        {
            GLOW_SCOPED(disable, GL_CULL_FACE); // no backface culling
            glDepthMask(GL_FALSE);

            auto shader = mShaderBg->use();

            shader.setTexture("uTexture", mBackgroundTexture);
            auto invProj = inverse(cam->getProjectionMatrix());
            auto invView = inverse(cam->getViewMatrix());
            shader.setUniform("uInvProj", invProj);
            shader.setUniform("uInvView", invView);

            // draw fullscreen quad
            mMeshQuad->bind().draw();

            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
        }



        // draw object
        {
            auto model = glm::rotate(mAngle, glm::vec3(0, 1, 0)) * glm::translate(glm::mat4(1.f), glm::vec3(0, -50, 0));

            auto shader = mShaderObj->use();
            shader.setUniform("uView", view);
            shader.setUniform("uProj", proj);
            shader.setUniform("uModel", model);

            shader.setUniform("uLightPos", lightPos);
            shader.setUniform("uCamPos", camPos);

            shader.setTexture("uTexColor", mTextureColor);
            shader.setTexture("uTexNormal", mTextureNormal);

            shader.setTexture("uSplatmapTex", mHeightmap.getSplatmapTexture());

            //terrain 2d texture array
            shader.setTexture("uTerrainTex", mTexture);
            shader.setTexture("uTerrainNormal", mTexNormal);
            shader.setTexture("uTexDisplacement", mHeightmap.GetDisplacementTexture());

            shader.setUniform("fRenderHeight", mHeightmap.getMfHeightScale());

            mPerlinTest->bind().draw();
        }
    }

    // render to screen
    // (applies gamma correction and dithering)

}

void GlowApp::initTerrain(){

    PerlinNoiseGenerator perlinNoise(seed);
    DiamondSquareNoiseGenerator diamondNoise(heightMapDim, heightMapDim, 128);
    IslandMaskGenerator islandFilter(glm::vec2(heightMapDim - 50, heightMapDim - 50), glm::vec2(heightMapDim, heightMapDim), seed);

    std::vector<MultiLayeredHeightmap::GeneratorProperties> properties;

    properties.emplace_back(islandFilter,
                            1,
                            1.0f,
                            0.0f, // unused
                            100.0f,
                            0.0f // unused
                            );


//    properties.emplace_back(diamondNoise,
//                            1,
//                            1.0f,
//                            0.0f, // unused
//                            100.0f,
//                            0.0f // unused
//                            );

//    properties.emplace_back(perlinNoise,
//                            4,
//                            1.0f,
//                            0.5f,
//                            50.0f,
//                            0.5f
//                            );

    mPerlinTest = mHeightmap.GenerateTerrain(properties, heightMapDim, heightMapDim);

    //define textures for terrain
    std::vector<std::string> mTerrainTextures = {"texture/snow009.jpg", "texture/grass007.jpg", "texture/rock007.jpg"};

    //define normals of textures for terrain (in the same order as the textures)
    std::vector<std::string> mTerrainNormals = {"texture/snow009_normal.png", "texture/grass007_normal9.png", "texture/rock007_normal9.png"};

    //load textures for terrain
    mTexture = mHeightmap.LoadTexture(mTerrainTextures);

    //load normals of textures for terrain
    mTexNormal = mHeightmap.LoadNormal(mTerrainNormals);
}

void GlowApp::setSeed(unsigned int var){

    if(seed != var){
      seed = var;
      button = true;
  }

  else
      button = false;
}

unsigned int GlowApp::getSeed() const{
    return seed;
}

void GlowApp::dropletErodeIterations()
{
    mHeightmap.IterateDroplet(mNumIterations);
}

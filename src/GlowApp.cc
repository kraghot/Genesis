#include "GlowApp.hh"
#include "PerlinNoiseGenerator.hh"

#include <AntTweakBar.h>

#include <fstream>

#include <glm/ext.hpp>

#include<glow-extras/glfw/GlfwApp.hh>
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
const int heightMapDim = 150;

void TW_CALL GlowApp::randomTerrain(void *clientData){
    static_cast<GlowApp *>(clientData)->setSeed(std::rand());
}

void TW_CALL GlowApp::setSeedTerrain(const void *value, void *clientData){
    static_cast<GlowApp *>(clientData)->setSeed(*static_cast<const unsigned int *>(value));
}

void TW_CALL GlowApp::getSeedTerrain(void *value, void *clientData){

     *static_cast<unsigned int *>(value) = static_cast<const GlowApp *>(clientData)->getSeed();
}


GlowApp::GlowApp():
    mHeightmap(20.0f,3.0f)
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
    TwAddButton(tweakbar(), "terrain", GlowApp::randomTerrain, NULL, " label='Generate random terrain '");





    PerlinNoiseGenerator noise(2924319);
    mPerlinTest = mHeightmap.GenerateTerrain(&noise, heightMapDim, heightMapDim);


    // load object
    mMeshCube = assimp::Importer().load("mesh/cube.obj");
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
    mBackgroundTexture = glow::TextureCubeMap::createFromData(glow::TextureData::createFromFileCube(pbt + "posx.jpg",
                                                                                                    pbt + "negx.jpg",
                                                                                                    pbt + "posy.jpg",
                                                                                                    pbt + "negy.jpg",
                                                                                                    pbt + "posz.jpg",
                                                                                                    pbt + "negz.jpg",
                                                                                                    glow::ColorSpace::sRGB));
    mShaderBg = glow::Program::createFromFile("shader/bg");
    mShaderLine = glow::Program::createFromFile("shader/line");

    std::vector<glm::vec3> mPositions = {{0, 50, 0}, {0, 0, 0}};
    auto ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aPosition");
    ab->bind().setData(mPositions);
    ab->setObjectLabel(ab->getAttributes()[0].name + " of " + "Line");

    mLineVao = glow::VertexArray::create(ab, GL_LINES);

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
//            auto invView = inverse(cam->getViewMatrix());
            auto invView = cam->getInverseViewMatrix();

            //world space mouse position

//            if(GlfwApp::isMouseButtonPressed(mLeftClick)){
//                std::cout << "mouse x: " << mMousePosFinal.x << " mouse pos y: " << mMousePosFinal.y << " mouse pos z: " << mMousePosFinal.z << std::endl;
//            }



            //std::cout << "mouse x: " << testRay.direction.x << "mouse x: " << testRay.direction.y << std::endl;

            shader.setUniform("uInvProj", invProj);
            shader.setUniform("uInvView", invView);

            // draw fullscreen quad
            mMeshQuad->bind().draw();

            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);

            mMousePosWin = GlfwApp::getMousePosition();
            mMouseNDC = glm::vec4((mMousePosWin.x/(getWindowWidth()/2.f) - 1.f), ((getWindowHeight()-mMousePosWin.y)/(getWindowHeight()/2.f) - 1.f), -1.0f, 1.f);
            mMousePosWorld =invProj *  mMouseNDC;
            mMousePosWorld /= mMousePosWorld.w;
            mMousePosWorld = invView * mMousePosWorld;
            mMousePosFinal = glm::vec3(mMousePosWorld);
//            mMousePosFinal = invView * glm::vec4(mMousePosWorld.x, mMousePosWorld.y, -1.f, 0.f);
//            mMousePosFinal = glm::vec3(mMousePosWorld.x/mMousePosWorld.w, mMousePosWorld.y/mMousePosWorld.w, mMousePosWorld.z/mMousePosWorld.w);
        }

//        std::cout << mMousePosFinal.x << " " << mMousePosFinal.y << " " << mMousePosFinal.z << std::endl;

        // draw object
        {
            auto lineShader = mShaderLine->use();
            lineShader.setUniform("uView", view);
            lineShader.setUniform("uProj", proj);

            Ray testRay;
            testRay.origin = camPos;

//            mMousePosFinal = glm::normalize(mMousePosFinal);
            testRay.direction = glm::normalize(mMousePosFinal - camPos);

            if(isKeyPressed(71)) // GLFW_KEY_G
            {
                std::vector<glm::vec3> mPositions = {testRay.origin, testRay.origin + (testRay.direction * 100)};
                auto ab = glow::ArrayBuffer::create();
                ab->defineAttribute<glm::vec3>("aPosition");
                ab->bind().setData(mPositions);
                ab->setObjectLabel(ab->getAttributes()[0].name + " of " + "Line");

                mLineVao = glow::VertexArray::create(ab, GL_LINES);
            }

//            mHeightmap.intersect(testRay);

            mLineVao->bind().draw();

            glm::vec3 camPos1 = camPos;
            glm::vec3 testRaydir =  testRay.direction;

            auto model = glm::translate(glm::mat4(1.f), glm::vec3(0, -50, 0));
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
            //mMeshCube->bind().draw();
        }
    }

    // render to screen
    // (applies gamma correction and dithering)

}

void GlowApp::initTerrain(){

    PerlinNoiseGenerator noise(seed);
    mPerlinTest = mHeightmap.GenerateTerrain(&noise, heightMapDim, heightMapDim);

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




#include "GlowApp.hh"
#include "PerlinNoiseGenerator.hh"
#include "IslandMaskGenerator.hh"

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
bool buttonTerrain;
bool recalculateSplatmap;
bool randomWind;

using namespace glow;
const int heightMapDim = 512;

void TW_CALL GlowApp::TweakSetSplatmap(void *clientData){
    static_cast<GlowApp *>(clientData)->SetSplatmap();
}

void TW_CALL GlowApp::TweakRandomWind(void *clientData){
    static_cast<GlowApp *>(clientData)->SetRandomWind();
}

GlowApp::GlowApp():
     mHeightmap(20.0f,1.0f),
     mBrush(&mHeightmap),
     mBiomes(&mHeightmap),
     mFlowMap(heightMapDim, heightMapDim, &mHeightmap)
{

}

void GlowApp::addMesh(const std::string &name)
{
   // auto path = util::pathOf(__FILE__) + "/" + name + ".obj";
    auto mesh = assimp::Importer().load("mesh/" + name + "/" + name + ".obj");
//    mesh->setPrimitiveMode(GL_PATCHES);
//    mesh->setVerticesPerPatch(3);
    mMeshesBiome1.push_back(mesh);
}


void GlowApp::init()
{

    GlfwApp::init(); // call to base!

    // check correct working dir
//    if (!std::ifstream("mesh/cube.obj").good())
//    {
//        glow::error() << "Working directory must be set to `bin/`!";
//        exit(0);
//    }

    // configure GlfwApp
    setTitle("Genesis");


    TwEnumVal TextureChoices[] = { {TEXTURE_SNOW, "Snow"},{TEXTURE_GRASS, "Grass"}, {TEXTURE_ROCK, "Rock"} };
    TwType TextureTwType = TwDefineEnum("TextureType", TextureChoices, 3);
    TwAddVarRW(tweakbar(), "Texture Brush", TextureTwType, &m_selectedTexture, NULL);

    TwEnumVal BrushChoices[] = { {BRUSH_TEXTURE, "Texture Brush"}, {BRUSH_HEIGHT, "Height Brush"}};
    TwType BrushTwType = TwDefineEnum("BrushType", BrushChoices, 2);
    TwAddVarRW(tweakbar(), "Brush Type", BrushTwType, &m_selectedBrush, NULL);

    TwEnumVal MapChoices[] = { {MAP_SPLAT, "Splatmap"}, {MAP_RAIN, "Rain map"}, {MAP_DROPLET, "Droplet Erode debug"}, {MAP_BIOMES, "Biomes Map"} };
    TwType MapTwType = TwDefineEnum("MapType", MapChoices, 4);
    TwAddVarRW(tweakbar(), "Map Type", MapTwType, &m_selectedMap, NULL);

    TwEnumVal WindChoices[] = { {NS, "North -> South"}, {SN, "South -> North"}, {WE, "West -> East"}, {EW, "East -> West"}};
    TwType WindTwType = TwDefineEnum("WindType", WindChoices, 4);
    TwAddVarRW(tweakbar(), "Wind direction", WindTwType, &m_selectedWind, NULL);


    // set up tweakbar
    //TwAddVarRW(tweakbar(), "bg color", TW_TYPE_COLOR3F, &mClearColor, "group=rendering");
    //TwAddVarRW(tweakbar(), "light direction", TW_TYPE_DIR3F, &mLightDir, "group=scene");
    TwAddVarRW(tweakbar(), "light distance", TW_TYPE_FLOAT, &mLightDis, "group=scene step=0.1 min=1 max=1000");
    //TwAddVarRW(tweakbar(), "rotation speed", TW_TYPE_FLOAT, &mSpeed, "group=scene step=0.1");
    TwAddVarCB(tweakbar(), "seed", TW_TYPE_UINT16, GlowApp::setSeedTerrain, GlowApp::getSeedTerrain, &seed, "group=scene step=1");
    TwAddButton(tweakbar(), "terrain", GlowApp::randomTerrain, NULL, " label='Generate random terrain '");
    TwAddVarRW(tweakbar(), "Number of Iterations", TW_TYPE_UINT16, &mNumIterations, "group=scene step=1");
    TwAddButton(tweakbar(), "Erode Terrain", GlowApp::dropletErode, this, "label='Erode Terrain' key=e");
    TwAddVarRW(tweakbar(), "Height Brush", TW_TYPE_FLOAT, &mHeightBrushFactor, "group=scene step=0.5");
    TwAddVarRW(tweakbar(), "Circle radius", TW_TYPE_FLOAT, &mCircleRadius, "group=scene step=0.5");
    TwAddVarRW(tweakbar(), "DebugFlow", TW_TYPE_BOOLCPP, &mDebugFlow, "group=scene key=l label='DebugFlow'");
    TwAddButton(tweakbar(), "splatmap", GlowApp::TweakSetSplatmap, NULL, " label='Recalculate textures '");
    TwAddButton(tweakbar(), "wind", GlowApp::TweakRandomWind, NULL, " label='Random wind direction '");

    // load object
    mMeshCube = assimp::Importer().load("mesh/cube.obj");
    mShaderObj = Program::createFromFile("shader/obj");
    mTextureColor = Texture2D::createFromFile("texture/rock-albedo.png", ColorSpace::sRGB);
    mTextureNormal = Texture2D::createFromFile("texture/rock-normal.png", ColorSpace::Linear);



    //addMesh("palm0");

    //generate first random seed for terrain
    std::srand(std::time(0));
    seed = std::rand();
    GlowApp::initTerrain();
    std::cout << "seed: " << seed << std::endl;



    //set up mesh shaders
    meshShader = Program::createFromFile("shader/mesh");
    meshTextureColor = Texture2D::createFromFile("texture/rock-albedo.png", ColorSpace::sRGB);
    meshTextureNormal = Texture2D::createFromFile("texture/rock-normal.png", ColorSpace::Linear);


    // set up framebuffer and output
    mShaderOutput = Program::createFromFile("shader/output");
    mMeshQuad = geometry::Quad<>().generate();
    mTargetColor = TextureRectangle::create(1, 1, GL_RGB16F);
    mTargetDepth = TextureRectangle::create(1, 1, GL_DEPTH_COMPONENT32);
    mFramebuffer = Framebuffer::create("fColor", mTargetColor, mTargetDepth);

    //setup background
    auto pbt = util::pathOf(__FILE__) + "/../bin/cubemap/sea/";
    mBackgroundTexture = glow::TextureCubeMap::createFromData(
                glow::TextureData::createFromFileCube(pbt + "posx.jpg",
                                                      pbt + "negx.jpg",
                                                      pbt + "posy.jpg",
                                                      pbt + "negy.jpg",
                                                      pbt + "posz.jpg",
                                                      pbt + "negz.jpg",
                                                      glow::ColorSpace::sRGB));


    mShaderBg = glow::Program::createFromFile("shader/bg");
    mShaderLine = glow::Program::createFromFile("shader/line");
    mShaderWater = glow::Program::createFromFile("shader/water");
    mShaderRiver = glow::Program::createFromFile("shader/river");
    mBrush.GenerateArc(mCircleRadius);

    mWaterNormal1 = glow::Texture2D::createFromFile("texture/waternormal1.jpg", ColorSpace::Linear);
    mWaterNormal2 = glow::Texture2D::createFromFile("texture/waternormal2.jpg", ColorSpace::Linear);

    std::vector<glm::vec3> linePositions = {{0, 50, 0}, {0, 0, 0}};
    auto ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aPosition");
    ab->bind().setData(linePositions);
    ab->setObjectLabel(ab->getAttributes()[0].name + " of " + "Line");

    mLineVao = glow::VertexArray::create(ab, GL_LINES);

    mBiomes.randomWindDirection();
    mFlowMap.SetWindDirection(mBiomes.GetWindDirection());
    mHeightmap.mFlowMap = &mFlowMap;

    mWaterTimeLoop[0] = 0.0f;
    mWaterTimeLoop[1] = 2.0f;

    //=====Poisson Disk Sampling for mesh positions======

    rainforest.resize(4);

    addMesh("jungle_tree1");
    addMesh("jungle_bush1");
    addMesh("jungle_bush2");

    getMeshPositions(rainforest);
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
    if(buttonTerrain){
        GlowApp::initTerrain();
        mBiomes.randomWindDirection();
        mFlowMap.SetWindDirection(mBiomes.GetWindDirection());
        buttonTerrain = false;

        rainy_positions1.clear();
        rainy_positions2.clear();
        rainy_positions3.clear();

        getMeshPositions(rainforest);

    }

    GlfwApp::render(elapsedSeconds); // call to base!

    auto cam = getCamera();
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
            //auto c = pow(mClearColor, glm::vec3(2.224f)); // inverse gamma correction
            //GLOW_SCOPED(clearColor, c.r, c.g, c.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        {
            GLOW_SCOPED(disable, GL_CULL_FACE); // no backface culling
            glDepthMask(GL_FALSE);

            auto shader = mShaderBg->use();

            shader.setTexture("uTexture", mBackgroundTexture);
            auto invProj = inverse(cam->getProjectionMatrix());
            auto invView = cam->getInverseViewMatrix();

            shader.setUniform("uInvProj", invProj);
            shader.setUniform("uInvView", invView);

            // draw fullscreen quad
            mMeshQuad->bind().draw();

            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);

            //world space mouse position
            mMousePosWin = GlfwApp::getMousePosition();
            mMouseNDC = glm::vec4((mMousePosWin.x/(getWindowWidth()/2.f) - 1.f), ((getWindowHeight()-mMousePosWin.y)/(getWindowHeight()/2.f) - 1.f), -1.0f, 1.f);
            mMousePosWorld =invProj *  mMouseNDC;
            mMousePosWorld /= mMousePosWorld.w;
            mMousePosWorld = invView * mMousePosWorld;
            mMousePosFinal = glm::vec3(mMousePosWorld);

        }

        // draw object
        {
            auto lineShader = mShaderLine->use();
            lineShader.setUniform("uView", view);
            lineShader.setUniform("uProj", proj);
            lineShader.setUniform("uModel", glm::mat4());

            Ray testRay;
            testRay.origin = camPos;
            testRay.direction = glm::normalize(mMousePosFinal - camPos);


            if(isKeyPressed(71)) // GLFW_KEY_G
            {
                mBrush.intersect_quadtree(testRay, RayIntersectionQuadtree_nodes);
//                mBrush.intersect(testRay);
                std::vector<glm::vec3> linePositions = {glm::vec3(0, 0, 0), glm::vec3(0, 100, 0)};
                auto ab = glow::ArrayBuffer::create();
                ab->defineAttribute<glm::vec3>("aPosition");
                ab->bind().setData(linePositions);
                ab->setObjectLabel(ab->getAttributes()[0].name + " of " + "Line");

                auto intersectionPoint = mBrush.getIntersectionPoint();
                auto localInterection = mHeightmap.WorldToLocalCoordinates({intersectionPoint.x, intersectionPoint.z});
                printf("World: %f %f %f\nLocal: %u %u\n", intersectionPoint.x, intersectionPoint.y, intersectionPoint.z, localInterection.x, localInterection.y);
                std::cout << std::flush;

                mLineVao = glow::VertexArray::create(ab, GL_LINES);
            }

            if(isKeyPressed(66)) //GLFW_KEY_B
            {
                mBiomes.generateRainMap(m_selectedWind);
                mFlowMap.SetWindDirection(mBiomes.GetWindDirection());
            }
            mLineVao->bind().draw();

            lineShader.setUniform("uModel", mBrush.GetCircleRotation(mBrush.mIntersectionTriangle.normal, mBrush.mIntersection));
            mBrush.getCircleVao()->bind().draw();

            if(recalculateSplatmap){
                mHeightmap.LoadSplatmap();
                recalculateSplatmap = false;
            }

            if(randomWind){
                mBiomes.randomWindDirection();
                mFlowMap.SetWindDirection(mBiomes.GetWindDirection());
                randomWind = false;
            }

            if(GlfwApp::isMouseButtonPressed(mRightClick))
                m_selectedBrush == 0? mBrush.SetTextureBrush(m_selectedTexture, mBiomes.mBiomeMap, mBiomes.getBiomesTexture()) : mBrush.SetHeightBrush(mHeightBrushFactor);

            std::vector<glow::SharedTexture2D> selectedMap = {mHeightmap.getSplatmapTexture(), mBiomes.getRainTexture(), mHeightmap.getSplatmapTexture(), mBiomes.getBiomesTexture()};


            auto model = glm::mat4(1.f); // glm::translate(glm::mat4(1.f), glm::vec3(0, -50, 0));
            auto shader = mShaderObj->use();
            shader.setUniform("uView", view);
            shader.setUniform("uProj", proj);
            shader.setUniform("uModel", model);

            shader.setUniform("uLightPos", lightPos);
            shader.setUniform("uCamPos", camPos);

            shader.setTexture("uTexColor", mTextureColor);
            shader.setTexture("uTexNormal", mTextureNormal);

            shader.setTexture("uSplatmapTex", selectedMap[m_selectedMap]);
            shader.setUniform("uDrawDebugRain", (m_selectedMap == MAP_DROPLET));

            //terrain 2d texture array
            shader.setTexture("uTerrainTex", mTexture);
            shader.setTexture("uTerrainNormal", mTexNormal);
            shader.setTexture("uTexDisplacement", mHeightmap.GetDisplacementTexture());

            shader.setUniform("fRenderHeight", mHeightmap.getMfHeightScale());

            mHeightmap.getVao()->bind().draw();
        }

        //========== mesh rendering ==========

        renderMesh(rainy_positions1, view, proj, 0);

        if(rainy_positions2 != rainy_positions1)
            renderMesh(rainy_positions2, view, proj, 1);

        if(rainy_positions3 != rainy_positions2)
            renderMesh(rainy_positions3, view, proj, 2);

        //===================================

            GLOW_SCOPED(enable, GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            auto shaderWater = mShaderWater->use();
            shaderWater.setUniform("uView", view);
            shaderWater.setUniform("uProj", proj);
            shaderWater.setUniform("uLightPos", lightPos);
            shaderWater.setUniform("uCameraPos", camPos);
            shaderWater.setUniform("uHeightmapDim", heightMapDim);
            shaderWater.setTexture("uCubeMap", mBackgroundTexture);
            shaderWater.setTexture("uNormalMap1", mWaterNormal1);
            shaderWater.setTexture("uNormalMap2", mWaterNormal2);
            shaderWater.setTexture("uFlowMap", mFlowMap.GetFlowTexture());
            shaderWater.setUniform("uDrawFlowMap", mDebugFlow);

            const float periodLength = 4.0f;
            const float  halfPeriod = periodLength / 2.0f;

            for(int i = 0; i < 2; i++)
            {
                mWaterTimeLoop[i] += elapsedSeconds;
                // Full period length is 2 seconds
                if(mWaterTimeLoop[i] >= periodLength)
                    mWaterTimeLoop[i] -= periodLength;

                // One normal map rises and the other falls the first half and then reverse
            }

            float elapsedPeriod = mWaterTimeLoop[0];
            bool isSecondPhase = false;
            if(elapsedPeriod >= halfPeriod)
            {
                elapsedPeriod -= halfPeriod;
                isSecondPhase = true;
            }

            float waterLerpFactor = elapsedPeriod / halfPeriod;
            if(isSecondPhase)
                waterLerpFactor = 1.0f - waterLerpFactor;

            shaderWater.setUniform("uElapsedTime1", mWaterTimeLoop[0]);
            shaderWater.setUniform("uElapsedTime2", mWaterTimeLoop[1]);
            shaderWater.setUniform("uLerpFactor", waterLerpFactor);

            mMeshQuad->bind().draw();

            auto shaderRiver = mShaderRiver->use();
            shaderRiver.setUniform("uView", view);
            shaderRiver.setUniform("uProj", proj);
            shaderRiver.setUniform("uLightPos", lightPos);
            shaderRiver.setUniform("uCameraPos", camPos);
            shaderRiver.setTexture("uCubeMap", mBackgroundTexture);
            shaderRiver.setTexture("uNormalMap1", mWaterNormal1);
            shaderRiver.setTexture("uNormalMap2", mWaterNormal2);
            shaderRiver.setTexture("uFlowMap", mFlowMap.GetFlowTexture());
            shaderRiver.setUniform("uDrawFlowMap", mDebugFlow);
            shaderRiver.setUniform("uElapsedTime1", mWaterTimeLoop[0]);
            shaderRiver.setUniform("uElapsedTime2", mWaterTimeLoop[1]);
            shaderRiver.setUniform("uLerpFactor", waterLerpFactor);
            shaderRiver.setTexture("uTexDisplacement", mHeightmap.GetDisplacementTexture());
            shaderRiver.setTexture("uTexRainFlow", mHeightmap.mRainFlowMapTexture);


            mHeightmap.getVao()->bind().draw();

    }

    // render to screen
    // (applies gamma correction and dithering)

}

void GlowApp::initTerrain(){
    QuadTree quadtree(&mHeightmap);

    PerlinNoiseGenerator perlinNoise(seed);
    DiamondSquareNoiseGenerator diamondNoise(heightMapDim, heightMapDim, 64);
    IslandMaskGenerator islandFilter(glm::vec2(heightMapDim - 100, heightMapDim - 100), glm::vec2(heightMapDim, heightMapDim), seed);

    std::vector<MultiLayeredHeightmap::GeneratorProperties> properties;

    properties.emplace_back(diamondNoise,
                            1,
                            1.0f,
                            0.0f, // unused
                            50.0f,
                            0.0f // unused
                            );

//    properties.emplace_back(perlinNoise,
//                            3,
//                            1.0f,
//                            0.5f,
//                            30.0f,
//                            0.5f
//                            );

    std::vector<FilterGenerator*> filters;
    filters.push_back(&islandFilter);

    mPerlinTest = mHeightmap.GenerateTerrain(properties, filters, heightMapDim, heightMapDim);

    //define textures for terrain
    std::vector<std::string> mTerrainTextures = {"texture/01grass.jpg", "texture/04grass.jpg", "texture/rock007.jpg", "texture/beach.jpg"};

    //define normals of textures for terrain (in the same order as the textures)
    std::vector<std::string> mTerrainNormals = {"texture/01grass.png", "texture/04grass.png", "texture/rock007_normal9.png", "texture/beach_normal.png"};

    //load textures for terrain
    mTexture = mHeightmap.LoadTexture(mTerrainTextures);

    //load normals of textures for terrain
    mTexNormal = mHeightmap.LoadNormal(mTerrainNormals);

    RayIntersectionQuadtree_nodes = quadtree.construct_quadtree();
}

void GlowApp::setSeed(unsigned int var){

    if(seed != var){
      seed = var;
      buttonTerrain = true;
  }

  else
      buttonTerrain = false;
}

unsigned int GlowApp::getSeed() const{
    return seed;
}

void GlowApp::dropletErodeIterations()
{
    mHeightmap.IterateDroplet(mNumIterations);
}

void GlowApp::SetSplatmap(){
    recalculateSplatmap = true;
}

void GlowApp::SetRandomWind(){
    randomWind = true;
}

void GlowApp::renderMesh(std::vector<glm::vec3> mesh_positions, glm::mat4 view, glm::mat4 proj, unsigned int vegType){

    auto mesh_shader = meshShader->use();
    mesh_shader.setUniform("uView", view);
    mesh_shader.setUniform("uProj", proj);

    auto temp = mMeshesBiome1[vegType]->bind(); //MUST STAY
    auto ab = glow::ArrayBuffer::create();
    auto mesh_vao1 = mMeshesBiome1[vegType]->getCurrentVAO();

    std::vector<glm::mat4> scalingMatrices;

    scalingMatrices = {glm::scale(glm::vec3(1.f, 1.f, 1.f)), glm::scale(glm::vec3(0.01f, 0.01f, 0.01f)), glm::scale(glm::vec3(0.01f, 0.01f, 0.01f))};

    std::vector<glm::vec4> m1;
    std::vector<glm::vec4> m2;
    std::vector<glm::vec4> m3;
    std::vector<glm::vec4> m4;

    {
        int a = 0;
        std::vector<glow::SharedArrayBuffer> mAbs;
        while(a < mesh_positions.size()){

            auto localCoords = mHeightmap.WorldToLocalCoordinates({mesh_positions.at(a).x, mesh_positions.at(a).z});
            glm::mat4 mesh_model(1.0f);
            auto rotMat = mBrush.GetCircleRotation(mHeightmap.mNormalsFinal.at(localCoords.y * mHeightmap.mHeightmapDimensions.x + localCoords.x), {0, 0, 0});
            //auto scalingMat = glm::scale(glm::vec3(0.1f, 0.1f, 0.1f));
            auto translMat = glm::translate(mesh_positions.at(a));

            mesh_model = translMat * scalingMatrices[vegType] * rotMat * mesh_model;

            //glm::vec3(0, mHeightmap.mPositions.at(mHeightmap.WorldToLocalCoordinates(glm::vec3(0.f,0.f,0.f)).x * mHeightmap.mHeightmapDimensions.x + mHeightmap.WorldToLocalCoordinates(glm::vec3(0.f,0.f,0.f)).y).y, 0));
            m1.resize(mesh_positions.size());
            m2.resize(mesh_positions.size());
            m3.resize(mesh_positions.size());
            m4.resize(mesh_positions.size());


            m1.at(a) = {mesh_model[0][0], mesh_model[0][1], mesh_model[0][2], mesh_model[0][3]};
            m2.at(a) = {mesh_model[1][0], mesh_model[1][1], mesh_model[1][2], mesh_model[1][3]};
            m3.at(a) = {mesh_model[2][0], mesh_model[2][1], mesh_model[2][2], mesh_model[2][3]};
            m4.at(a) = {mesh_model[3][0], mesh_model[3][1], mesh_model[3][2], mesh_model[3][3]};


            a++;
        }

        ab = glow::ArrayBuffer::create();
        ab->defineAttribute<glm::vec4>("uM1");
        ab->bind().setData(m1);
        ab->setDivisor(1);
        mAbs.push_back(ab);


        ab = glow::ArrayBuffer::create();
        ab->defineAttribute<glm::vec4>("uM2");
        ab->bind().setData(m2);
        ab->setDivisor(1);
        mAbs.push_back(ab);


        ab = glow::ArrayBuffer::create();
        ab->defineAttribute<glm::vec4>("uM3");
        ab->bind().setData(m3);
        ab->setDivisor(1);
        mAbs.push_back(ab);


        ab = glow::ArrayBuffer::create();
        ab->defineAttribute<glm::vec4>("uM4");
        ab->bind().setData(m4);
        ab->setDivisor(1);
        mAbs.push_back(ab);


        mesh_vao1->attach(mAbs);
        mesh_vao1->draw(mesh_positions.size());

        }

}

 void GlowApp::getMeshPositions(std::vector<std::vector<glm::vec2>> plist){
     int a = 0;

     plist.at(0) = mBiomes.poissonDiskSampling(6, 40, mBiomes.rain_start, mBiomes.rain_end, plist.at(0));
     plist.at(1) = mBiomes.poissonDiskSampling(10, 80, mBiomes.rain_start, mBiomes.rain_end, plist.at(0));
     plist.at(2) = mBiomes.poissonDiskSampling(10, 80, mBiomes.rain_start, mBiomes.rain_end, plist.at(1));


            while(a < plist.at(0).size()){
                auto worldCoordinates = mHeightmap.LocalToWorldCoordinates(plist.at(0)[a]);
                worldCoordinates.y = mHeightmap.GetDisplacementAt(plist.at(0)[a]); //mHeightmap.mPositions.at(plist[a].x * mHeightmap.mHeightmapDimensions.x + plist[a].y).y;
                rainy_positions1.push_back(worldCoordinates);
                a++;
            }

            a = 0;

            if(plist.at(1) == plist.at(0))
                rainy_positions2 = rainy_positions1;

            else{
            while(a < plist.at(1).size()){
                auto worldCoordinates = mHeightmap.LocalToWorldCoordinates(plist.at(1)[a]);
                worldCoordinates.y = mHeightmap.GetDisplacementAt(plist.at(1)[a]); //mHeightmap.mPositions.at(plist[a].x * mHeightmap.mHeightmapDimensions.x + plist[a].y).y;
                rainy_positions2.push_back(worldCoordinates);
                a++;
                }
            }

            a = 0;
            if(plist.at(2) == plist.at(1))
                rainy_positions3 = rainy_positions2;


            else{
            while(a < plist.at(2).size()){
                auto worldCoordinates = mHeightmap.LocalToWorldCoordinates(plist.at(2)[a]);
                worldCoordinates.y = mHeightmap.GetDisplacementAt(plist.at(2)[a]); //mHeightmap.mPositions.at(plist[a].x * mHeightmap.mHeightmapDimensions.x + plist[a].y).y;
                rainy_positions3.push_back(worldCoordinates);
                a++;
                }
            }
}

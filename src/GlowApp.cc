#include "GlowApp.hh"
#include "PerlinNoiseGenerator.hh"
#include "IslandMaskGenerator.hh"
#include "CircularIslandMaskFilter.hh"

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
bool recalculateSplatmap = true;
bool randomWind;
bool windDirection;

using namespace glow;
const int heightMapDim = 512;

GlowApp::GlowApp():
     mHeightmap(20.0f,1.0f),
     mBrush(&mHeightmap),
     mBiomes(&mHeightmap),
     mFlowMap(heightMapDim, heightMapDim, &mHeightmap)
{

}




void GlowApp::addMesh(const std::string &name, const std::string &tex_path, const std::string &norm_path)
{
    auto mesh = assimp::Importer().load("mesh/" + name + "/" + name + ".obj");
    mMeshesArray.push_back(mesh);

    auto tex = Texture2D::createFromFile(tex_path, ColorSpace::sRGB);
    mesh_textures.push_back(tex);

    auto norm = Texture2D::createFromFile(norm_path, ColorSpace::Linear);
    mesh_normals.push_back(norm);
}


void GlowApp::init()
{

    GlfwApp::init(); // call to base!

    // configure GlfwApp
    setTitle("Genesis");

    TwAddButton(tweakbar(), "terrain", GlowApp::randomTerrain, NULL, " label='Generate random terrain '");
//    TwAddVarCB(tweakbar(), "Seed", TW_TYPE_UINT16, GlowApp::setSeedTerrain, GlowApp::getSeedTerrain, &seed, "step=1");
    TwAddVarRW(tweakbar(), "Edit mode", TW_TYPE_BOOLCPP, &mEditMode, "key=l label='Edit Mode'");

    TwEnumVal TextureChoices[] = { {TEXTURE_JUNGLE, "Jungle ground"},{TEXTURE_FOREST, "Forest ground"}, {TEXTURE_ROCK, "Rock"}, {TEXTURE_BEACH, "Beach sand"} };
    TwType TextureTwType = TwDefineEnum("TextureType", TextureChoices, 4);
    TwAddVarRW(tweakbar(), "Texture Brush", TextureTwType, &m_selectedTexture, "group=Editing");

    TwEnumVal BrushChoices[] = { {BRUSH_TEXTURE, "Texture Brush"}, {BRUSH_HEIGHT, "Height Brush"}};
    TwType BrushTwType = TwDefineEnum("BrushType", BrushChoices, 2);
    TwAddVarRW(tweakbar(), "Brush Type", BrushTwType, &m_selectedBrush, "group=Editing");
    TwAddVarRW(tweakbar(), "Height Brush", TW_TYPE_FLOAT, &mHeightBrushFactor, "group=Editing step=0.5");
    TwAddButton(tweakbar(), "splatmap", GlowApp::TweakSetSplatmap, NULL, "group=Editing label='Recalculate textures '");

    TwEnumVal WindChoices[] = { {NS, "North -> South"}, {SN, "South -> North"}, {WE, "West -> East"}, {EW, "East -> West"}};
    TwType WindTwType = TwDefineEnum("WindType", WindChoices, 4);
    TwAddVarRW(tweakbar(), "Wind direction", WindTwType, &m_selectedWind, "group=Biomes");
    TwAddButton(tweakbar(), "wind direction", GlowApp::TweakSetWindDirection, NULL, "group=Biomes label='Set wind direction '");
    TwAddButton(tweakbar(), "wind", GlowApp::TweakRandomWind, NULL, "group=Biomes label='Random wind direction '");

    // set up tweakbar
    TwAddButton(tweakbar(), "Droplet Erode Terrain", GlowApp::dropletErode, this, "group=Erosion label='Droplet Erode Terrain' key=e");
    TwAddVarRW(tweakbar(), "Number of Iterations", TW_TYPE_UINT16, &mNumIterations, "group=Erosion step=1");
    TwAddButton(tweakbar(), "Thermal Erode Terrain", GlowApp::thermalErode, this, "group=Erosion label='Thermal Erode Terrain' key=t");

    // load object
    mShaderObj = Program::createFromFile("shader/obj");

    mBrush.GenerateArc(mCircleRadius);

    randomScalingMatrices.resize(6);

    //generate first random seed for terrain
    std::srand(std::time(0));
    seed = std::rand();
    GlowApp::InitTerrain();
    std::cout << "seed: " << seed << std::endl;

    //set up mesh shaders
    meshShader = Program::createFromFile("shader/mesh");

    // set up framebuffer and output
    mShaderOutput = Program::createFromFile("shader/output");
    mMeshQuad = geometry::Quad<>().generate();
    mTargetColor = TextureRectangle::create(1, 1, GL_RGB16F);
    mTargetDepth = TextureRectangle::create(1, 1, GL_DEPTH_COMPONENT32);
    mFramebuffer = Framebuffer::create("fColor", mTargetColor, mTargetDepth);

    //setup background
    std::string pbt = "cubemap/sea/";
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
    mShaderInfiniteWater = glow::Program::createFromFile("shader/infinitewater");


    mWaterNormal1 = glow::Texture2D::createFromFile("texture/waternormal1.jpg", ColorSpace::Linear);
    mWaterNormal2 = glow::Texture2D::createFromFile("texture/waternormal2.jpg", ColorSpace::Linear);

    std::vector<glm::vec3> linePositions = {{0, 50, 0}, {0, 0, 0}};
    auto ab = glow::ArrayBuffer::create();
    ab->defineAttribute<glm::vec3>("aPosition");
    ab->bind().setData(linePositions);
    ab->setObjectLabel(ab->getAttributes()[0].name + " of " + "Line");

    mBiomes.randomWindDirection();
    mFlowMap.SetWindDirection(mBiomes.GetWindDirection());
    mHeightmap.mFlowMap = &mFlowMap;

    mWaterTimeLoop[0] = 0.0f;
    mWaterTimeLoop[1] = 2.0f;

    //rainforest

    addMesh("jungle_tree1", "mesh/jungle_tree1/jungle_tree1.png", "mesh/jungle_tree1/jungle_tree1_normal.jpg");
    addMesh("jungle_bush", "mesh/jungle_bush/diffuse.tga", "mesh/jungle_bush/normal.tga");
    addMesh("lavender", "mesh/lavender/lavender.png", "mesh/lavender/lavender_normal.png");

    //forest
    addMesh("pinetree", "mesh/pinetree/pinetree.png", "mesh/pinetree/pinetree_normal.png");
    addMesh("forest_bush2", "mesh/forest_bush2/diffuse.tga", "mesh/forest_bush2/normal.tga");
    addMesh("mushroom", "mesh/mushroom/mushroom.png", "mesh/mushroom/mushroom_normal.png");


    rainforest = getMeshPositions(true);
    forest = getMeshPositions(false);
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
        GlowApp::InitTerrain();
        mBiomes.randomWindDirection();
        mFlowMap.SetWindDirection(mBiomes.GetWindDirection());
        buttonTerrain = false;

        rainforest.clear();
        forest.clear();

        rainforest = getMeshPositions(true);
        forest = getMeshPositions(false);

    }

    GlfwApp::render(elapsedSeconds); // call to base!

    auto cam = getCamera();
    auto view = cam->getViewMatrix();
    auto proj = cam->getProjectionMatrix();
    auto camPos = cam->getPosition();

    auto lightPos = normalize(mLightDir) * mLightDis;

    {
        GLOW_SCOPED(enable, GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(65535);

        // clear buffer (color + depth)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


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
        }

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        glClear(GL_DEPTH_BUFFER_BIT);

        // draw object
        {
            mBrush.GenerateArc(mCircleRadius);

            if(isKeyPressed(71)) // GLFW_KEY_G
            {
                auto intersectionPoint = mBrush.getIntersectionPoint();
                auto localInterection = mHeightmap.WorldToLocalCoordinates({intersectionPoint.x, intersectionPoint.z});
                printf("World: %f %f %f\nLocal: %u %u\n", intersectionPoint.x, intersectionPoint.y, intersectionPoint.z, localInterection.x, localInterection.y);
                std::cout << std::flush;
            }

            if(windDirection)
            {
                mBiomes.generateRainMap(m_selectedWind);
                mFlowMap.SetWindDirection(mBiomes.GetWindDirection());
                windDirection = false;

                rainforest.clear();
                forest.clear();

                rainforest = getMeshPositions(true); //true = it's the first biome
                forest = getMeshPositions(false); // false = it's not the first  biome

                firstRender = 0;
            }

            if(recalculateSplatmap){
                mHeightmap.LoadSplatmap();
                mBiomes.LoadBiomesMap();
                mHeightmap.ComputeAmbientOcclusionMap();
                recalculateSplatmap = false;
            }

            if(randomWind){
                mBiomes.randomWindDirection();
                mFlowMap.SetWindDirection(mBiomes.GetWindDirection());
                randomWind = false;

                rainforest.clear();
                forest.clear();

                rainforest = getMeshPositions(true); //true = it's the first biome
                forest = getMeshPositions(false); // false = it's not the first  biome

                firstRender = 0;
            }

            if(GlfwApp::isMouseButtonPressed(mRightClick))
                m_selectedBrush == 0? mBrush.SetTextureBrush(m_selectedTexture, mBiomes.mBiomeMap, mBiomes.getBiomesTexture()) : mBrush.SetHeightBrush(mHeightBrushFactor);

            GLOW_SCOPED(enable, GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            auto model = glm::mat4(1.f);
            auto shader = mShaderObj->use();
            shader.setUniform("uView", view);
            shader.setUniform("uProj", proj);
            shader.setUniform("uModel", model);

            shader.setUniform("uLightPos", lightPos);
            shader.setUniform("uCamPos", camPos);


            shader.setTexture("uSplatmapTex", mBiomes.getBiomesTexture());
            shader.setTexture("uIndexMap", mBiomes.GetIndicesTexture());
            shader.setUniform("uDrawDebugRain", (m_selectedMap == MAP_DROPLET));

            //terrain 2d texture array
            shader.setTexture("uTerrainTex", mTexture);
            shader.setTexture("uTerrainNormal", mTexNormal);
            shader.setTexture("uTexDisplacement", mHeightmap.GetDisplacementTexture());
            shader.setTexture("uAmbientOcclusionMap", mHeightmap.mAmbientOcclusionMap);

            shader.setUniform("fRenderHeight", mHeightmap.getMfHeightScale());

            mHeightmap.GetVao()->bind().draw();
        }

        // Intersect
        {
            mMousePosWin = GlfwApp::getMousePosition();
            glm::vec2 mouseNdc = {(mMousePosWin.x/(getWindowWidth()/2.f) - 1.f), ((getWindowHeight()-mMousePosWin.y)/(getWindowHeight()/2.f) - 1.f)};
            float depth = 1;
            auto invProj = glm::inverse(proj);
            auto invView = glm::inverse(view);

            glReadPixels(mMousePosWin.x, (getWindowHeight() - mMousePosWin.y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

            if(depth != 1.0)
                mBrush.IntersectUnproject(mouseNdc, invView, invProj, depth);

            auto lineShader = mShaderLine->use();
            lineShader.setUniform("uView", view);
            lineShader.setUniform("uProj", proj);
            lineShader.setUniform("uModel", mBrush.GetCircleRotation(mBrush.mIntersectionTriangle.normal, mBrush.mIntersection, {0, 1, 0}));
            mBrush.getCircleVao()->bind().draw();
        }

        {
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

            mHeightmap.GetVao()->bind().draw();

            auto shaderInfinite = mShaderInfiniteWater->use();
            shaderInfinite.setUniform("uView", view);
            shaderInfinite.setUniform("uProj", proj);
            shaderInfinite.setTexture("uNormalMap1", mWaterNormal1);
            shaderInfinite.setTexture("uCubeMap", mBackgroundTexture);


            mMeshQuad->bind().draw();
        }

        //========== mesh rendering ==========

        {
            if(!mEditMode){
                if(!rainforest.empty())
                    renderMesh(rainforest, view, proj, true);
                if(!forest.empty())
                    renderMesh(forest, view, proj, false);
            }
        }

        //===================================
    }
}

void GlowApp::InitTerrain(){
    QuadTree quadtree(&mHeightmap);

    PerlinNoiseGenerator perlinNoise(seed);
    DiamondSquareNoiseGenerator diamondNoise(heightMapDim, heightMapDim, 64);

    CircularIslandMaskFilter islandFilter(0.65f, 0.95f, perlinNoise);

    std::vector<MultiLayeredHeightmap::GeneratorProperties> properties;

    properties.emplace_back(diamondNoise,
                            1,
                            1.0f,
                            0.0f, // unused
                            50.0f,
                            0.0f // unused
                            );

    std::vector<FilterGenerator*> filters;
    filters.push_back(&islandFilter);

    mPerlinTest = mHeightmap.GenerateTerrain(properties, filters, heightMapDim, heightMapDim);

    //define textures for terrain
    std::vector<std::string> terrainTextures = {"texture/91.png", "texture/04grass.jpg", "texture/beach.jpg", "texture/rock007.jpg", "texture/snow009.jpg"};

    //define normals of textures for terrain (in the same order as the textures)
    std::vector<std::string> terrainNormals = {"texture/91_normal.png", "texture/04grass.png", "texture/beach_normal.png", "texture/rock007_normal9.png", "texture/snow009_normal.png"};

    //load textures for terrain
    mTexture = mHeightmap.LoadTexture(terrainTextures);

    //load normals of textures for terrain
    mTexNormal = mHeightmap.LoadNormal(terrainNormals);

    RayIntersectionQuadtree_nodes = quadtree.ConstructQuadtree();

    mFlowMap.SetWindDirection(mBiomes.GetWindDirection());

    firstRender = 0;

}

void GlowApp::SetSeed(unsigned int var){

    if(seed != var){
      seed = var;
      buttonTerrain = true;
  }

  else
      buttonTerrain = false;
}

unsigned int GlowApp::GetSeed() const{
    return seed;
}

void GlowApp::DropletErodeIterations()
{
    mHeightmap.IterateDroplet(mNumIterations);
}

void GlowApp::ThermalErosion()
{
    mHeightmap.ThermalErodeTerrain();
}

void GlowApp::SetSplatmap(){
    recalculateSplatmap = true;
}

void GlowApp::SetRandomWind(){
    randomWind = true;
}

void GlowApp::SetWindDirection(){
    windDirection = true;
}


void GlowApp::renderMesh(std::vector<std::vector<glm::vec3>> mesh_positions, glm::mat4 view, glm::mat4 proj, bool rainy){

    auto cam = getCamera();
    auto lightPos = normalize(mLightDir) * mLightDis;
    auto camPos = cam->getPosition();

    auto mesh_shader = meshShader->use();
    mesh_shader.setUniform("uView", view);
    mesh_shader.setUniform("uProj", proj);
    mesh_shader.setUniform("uLightPos", lightPos);
    mesh_shader.setUniform("uCamPos", camPos);


    unsigned int rainy_inc;

    if(rainy)
       rainy_inc = 0;
    else
       rainy_inc = 3;

    for(unsigned int i = 0; i < mesh_positions.size(); i++){

        mesh_shader.setTexture("uTexColor", mesh_textures[i + rainy_inc]);
        mesh_shader.setTexture("uTexNormal", mesh_normals[i + rainy_inc]);

        auto temp = mMeshesArray[i + rainy_inc]->bind(); //MUST STAY
        auto ab = glow::ArrayBuffer::create();
        auto mesh_vao1 = mMeshesArray[i + rainy_inc]->getCurrentVAO();


        // Default scaling factors
        std::vector<glm::vec3> tmp = {glm::vec3(0.7f, 0.7f, 0.7f), glm::vec3(0.07f, 0.07f, 0.07f), glm::vec3(0.003f, 0.003f, 0.003f), glm::vec3(0.01f, 0.01f, 0.01f), glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(2.f, 2.f, 2.f)};

        std::vector<std::vector<glm::vec4>> transformation_matrix;

        if(firstRender < 6)
            randomScalingMatrices.at(i+ rainy_inc).resize(mesh_positions.at(i).size());

        transformation_matrix.resize(4);
        transformation_matrix.at(0).resize(mesh_positions.at(i).size());
        transformation_matrix.at(1).resize(mesh_positions.at(i).size());
        transformation_matrix.at(2).resize(mesh_positions.at(i).size());
        transformation_matrix.at(3).resize(mesh_positions.at(i).size());


        size_t a = 0;
        std::vector<glow::SharedArrayBuffer> mAbs;

        while(a < mesh_positions.at(i).size()){

            auto localCoords = mHeightmap.WorldToLocalCoordinates({mesh_positions.at(i).at(a).x, mesh_positions.at(i).at(a).z});
            glm::mat4 mesh_model(1.0f);
            glm::mat4 rotMat;

            int randNumber = rand() % 5;

            float randomScalingFactor = 1.f - ((float)randNumber/10.f);

            if(firstRender < 6)
                randomScalingMatrices.at(i + rainy_inc).at(a) = glm::scale(glm::vec3(tmp.at(i + rainy_inc)[0] * randomScalingFactor, tmp.at(i + rainy_inc)[1] * randomScalingFactor, tmp.at(i + rainy_inc)[2] * randomScalingFactor));

            if(i == 0)
                rotMat = mBrush.GetCircleRotation(glm::normalize(glm::vec3(mHeightmap.mNormalsFinal.at(localCoords.y * mHeightmap.mHeightmapDimensions.x + localCoords.x).x, mHeightmap.mNormalsFinal.at(localCoords.y * mHeightmap.mHeightmapDimensions.x + localCoords.x).y + 10, mHeightmap.mNormalsFinal.at(localCoords.y * mHeightmap.mHeightmapDimensions.x + localCoords.x).z)), {0, 0, 0}, {0, 1, 0});
            else
                rotMat = mBrush.GetCircleRotation(mHeightmap.mNormalsFinal.at(localCoords.y * mHeightmap.mHeightmapDimensions.x + localCoords.x), {0, 0, 0}, {0, 1, 0});

            auto translMat = glm::translate(mesh_positions.at(i).at(a));

            mesh_model = translMat * randomScalingMatrices.at(i + rainy_inc).at(a) * rotMat * mesh_model;

            transformation_matrix.at(0).at(a) = {mesh_model[0][0], mesh_model[0][1], mesh_model[0][2], mesh_model[0][3]};
            transformation_matrix.at(1).at(a) = {mesh_model[1][0], mesh_model[1][1], mesh_model[1][2], mesh_model[1][3]};
            transformation_matrix.at(2).at(a) = {mesh_model[2][0], mesh_model[2][1], mesh_model[2][2], mesh_model[2][3]};
            transformation_matrix.at(3).at(a) = {mesh_model[3][0], mesh_model[3][1], mesh_model[3][2], mesh_model[3][3]};

            a++;

        }


        ab = glow::ArrayBuffer::create();
        ab->defineAttribute<glm::vec4>("uM1");
        ab->bind().setData(transformation_matrix.at(0));
        ab->setDivisor(1);
        mAbs.push_back(ab);


        ab = glow::ArrayBuffer::create();
        ab->defineAttribute<glm::vec4>("uM2");
        ab->bind().setData(transformation_matrix.at(1));
        ab->setDivisor(1);
        mAbs.push_back(ab);


        ab = glow::ArrayBuffer::create();
        ab->defineAttribute<glm::vec4>("uM3");
        ab->bind().setData(transformation_matrix.at(2));
        ab->setDivisor(1);
        mAbs.push_back(ab);


        ab = glow::ArrayBuffer::create();
        ab->defineAttribute<glm::vec4>("uM4");
        ab->bind().setData(transformation_matrix.at(3));
        ab->setDivisor(1);
        mAbs.push_back(ab);


        mesh_vao1->attach(mAbs);
        mesh_vao1->draw(mesh_positions.at(i).size());

        if(firstRender < 7)
            firstRender++;
        }


}

 std::vector<std::vector<glm::vec3>> GlowApp::getMeshPositions(bool rainy){
     size_t a = 0;
     std::vector<std::vector<glm::vec2>> plist;
     std::vector<std::vector<glm::vec3>> mesh_positions;

     plist.resize(3);
     mesh_positions.resize(3);

     if(rainy){
         plist.at(0) = mBiomes.poissonDiskSampling(7, 40, mBiomes.rain_start, mBiomes.rain_end, plist.at(0), true);
         plist.at(1) = mBiomes.poissonDiskSampling(7, 80, mBiomes.rain_start, mBiomes.rain_end, plist.at(0), true);
         plist.at(2) = mBiomes.poissonDiskSampling(12 , 80, mBiomes.rain_start, mBiomes.rain_end, plist.at(1), true);
     }
         else{
         plist.at(0) = mBiomes.poissonDiskSampling(4, 40, mBiomes.NoRain_start, mBiomes.NoRain_end, plist.at(0), false);
         plist.at(1) = mBiomes.poissonDiskSampling(15, 80, mBiomes.NoRain_start, mBiomes.NoRain_end, plist.at(0), false);
         plist.at(2) = mBiomes.poissonDiskSampling(7, 80, mBiomes.NoRain_start, mBiomes.NoRain_end, plist.at(1), false);
     }

            while(a < plist.at(0).size()){
                auto worldCoordinates = mHeightmap.LocalToWorldCoordinates(plist.at(0)[a]);
                worldCoordinates.y = mHeightmap.GetDisplacementAt(plist.at(0)[a]);
                mesh_positions.at(0).push_back(worldCoordinates);
                a++;
            }

            a = 0;


            while(a < plist.at(1).size()){
                auto worldCoordinates = mHeightmap.LocalToWorldCoordinates(plist.at(1)[a]);
                worldCoordinates.y = mHeightmap.GetDisplacementAt(plist.at(1)[a]);
                 mesh_positions.at(1).push_back(worldCoordinates);
                a++;
                }


            a = 0;

            while(a < plist.at(2).size()){
                auto worldCoordinates = mHeightmap.LocalToWorldCoordinates(plist.at(2)[a]);
                worldCoordinates.y = mHeightmap.GetDisplacementAt(plist.at(2)[a]);
                mesh_positions.at(2).push_back(worldCoordinates);
                a++;
                }


            return mesh_positions;
}

#include"MultiLayeredHeightmap.hh"
#include "DiamondSquareNoiseGenerator.hh"
#include "PerlinNoiseGenerator.hh"
#include "Brush.hh"
#include "Biomes.hh"
#include "FlowMapWater.hh"
#include "QuadTree.hh"

#include <AntTweakBar.h>

#include <glow-extras/glfw/GlfwApp.hh>
#include <glow/fwd.hh>
#include <glm/glm.hpp>

#include <glow/objects/Texture2DArray.hh>
#include<glow/objects/TextureCubeMap.hh>


#ifndef GLOWAPP
#define GLOWAPP

class GlowApp : public glow::glfw::GlfwApp
{
private:
    glm::vec3 mClearColor = {0 / 255.0f, 85 / 255.0f, 159 / 255.0f};
    glm::vec3 mLightDir = normalize(glm::vec3(.2, .7, .7));
    float mLightDis = 300.0f;

    float mAngle = 0.0f;
    float mSpeed = 0.0f;
    unsigned int mNumIterations = 1000;

    glm::vec2 mMousePosWin;
    const int mLeftClick = 0;
    const int mRightClick = 1;

    glm::vec3 mMousePosFinal;
    glm::vec4 mMousePosWorld;
    glm::vec4 mMouseNDC;

    float mHeightBrushFactor = 10.f;
    float mCircleRadius = 10.f;

private:
    glow::SharedProgram mShaderOutput;
    glow::SharedVertexArray mMeshQuad;
    glow::SharedFramebuffer mFramebuffer;
    glow::SharedTextureRectangle mTargetColor;
    glow::SharedTextureRectangle mTargetDepth;

    glow::SharedProgram mShaderObj;
    glow::SharedProgram mShaderLine;
    glow::SharedProgram mShaderWater;
    glow::SharedProgram mShaderRiver;
    glow::SharedProgram mShaderInfiniteWater;
    glow::SharedVertexArray mPerlinTest;
    glow::SharedTexture2D mTextureColor;
    glow::SharedTexture2D mTextureNormal;

    glow::SharedTexture2DArray mTexture;
    glow::SharedTexture2DArray mTexNormal;

    MultiLayeredHeightmap mHeightmap;
    glow::SharedTexture2D mSplatmapTexture;
    glow::SharedTexture2D mWaterNormal1;
    glow::SharedTexture2D mWaterNormal2;

    glow::SharedTextureCubeMap mBackgroundTexture;
    glow::SharedProgram mShaderBg;

    bool mDebugFlow = false;

    bool mEditMode = true;

    Brush mBrush;
    Biomes mBiomes;

    FlowMapWater mFlowMap;
    float mWaterTimeLoop[2];

    std::vector<quadtree_node> RayIntersectionQuadtree_nodes;


    typedef enum { TEXTURE_SNOW, TEXTURE_GRASS, TEXTURE_ROCK} SelectedTexture;
    SelectedTexture m_selectedTexture = TEXTURE_GRASS;

    typedef enum { BRUSH_TEXTURE, BRUSH_HEIGHT} SelectedBrush;
    SelectedBrush m_selectedBrush = BRUSH_TEXTURE;

    //typedef enum { MAP_SPLAT, MAP_RAIN, MAP_DROPLET, MAP_RAINFLOW} SelectedMap;
    typedef enum { MAP_SPLAT, MAP_RAIN, MAP_DROPLET, MAP_BIOMES, MAP_OCCLUSION} SelectedMap;
    SelectedMap m_selectedMap = MAP_BIOMES;

    typedef enum { NS, SN, WE, EW} SelectedWind;
    SelectedWind m_selectedWind = NS;

    void renderMesh(std::vector<std::vector<glm::vec3>> mesh_positions, glm::mat4 view, glm::mat4 proj, bool rainy);
    std::vector<std::vector<glm::vec3>> getMeshPositions(bool rainy);

    std::vector<std::vector<glm::vec3>> rainforest;
    std::vector<std::vector<glm::vec3>> forest;


public:
    GlowApp();
    // load resources, initialize app
    void init() override;
    // update game logic with fixed timestep
    void update(float elapsedSeconds) override;
    // render game with variable timestep
    void render(float elapsedSeconds) override;
    // called after window is resized
    void onResize(int w, int h) override;

    void addMesh(const std::string &name, const std::string &tex_path, const std::string &norm_path);
    //void getMeshPositions(std::vector<std::vector<glm::vec2>> plist);

    std::vector<glow::SharedVertexArray> mMeshesArray;

    int mMeshIdx = 0;
    glow::SharedProgram meshShader;
    glow::SharedTexture2D meshTextureColor_rainforest1;
    glow::SharedTexture2D meshTextureColor_rainforest2;
    glow::SharedTexture2D meshTextureColor_rainforest3;

    glow::SharedTexture2D meshNormalColor_rainforest1;
    glow::SharedTexture2D meshNormalColor_rainforest2;
    glow::SharedTexture2D meshNormalColor_rainforest3;

    glow::SharedTexture2D meshTextureColor_forest1;
    glow::SharedTexture2D meshTextureColor_forest2;
    glow::SharedTexture2D meshTextureColor_forest3;

    glow::SharedTexture2D meshNormalColor_forest1;
    glow::SharedTexture2D meshNormalColor_forest2;
    glow::SharedTexture2D meshNormalColor_forest3;

    std::vector<glow::SharedTexture2D> rainforest_textures;
    std::vector<glow::SharedTexture2D> forest_textures;

    std::vector<glow::SharedTexture2D> mesh_textures;
    std::vector<glow::SharedTexture2D> mesh_normals;



    void randomTerrain();
    void initTerrain();

    static void TW_CALL randomTerrain(void *clientData){
        static_cast<GlowApp *>(clientData)->setSeed(std::rand());
    }
    static void TW_CALL setSeedTerrain(const void *value, void *clientData){
        static_cast<GlowApp *>(clientData)->setSeed(*static_cast<const unsigned int *>(value));
    }
    static void TW_CALL getSeedTerrain(void *value, void *clientData){

        *static_cast<unsigned int *>(value) = static_cast<const GlowApp *>(clientData)->getSeed();
    }
    static void TW_CALL dropletErode(void* clientData){
        static_cast<GlowApp *>(clientData)->dropletErodeIterations();
    }

    void setSeed(unsigned int var);
    unsigned int getSeed() const;
    void dropletErodeIterations();
    void SetSplatmap();
    static void TW_CALL TweakSetSplatmap(void *clientData);

    static void TW_CALL TweakRandomWind(void *clientData);
    void SetRandomWind();

    std::vector<glm::vec3> rainy_positions1;
    std::vector<glm::vec3> rainy_positions2;
    std::vector<glm::vec3> rainy_positions3;
   // glm::mat4 mesh_model;

};

#endif

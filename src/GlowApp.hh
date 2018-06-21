#include"MultiLayeredHeightmap.hh"
#include "DiamondSquareNoiseGenerator.hh"
#include "PerlinNoiseGenerator.hh"
#include "Brush.hh"
#include "Biomes.hh"
#include "FlowMapWater.hh"

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
    unsigned int mNumIterations = 1;

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
    glow::SharedVertexArray mMeshCube;
    glow::SharedVertexArray mPerlinTest;
    glow::SharedVertexArray mLineVao;
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

    Brush mBrush;
    Biomes mBiomes;

    FlowMapWater mFlowMap;
    float mWaterTimeLoop[2];


    typedef enum { TEXTURE_SNOW, TEXTURE_GRASS, TEXTURE_ROCK} SelectedTexture;
    SelectedTexture m_selectedTexture = TEXTURE_GRASS;

    typedef enum { BRUSH_TEXTURE, BRUSH_HEIGHT} SelectedBrush;
    SelectedBrush m_selectedBrush = BRUSH_TEXTURE;

    typedef enum { MAP_SPLAT, MAP_RAIN, MAP_DROPLET} SelectedMap;
    SelectedMap m_selectedMap = MAP_SPLAT;

    typedef enum { NS, SN, WE, EW} SelectedWind;
    SelectedWind m_selectedWind = NS;

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

};

#endif

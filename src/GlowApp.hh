#include <glow-extras/glfw/GlfwApp.hh>

#include <glow/fwd.hh>

#include <glm/glm.hpp>

class GlowApp : public glow::glfw::GlfwApp
{
private:
    glow::SharedVertexArray createPerlinTerrain();
    glm::vec3 mClearColor = {0 / 255.0f, 85 / 255.0f, 159 / 255.0f};
    glm::vec3 mLightDir = normalize(glm::vec3(.2, .7, .7));
    float mLightDis = 2.0f;

    float mAngle = 0.0f;
    float mSpeed = 0.0f;

private:
    glow::SharedProgram mShaderOutput;
    glow::SharedVertexArray mMeshQuad;
    glow::SharedFramebuffer mFramebuffer;
    glow::SharedTextureRectangle mTargetColor;
    glow::SharedTextureRectangle mTargetDepth;

    glow::SharedProgram mShaderObj;
    glow::SharedVertexArray mMeshCube;
    glow::SharedVertexArray mPerlinTest;
    glow::SharedTexture2D mTextureColor;
    glow::SharedTexture2D mTextureNormal;

public:
    // load resources, initialize app
    void init() override;
    // update game logic with fixed timestep
    void update(float elapsedSeconds) override;
    // render game with variable timestep
    void render(float elapsedSeconds) override;
    // called after window is resized
    void onResize(int w, int h) override;
};

#include "GlowApp.hh"

#include <AntTweakBar.h>

#include <fstream>

#include <glm/ext.hpp>

#include <glow/common/scoped_gl.hh>
#include <glow/gl.hh>
#include <glow/objects/Framebuffer.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/TextureRectangle.hh>
#include <glow/objects/VertexArray.hh>

#include <glow-extras/assimp/Importer.hh>
#include <glow-extras/camera/GenericCamera.hh>
#include <glow-extras/geometry/Quad.hh>

using namespace glow;

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
    setTitle("GlowApp");

    // set up tweakbar
    TwAddVarRW(tweakbar(), "bg color", TW_TYPE_COLOR3F, &mClearColor, "group=rendering");
    TwAddVarRW(tweakbar(), "light direction", TW_TYPE_DIR3F, &mLightDir, "group=scene");
    TwAddVarRW(tweakbar(), "light distance", TW_TYPE_FLOAT, &mLightDis, "group=scene step=0.1 min=1 max=100");
    TwAddVarRW(tweakbar(), "rotation speed", TW_TYPE_FLOAT, &mSpeed, "group=scene step=0.1");

    // load object
    mMeshCube = assimp::Importer().load("mesh/cube.obj");
    mShaderObj = Program::createFromFile("shader/obj");
    mTextureColor = Texture2D::createFromFile("texture/rock-albedo.png", ColorSpace::sRGB);
    mTextureNormal = Texture2D::createFromFile("texture/rock-normal.png", ColorSpace::Linear);

    // set up framebuffer and output
    mShaderOutput = Program::createFromFile("shader/output");
    mMeshQuad = geometry::Quad<>().generate();
    mTargetColor = TextureRectangle::create(1, 1, GL_RGB16F);
    mTargetDepth = TextureRectangle::create(1, 1, GL_DEPTH_COMPONENT32);
    mFramebuffer = Framebuffer::create("fColor", mTargetColor, mTargetDepth);
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
    GlfwApp::render(elapsedSeconds); // call to base!

    auto cam = getCamera(); // internal camera from GlfwApp with some default input handling
    auto view = cam->getViewMatrix();
    auto proj = cam->getProjectionMatrix();
    auto camPos = cam->getPosition();

    auto lightPos = normalize(mLightDir) * mLightDis;

    // render to framebuffer
    {
        auto fb = mFramebuffer->bind();
        GLOW_SCOPED(enable, GL_CULL_FACE);  // use backface culling
        GLOW_SCOPED(enable, GL_DEPTH_TEST); // use z-Buffer

        // clear buffer (color + depth)
        {
            auto c = pow(mClearColor, glm::vec3(2.224f)); // inverse gamma correction
            GLOW_SCOPED(clearColor, c.r, c.g, c.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        // draw object
        {
            auto model = glm::rotate(mAngle, glm::vec3(0, 1, 0));

            auto shader = mShaderObj->use();
            shader.setUniform("uView", view);
            shader.setUniform("uProj", proj);
            shader.setUniform("uModel", model);

            shader.setUniform("uLightPos", lightPos);
            shader.setUniform("uCamPos", camPos);

            shader.setTexture("uTexColor", mTextureColor);
            shader.setTexture("uTexNormal", mTextureNormal);

            mMeshCube->bind().draw();
        }
    }

    // render to screen
    // (applies gamma correction and dithering)
    {
        GLOW_SCOPED(disable, GL_CULL_FACE); // no backface culling

        auto shader = mShaderOutput->use();
        shader.setTexture("uTexture", mTargetColor);

        // draw fullscreen quad
        mMeshQuad->bind().draw();
    }
}

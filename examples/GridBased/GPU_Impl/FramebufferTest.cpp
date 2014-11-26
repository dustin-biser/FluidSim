#include "FramebufferTest.hpp"
#include <Rigid3D/Rigid3D.hpp>
using namespace Rigid3D;

#include <iostream>
using namespace std;


//----------------------------------------------------------------------------------------
int main() {
    std::shared_ptr<GlfwOpenGlWindow> demo = FramebufferTest::getInstance();
    demo->create(800,800, "Framebuffer Test");

    return 0;
}

//----------------------------------------------------------------------------------------
std::shared_ptr<GlfwOpenGlWindow> FramebufferTest::getInstance() {
    static GlfwOpenGlWindow * instance = new FramebufferTest();
    if (p_instance == nullptr) {
        p_instance = std::shared_ptr<GlfwOpenGlWindow>(instance);
    }

    return p_instance;
}

//----------------------------------------------------------------------------------------
void FramebufferTest::init() {
    glGenFramebuffers(1, &framebuffer);

    createTextureStorage();
    setupFramebufferAttachments();

    glClearColor(-0.00000001, 9999999.9f, 123.123456789f, 1.0f);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void FramebufferTest::createTextureStorage() {
    glGenTextures(1, &tex2DColorBuffer);
    glBindTexture(GL_TEXTURE_2D, tex2DColorBuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureStorageWidth, textureStorageHeight,
            0, GL_RGB, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void FramebufferTest::setupFramebufferAttachments() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Attach texture2DColorBuffer as COLOR_ATTACHMENT_0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            tex2DColorBuffer, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        throw Rigid3DException("Error. Framebuffer not complete.");
    }

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void FramebufferTest::logic() {
    static bool firstRun = true;
    if (firstRun) {
        firstRun = false;
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glViewport(0, 0, textureStorageWidth, textureStorageHeight);

    float data[3];
    glReadPixels(0, 0, 1, 1, GL_RGB, GL_FLOAT, data);

    cout << "(R,G,B) = (" << data[0] << ", " << data[1] << ", " << data[2] << ")" << endl;

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void FramebufferTest::draw() {
    glFinish();
    CHECK_GL_ERRORS;
}

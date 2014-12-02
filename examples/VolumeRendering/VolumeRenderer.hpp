/**
* @brief VolumeRenderer
*
* @author Dustin Biser
*/
#include <GlfwOpenGlWindow.hpp>
#include <Rigid3D/Rigid3D.hpp>

using namespace Rigid3D;

const int kScreenWidth = 1024;
const int kScreenHeight = 768;

const int kBoundingVolumeWidth  = 512;
const int kBoundingVolumeHeight = 512;
const int kBoundingVolumeDepth  = 512;

const GLuint position_attrib_index = 0;
const GLuint color_attrib_index = 1;

class VolumeRenderer : public GlfwOpenGlWindow {

public:
    ~VolumeRenderer() { }

    static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
    VolumeRenderer() = default; // Singleton. Prevent direct construction.

    GLuint volumeDensity_texture3d;
    GLuint bvFrontFace_texture2d;  // bounding volume front face texture.

    GLuint bvVao;          // Bounding volume Vertex Array Object.
    GLuint bvVertexBuffer; // Bounding volume Vertex Buffer Obejct.
    GLuint bvIndexBuffer;  // Bounding volume Index Buffer Object.

    ShaderProgram shaderProgram_BvEntry;

    virtual void setupGl();
    virtual void init();
    virtual void logic();
    virtual void draw();
    virtual void cleanup();
//    virtual void keyInput(int key, int action, int mods);


    void setupCamera();
    void updateShaderUniforms();
    void initShaders();
    void createTextureStorage();
    void initCubeDensityTexture();
    void setupBoundingCubeVertexData();
    void computeVolumeEntryPoint();
};

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
const GLuint textureCoord_attrib_index = 2;

class VolumeRenderer : public GlfwOpenGlWindow {

public:
    ~VolumeRenderer() { }

    static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
    VolumeRenderer() = default; // Singleton. Prevent direct construction.

    GLuint framebuffer;
    GLuint depth_rbo; // Render Buffer Obejct.

    GLuint volumeDensity_texture3d;
    GLuint bvEntrace_texture2d;  // Bounding volume entrance colors
    GLuint rayDirection_texture2d; // Ray direction texture into bounding volume

    GLuint bvVao;          // Bounding volume Vertex Array Object.
    GLuint bvVertexBuffer; // Bounding volume Vertex Buffer Obejct.
    GLuint bvIndexBuffer;  // Bounding volume Index Buffer Object.

    ShaderProgram shaderProgram_BvEntry;

    virtual void setupGl();
    virtual void init();
    virtual void logic();
    virtual void draw();
    virtual void cleanup();


    void initCamera();
    void updateShaderUniforms();
    void setupShaders();
    void createTextureStorage();
    void createDepthStencilBufferStorage();
    void initCubeDensityTexture();
    void setupBoundingCubeVertexData();
    void computeVolumeEntryPoint();

    void renderBoundingVolume();

    //-- For Rendering Texture To Screen:
    GLuint screenQuadVao;         // Vertex Array Object
    GLuint screenQuadVertBuffer;  // Vertex Buffer Object
    GLuint screenQuadIndexBuffer; // Element Buffer Object
    ShaderProgram shaderProgram_RenderTexture;
    void setupScreenQuadVboData();
    void renderTextureToScreen(GLuint textureName);


    //TODO Dustin - Remove this after testing:
    ShaderProgram shaderProgram_ColorScreen;
};

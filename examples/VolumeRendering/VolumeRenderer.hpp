/**
* @brief VolumeRenderer
*
* @author Dustin Biser
*/
#include <Rigid3D/Rigid3D.hpp>
using namespace Rigid3D;

const GLuint position_attrib_index = 0;
const GLuint color_attrib_index = 1;
const GLuint textureCoord_attrib_index = 2;

class VolumeRenderer {

public:
    VolumeRenderer(uint32 boundingVolumeWidth,
                   uint32 boundingVolumeHeight,
                   uint32 boundingVolumeDepth,
                   uint32 framebufferWidth,
                   uint32 framebufferHeight);

    ~VolumeRenderer();

    void draw(const Camera & camera, float32 rayStepSize, GLuint volumeData_texture3d);

private:
    uint32 boundingVolumeWidth;
    uint32 boundingVolumeHeight;
    uint32 boundingVolumeDepth;
    uint32 framebufferWidth;
    uint32 framebufferHeight;

    GLuint framebuffer;

    GLuint bvEntrance_texture2d;         // Bounding volume entrance colors.
    GLuint rayDirection_texture2d;       // Ray direction texture into bounding volume.
    GLuint accumulatedDensity_texture2d; // Resultant volume rendering.
    GLuint noise_texture2d;

    GLuint bvVao;          // Bounding volume Vertex Array Object.
    GLuint bvVertexBuffer; // Bounding volume Vertex Buffer Object.
    GLuint bvIndexBuffer;  // Bounding volume Index Buffer Object.

    ShaderProgram shaderProgram_BvEntry;
    ShaderProgram shaderProgram_RayDirection;
    ShaderProgram shaderProgram_RayMarch;
    ShaderProgram shaderProgram_NoiseGenerator;

    void updateShaderUniforms(const Camera & camera);
    void setupShaders();
    void createTextureStorage();
    void setupBoundingCubeVertexData();
    void generateNoiseTexture();

    void composeVolumeEntranceTexture();
    void composeRayDirectionTexture();
    void renderVolume(GLuint in_dataTexture3d, float stepSize);

    void marchRaysForward(GLuint in_dataTexture3d,
                          float stepSize);

    void renderBoundingVolume(const ShaderProgram & shader);
    void renderScreenQuad(const ShaderProgram & shader);

    void accqiurePreviousGLSetings();
    void restorePreviousGLSettings();

    //-- For Rendering Texture To Screen:
    GLuint screenQuadVao;         // Vertex Array Object
    GLuint screenQuadVertBuffer;  // Vertex Buffer Object
    GLuint screenQuadIndexBuffer; // Element Buffer Object
    ShaderProgram shaderProgram_RenderTexture;
    void setupScreenQuadVboData();
    void renderTextureToScreen(GLuint textureName);

    //-- Previous OpenGL settings prior to calling draw(...)
    GLfloat prev_color_clear_value[4];
    GLint prev_cull_face;
    GLint prev_cull_face_mode;

};

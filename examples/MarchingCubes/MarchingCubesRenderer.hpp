/**
* @brief MarchingCubesRenderer
*
*/

#pragma once

#include <Rigid3D/Rigid3D.hpp>


const GLuint uvCoord_attrib_index = 0;
const GLuint zLayerCoord_attrib_index = 1;

const GLint textureUnitOffset = 0;


/**
* Given a 3D density texture density of dimension (gridWidth, gridHeight, gridDepth), this
* class will run the Marching Cubes algorithm which will generate
* (gridWidth-1)*(gridHeight-1)*(gridDepth-1) voxels with density values at the corner of
* each voxel.  From there triangles will be generated from voxel edge crossing of the
* iso-surface.
*/
class MarchingCubesRenderer {
    

public:
    MarchingCubesRenderer(Rigid3D::uint32 gridWidth,
                          Rigid3D::uint32 gridHeight,
                          Rigid3D::uint32 gridDepth);

    ~MarchingCubesRenderer();

    void render(const Rigid3D::Camera & camera, GLuint volumeData_texture3d);

private:
    Rigid3D::float32 gridWidth;
    Rigid3D::float32 gridHeight;
    Rigid3D::float32 gridDepth;
    GLsizei numVoxelsPerLayer;

    Rigid3D::ShaderProgram shaderProgram;

    GLuint sampler_volumeData;

    GLuint vao;
    GLuint vbo_voxelUvCoords;  // texture coordinate for lower-left corner of each voxel.
    GLuint vbo_voxelZLayer;  // texture z layer of each voxel.
    GLuint tbo; // transform feedback buffer.

    void setupVoxelUvCoordVboData();
    void setupVao();
    void setupShaders();
    void setShaderUniforms();
    void setupVoxelZLayerVboData();
    void setupSamplerObject();
    void setupTransformFeedbackBuffer();
    void inspectTransformFeedbackBuffer();

};
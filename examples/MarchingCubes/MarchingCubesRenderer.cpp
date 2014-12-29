#include "MarchingCubesRenderer.hpp"
using namespace Rigid3D;


//---------------------------------------------------------------------------------------
MarchingCubesRenderer::MarchingCubesRenderer(
    uint32 gridWidth,
    uint32 gridHeight,
    uint32 gridDepth)
    : gridWidth(gridWidth),
      gridHeight(gridHeight),
      gridDepth(gridDepth),
      numVoxelsPerLayer(0)
{
    setupShaders();
    setShaderUniforms();
    setupVoxelUvCoordVboData();
    setupVoxelZLayerVboData();
    setupVao();
    setupSamplerObject();
}

//---------------------------------------------------------------------------------------
MarchingCubesRenderer::~MarchingCubesRenderer() {

}

//---------------------------------------------------------------------------------------
void MarchingCubesRenderer::setupShaders() {
    shaderProgram.attachVertexShader
            ("examples/MarchingCubes/shaders/MarchingCubes.vs");
    shaderProgram.attachGeometryShader
            ("examples/MarchingCubes/shaders/MarchingCubes.gs");
    shaderProgram.attachFragmentShader
            ("examples/MarchingCubes/shaders/PhongLighting.fs");
    shaderProgram.link();
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::setShaderUniforms() {

    shaderProgram.setUniform("densityTexture", textureUnitOffset);
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::setupVoxelUvCoordVboData() {
    glGenBuffers(1, &vbo_voxelUvCoords);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_voxelUvCoords);

    // Every 2 floats will represent the texture-space coordinate of lower-left corner
    // vertex of voxel cell.
    vector<vec2> texelCoords;

    vec2 uv(0.0f);

    // i spans [0..gridWidth-2]
    // j spans [0..gridHeight-2]
    for (int32 i = 0; i <= gridWidth-2; ++i) {
        for (int32 j = 0; j <= gridHeight-2; ++j) {

            // texel center coordinates
            uv.x = (i + 0.5f) / gridWidth;
            uv.y = (j + 0.5f) / gridHeight;

            texelCoords.push_back(uv);
            ++numVoxelsPerLayer;
        }
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*texelCoords.size(), texelCoords.data(),
            GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::setupVoxelZLayerVboData() {
    glGenBuffers(1, &vbo_voxelZLayer);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_voxelZLayer);

    vector<float32> layerCoords;

    float32 zLayer;

    // i spans [0..gridDepth-2]
    for (int32 i = 0; i <= gridDepth-2; ++i) {

        // texel center z position
        zLayer = (i + 0.5) / gridDepth;

        layerCoords.push_back(zLayer);
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(float32) * layerCoords.size(),
            layerCoords.data(), GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Set Vao Data Bindings
void MarchingCubesRenderer::setupVao() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(uvCoord_attrib_index);

    //-- voxel uv coordinates:
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_voxelUvCoords);

        int32 elementsPerVertex = 2;
        int32 stride = 0;
        int32 offsetToFirstElement = 0;
        glVertexAttribPointer(uvCoord_attrib_index, elementsPerVertex, GL_FLOAT,
                GL_FALSE, stride, reinterpret_cast<void *>(offsetToFirstElement));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        CHECK_GL_ERRORS;
    }

    //-- voxel z layer texture coordinates (instanced):
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_voxelZLayer);

        int32 elementsPerVertex = 1;
        int32 stride = 0;
        int32 offsetToFirstElement = 0;
        glVertexAttribPointer(zLayerCoord_attrib_index, elementsPerVertex, GL_FLOAT,
                GL_FALSE, stride, reinterpret_cast<void *>(offsetToFirstElement));

        // Advance after (gridWidth-1)*(gridHeight-1) GL_POINT primitives have been
        // rendered, which corresponds to one 2D layer of GL_POINTS.
        glVertexAttribDivisor(zLayerCoord_attrib_index, (gridWidth-1)*(gridHeight-1));


        glBindBuffer(GL_ARRAY_BUFFER, 0);
        CHECK_GL_ERRORS;
    }

    //-- Restore Defaults:
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS;
}


//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::render(
        const Rigid3D::Camera & camera,
        GLuint volumeData_texture3d
){
    glBindVertexArray(vao);

    glActiveTexture(GL_TEXTURE0 + textureUnitOffset);
    glBindTexture(GL_TEXTURE_3D, volumeData_texture3d);
    glBindSampler(textureUnitOffset, sampler_volumeData);

    shaderProgram.enable();
    glDrawArraysInstanced(GL_POINTS, 0, numVoxelsPerLayer, gridDepth-1);
    shaderProgram.disable();

    glBindVertexArray(0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::setupSamplerObject() {
    glGenSamplers(1, &sampler_volumeData);
    glSamplerParameterf(sampler_volumeData, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameterf(sampler_volumeData, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    CHECK_GL_ERRORS;
}

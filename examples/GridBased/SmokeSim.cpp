#include "SmokeSim.hpp"
#include "SmokeGraphics.hpp"
#include "Utils.hpp"
#include "Interp.hpp"

#include <cmath>

#include <glm/glm.hpp>

#include <iostream>
using namespace std;

//----------------------------------------------------------------------------------------
int main() {
    shared_ptr<GlfwOpenGlWindow> smokeDemo = SmokeSim::getInstance();
    smokeDemo->create(kScreenWidth, kScreenHeight, "2D Smoke Simulation");

    return 0;
}

//---------------------------------------------------------------------------------------
SmokeSim::SmokeSim()
    : m_velocity(Grid<vec2>(kGridHeight+1, kGridWidth+1)),
      m_pressure(Grid<float32>(kGridHeight, kGridWidth))
{

}

//---------------------------------------------------------------------------------------
shared_ptr<GlfwOpenGlWindow> SmokeSim::getInstance() {
    static GlfwOpenGlWindow * instance = new SmokeSim();
    if (p_instance == nullptr) {
        p_instance = shared_ptr<GlfwOpenGlWindow>(instance);
    }

    return p_instance;
}

//----------------------------------------------------------------------------------------
void SmokeSim::init() {
    cout << "\nInitializing Simulation." << endl;


    // TODO Dustin - Create Grids:
    // Create Staggered velocity grid
    // Create pressure grid

    createTextures();
}

//----------------------------------------------------------------------------------------
void SmokeSim::createTextures() {
    //--Setup ink texture.
    // The ink texture will represent a grid of size (kGridWidth, kGridHeight).
    // Type: R16
    // Red Channel - ink density/opacity
    {
        glGenTextures(1, &m_tex2D_ink);
        glBindTexture(GL_TEXTURE_2D, m_tex2D_ink);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width = kGridWidth;
        int height = kGridHeight;
        float initialInk [width][height];
        utils::fillArray(initialInk, 0.0f, height, width);

        // Internally store texture as RG16 - two channels of half-floats:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_FLOAT, initialInk);
    }

    // Unbind the GL_TEXTURE_2D target.
    glBindTexture(GL_TEXTURE_2D, 0);

    CHECK_GL_ERRORS;
}


//----------------------------------------------------------------------------------------
void SmokeSim::logic() {
    advect(m_velocity, m_pressure, kDt);
}

//----------------------------------------------------------------------------------------
void SmokeSim::draw() {
}

//----------------------------------------------------------------------------------------
void SmokeSim::keyInput(int key, int action, int mods) {

}

//----------------------------------------------------------------------------------------
void SmokeSim::cleanup() {
    cout << "Simulation Clean Up" << endl;

    //-- Delete Textures:
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &m_tex2D_ink);

    cout << " ... Good Bye." << endl;
}

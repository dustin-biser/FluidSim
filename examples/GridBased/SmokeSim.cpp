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
SmokeSim::SmokeSim() {

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

    createTextures();
    initGrids();
}

//----------------------------------------------------------------------------------------
void SmokeSim::initGrids() {
    m_pressure = Grid<float32>(kGridWidth, kGridHeight, kDx, vec2(0,0));
    m_pressure.setAll(0);


    Grid<float32> u(m_pressure.width()+1,
                    m_pressure.height(),
                    kDx,
                    vec2(0, 0.5*kDx));

    Grid<float32> v(m_pressure.width(),
                    m_pressure.height()+1,
                    kDx,
                    vec2(0.5f*kDx, 0));

    m_velocity = StaggeredGrid<float32>(std::move(u), std::move(v));

    // Set initial velocity components:
    m_velocity.u.setAll(0);
    m_velocity.v.setAll(0.01f);
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

        // Internally store texture as R16 - one channel of half-floats:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_FLOAT, initialInk);
    }

    // Unbind the GL_TEXTURE_2D target.
    glBindTexture(GL_TEXTURE_2D, 0);

    CHECK_GL_ERRORS;
}


//----------------------------------------------------------------------------------------
void SmokeSim::logic() {
//    advect(m_velocity, m_pressure, kDt);


    // Apply the first 3 operators in Equation 12.
//    u = advect(u);
//    u = diffuse(u);
//    u = addForces(u);
//// Now apply the projection operator to the result.
//    p = computePressure(u);
//    u = subtractPressureGradient(u, p);
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

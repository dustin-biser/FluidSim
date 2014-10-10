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
template <typename U, typename V>
void SmokeSim::advect(const Grid<U> & velocityField, TimeStep dt, Grid<V> & quantity) {

    const Grid<U> & u = velocityField;
    Grid<V> & q = quantity;

    assert( (u.width() == q.width()+1) &&
             u.height() == q.height()+1 );


    uint32 u_width = u.width();
    uint32 u_height = u.height();


    vec2 x_g; // Grid location to update.
    vec2 x_p; // Grid location of the particle that will be at x_g in the next future time step.
    vec2 x_mid; // Mid point to help aid in better backtracing of particle location.
    Grid<V> q_new = quantity; // Deep copy.
    for(uint32 row(0); row < u_height; ++row) {
        for(uint32 col(0); col < u_width; ++col) {
            //-- Two stage Runge-Kutta method:
            {
                x_g = vec2(row, col);
                x_mid = x_g - inv_kDx * (0.5f * dt * u(row, col));
                x_p = x_g - inv_kDx * (dt * u(x_mid.x, x_mid.y));
            }

           q_new(row,col) = bilinear(q, x_p);
        }
    }

    // Move the advected grid data into q.
    q = std::move(q_new);
}

//----------------------------------------------------------------------------------------
void SmokeSim::logic() {
    advect(m_velocity, kDt, m_pressure);
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

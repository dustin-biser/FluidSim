/**
* Advect_Test.cpp
*
* @author Dustin Biser
*/

#include "gtest/gtest.h"
#include "FluidSim/Advect.hpp"

using namespace FluidSim;


namespace {  // limit class visibility to this file.

class Advect_Test : public ::testing::Test {
protected:
    static StaggeredGrid<float32> velocity; // velocity field.

    static Grid<float32> q;     // quantity to advect.
    static float32 dx;          // Grid cell length.
    static float32 dt;          // Timestep.

    // Ran before each test.
    virtual void SetUp() {

        dx = 1.0f;
        dt = 1.0f;

        // Data within y_velocity StaggeredGrid:
        //   -----v02--------v12----
        //   |          |          |
        //  u01        u11        u21
        //   |          |          |
        //   -----v01--------v11----
        //   |          |          |
        //  u00        u10        u20
        //   |          |          |
        //   -----v00--------v10----
        Grid<float32> u(q.width()+1, q.height());
        Grid<float32> v(q.width(), q.height()+1);

        u.setAll(0);
        v.setAll(0);
        velocity = StaggeredGrid<float32>(u,v);

    }

};

} // end namespace

StaggeredGrid<float32> Advect_Test::velocity;
Grid<float32> Advect_Test::q = Grid<float32>(2,2);
float32 Advect_Test::dx;
float32 Advect_Test::dt;


//------------------------------------------------------------------------------
TEST_F(Advect_Test, advect_up) {
    // Data within q Grid:
    //  ---------
    // | 0  | 0  |
    // |---------|
    // | 1  | 1  |
    //  ---------
    q(0, 1) = 0.0f; q(1, 1) = 0.0f;
    q(0, 0) = 1.0f; q(1, 0) = 1.0f;

    velocity.u.setAll(0);
    velocity.v.setAll(dx);
    advect(velocity, q, dx, dt);

    EXPECT_FLOAT_EQ(1, q(0,0));
    EXPECT_FLOAT_EQ(1, q(1,0));
    EXPECT_FLOAT_EQ(1, q(0,1));
    EXPECT_FLOAT_EQ(1, q(1,1));
}

//------------------------------------------------------------------------------
TEST_F(Advect_Test, advect_down) {
    // Data within q Grid:
    //  ---------
    // | 1  | 1  |
    // |---------|
    // | 0  | 0  |
    //  ---------
    q(0, 1) = 1.0f; q(1, 1) = 1.0f;
    q(0, 0) = 0.0f; q(1, 0) = 0.0f;

    velocity.u.setAll(0);
    velocity.v.setAll(-dx);
    advect(velocity, q, dx, dt);

    EXPECT_FLOAT_EQ(1, q(0,0));
    EXPECT_FLOAT_EQ(1, q(1,0));
    EXPECT_FLOAT_EQ(1, q(0,1));
    EXPECT_FLOAT_EQ(1, q(1,1));
}

//------------------------------------------------------------------------------
TEST_F(Advect_Test, advect_right) {
    // Data within q Grid:
    //  ---------
    // | 1  | 0  |
    // |---------|
    // | 1  | 0  |
    //  ---------
    q(0, 1) = 1.0f; q(1, 1) = 0.0f;
    q(0, 0) = 1.0f; q(1, 0) = 0.0f;

    velocity.u.setAll(dx);
    velocity.v.setAll(0);
    advect(velocity, q, dx, dt);

    EXPECT_FLOAT_EQ(1, q(0,0));
    EXPECT_FLOAT_EQ(1, q(1,0));
    EXPECT_FLOAT_EQ(1, q(0,1));
    EXPECT_FLOAT_EQ(1, q(1,1));
}

//------------------------------------------------------------------------------
TEST_F(Advect_Test, advect_left) {
    // Data within q Grid:
    //  ---------
    // | 0  | 1  |
    // |---------|
    // | 0  | 1  |
    //  ---------
    q(0, 1) = 0.0f; q(1, 1) = 1.0f;
    q(0, 0) = 0.0f; q(1, 0) = 1.0f;

    velocity.u.setAll(-dx);
    velocity.v.setAll(0);
    advect(velocity, q, dx, dt);

    EXPECT_FLOAT_EQ(1, q(0,0));
    EXPECT_FLOAT_EQ(1, q(1,0));
    EXPECT_FLOAT_EQ(1, q(0,1));
    EXPECT_FLOAT_EQ(1, q(1,1));
}

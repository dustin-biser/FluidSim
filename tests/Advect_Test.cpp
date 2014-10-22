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
    static StaggeredGrid<float32> velocity; // Velocity field.
    static Grid<float32> q;  // Quantity to advect.
    static constexpr float32 kCellLength = 1.0f; // Grid cell length.
    static constexpr float32 kDt = 1.0f; // Timestep.

    // Ran before each test.
    virtual void SetUp() {
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
        Grid<float32> u(q.width()+1, q.height(), kCellLength, vec2(0,0));
        Grid<float32> v(q.width(), q.height()+1, kCellLength, vec2(0,0));

        u.setAll(0);
        v.setAll(0);
        velocity = StaggeredGrid<float32>(u,v);
    }

};

} // end namespace

constexpr float32 Advect_Test::kCellLength;
constexpr float32 Advect_Test::kDt;
StaggeredGrid<float32> Advect_Test::velocity;
Grid<float32> Advect_Test::q = Grid<float32>(2,2,kCellLength,vec2(0,0));


//------------------------------------------------------------------------------
TEST_F(Advect_Test, advect_up) {
    // Data within q Grid:
    //  ---------
    // | 0  | 0  |
    // |---------|
    // | 1  | 1  |
    //  ---------
    q(0,1) = 0.0f; q(1,1) = 0.0f;
    q(0,0) = 1.0f; q(1,0) = 1.0f;

    velocity.u.setAll(0);
    velocity.v.setAll(kCellLength);
    advect(velocity, q, kDt);

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
    velocity.v.setAll(-kCellLength);
    advect(velocity, q, kDt);

    EXPECT_FLOAT_EQ(1, q(0,0));
    EXPECT_FLOAT_EQ(1, q(1,0));
    EXPECT_FLOAT_EQ(1, q(0,1));
    EXPECT_FLOAT_EQ(1, q(1,1));
}

//------------------------------------------------------------------------------
TEST_F(Advect_Test, advect_velocity_u) {
    // Data layout in staggered_grid:
    //
    // v Grid
    // * -- *
    // |    |
    // * -- *
    // |    |
    // * -- *
    // where * = 0
    //
    // u Grid pre-advection
    // 1 -- 2 -- 3
    // |    |    |
    // 0 -- 1 -- 2
    //
    // u Grid post-advection
    // 1 -- 1 --1.5
    // |    |    |
    // 0 --0.5-- 1

    Grid<float32> v(2,3, kCellLength, vec2(0.5*kCellLength, 0));
    v.setAll(0);

    Grid<float32> u(3,2, kCellLength, vec2(0, 0.5*kCellLength));
    u(0,1) = 1; u(1,1) = 2; u(2,1) = 3;
    u(0,0) = 0; u(1,0) = 1; u(2,0) = 2;

    StaggeredGrid<float32> staggered_grid =
            StaggeredGrid<float32>(std::move(u), std::move(v));

    advect(staggered_grid, staggered_grid.u, kDt);

    EXPECT_FLOAT_EQ(0, staggered_grid.u(0,0));
    EXPECT_FLOAT_EQ(0.5f, staggered_grid.u(1,0));
    EXPECT_FLOAT_EQ(1, staggered_grid.u(2,0));
    EXPECT_FLOAT_EQ(1, staggered_grid.u(0,1));
    EXPECT_FLOAT_EQ(1, staggered_grid.u(1,1));
    EXPECT_FLOAT_EQ(1.5f, staggered_grid.u(2,1));
}

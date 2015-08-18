/**
 * Interp_Test.cpp
 *
 * @author Dustin Biser
 */

#include "gtest/gtest.h"
#include "FluidSim/Interp.hpp"
using FluidSim::GridInterp::linear;
using FluidSim::GridInterp::bilinear;

using namespace FluidSim;


namespace {  // limit class visibility to this file.

class Interp_Test : public ::testing::Test {
protected:
    static Grid<float32> float_grid;
    static StaggeredGrid<float32> staggered_grid;
    static constexpr float32 kCellLength = 1.0f;

    // Ran before each test.
    virtual void SetUp() {
        // Data layout in staggered_grid:
        //
        // v Grid
        // 4 -- 5
        // |    |
        // 2 -- 3
        // |    |
        // 0 -- 1
        //
        // u Grid
        // 3 -- 4 -- 5
        // |    |    |
        // 0 -- 1 -- 2

        Grid<float32> v(2,3, kCellLength, vec2(0.5*kCellLength, 0));
        v(0,0) = 0;
        v(1,0) = 1;
        v(0,1) = 2;
        v(1,1) = 3;
        v(0,2) = 4;
        v(1,2) = 5;

        Grid<float32> u(3,2, kCellLength, vec2(0, 0.5*kCellLength));
        u(0,0) = 0;
        u(1,0) = 1;
        u(2,0) = 2;
        u(0,1) = 3;
        u(1,1) = 4;
        u(2,1) = 5;

        staggered_grid = StaggeredGrid<float32>(std::move(u),std::move(v));



        // Put data in float_grid:
        // 4 -- 5
        // |    |
        // 2 -- 3
        // |    |
        // 0 -- 1
        float_grid(0, 0) = 0;
        float_grid(1, 0) = 1;
        float_grid(0, 1) = 2;
        float_grid(1, 1) = 3;
        float_grid(0, 2) = 4;
        float_grid(1, 2) = 5;
    }

};

} // end namespace

Grid<float32> Interp_Test::float_grid = Grid<float32>(2,3, kCellLength, vec2(0,0));
StaggeredGrid<float32> Interp_Test::staggered_grid;


//------------------------------------------------------------------------------
TEST_F(Interp_Test, test_setup) {
    EXPECT_EQ(staggered_grid.u.width(), 3);
    EXPECT_EQ(staggered_grid.u.height(), 2);
    EXPECT_EQ(staggered_grid.v.width(), 2);
    EXPECT_EQ(staggered_grid.v.height(), 3);

    EXPECT_EQ(float_grid.width(), 2);
    EXPECT_EQ(float_grid.height(), 3);
}

//------------------------------------------------------------------------------
// Test float_grid
//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_float_grid_cell_centers) {
    EXPECT_FLOAT_EQ(1.5f, linear(float_grid, vec2(0.5f, 0.5f)) );
    EXPECT_FLOAT_EQ(3.5f, linear(float_grid, vec2(0.5f, 1.5f)) );
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_float_grid_clamp_to_zero_zero) {
    EXPECT_FLOAT_EQ(0.0f, linear(float_grid, vec2(-1.0f, 0.0f)) );
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_float_grid_clamp_to_one_one) {
    EXPECT_FLOAT_EQ(1.0f, linear(float_grid, vec2(2.0f, 0.0f)) );
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_float_grid_clamp_to_four_four) {
    EXPECT_FLOAT_EQ(4.0f, linear(float_grid, vec2(-1.0f, 4.3f)) );
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_float_grid_clamp_to_five_five) {
    EXPECT_FLOAT_EQ(5.0f, linear(float_grid, vec2(1.5f, 5.01f)) );
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_float_grid_midpoint) {
    EXPECT_FLOAT_EQ(2.5f, linear(float_grid, vec2(0.5f, 1.0f)) );
}

//------------------------------------------------------------------------------
// Test staggered_grid
//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_staggered_grid_origins) {
    vec2 result = bilinear(staggered_grid, vec2(0,0));
    EXPECT_FLOAT_EQ(0, result[0]);
    EXPECT_FLOAT_EQ(0, result[1]);
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_staggered_grid_half_half) {
    vec2 result = bilinear(staggered_grid, vec2(0.5,0.5));
    EXPECT_FLOAT_EQ(0.5f, result[0]);
    EXPECT_FLOAT_EQ(1.0f, result[1]);
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_staggered_grid_one_one) {
    vec2 result = bilinear(staggered_grid, vec2(1,1));
    EXPECT_FLOAT_EQ(2.5f, result[0]);
    EXPECT_FLOAT_EQ(2.5f, result[1]);
}


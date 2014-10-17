/**
 * Interp_Test.cpp
 *
 * @author Dustin Biser
 */

#include "gtest/gtest.h"
#include "FluidSim/Interp.hpp"

using namespace FluidSim;


namespace {  // limit class visibility to this file.

class Interp_Test : public ::testing::Test {
protected:
    static Grid<vec2> vec2_grid;
    static Grid<float32> float_grid;

    // Ran before each test.
    virtual void SetUp() {

        // Put data in vec2_grid:
        // (0,2) -- (1,2) -- (2,2)
        //   |        |        |
        // (0,1) -- (1,1) -- (2,1)
        //   |        |        |
        // (0,0) -- (1,0) -- (2,0)
        vec2_grid(0, 0) = vec2(0.0f, 0.0f);
        vec2_grid(1, 0) = vec2(1.0f, 0.0f);
        vec2_grid(2, 0) = vec2(2.0f, 0.0f);

        vec2_grid(0, 1) = vec2(0.0f, 1.0f);
        vec2_grid(1, 1) = vec2(1.0f, 1.0f);
        vec2_grid(2, 1) = vec2(2.0f, 1.0f);

        vec2_grid(0, 2) = vec2(0.0f, 2.0f);
        vec2_grid(1, 2) = vec2(1.0f, 2.0f);
        vec2_grid(2, 2) = vec2(2.0f, 2.0f);


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

Grid<vec2> Interp_Test::vec2_grid = Grid<vec2>(3,3);
Grid<float32> Interp_Test::float_grid = Grid<float32>(2,3);


//------------------------------------------------------------------------------
TEST_F(Interp_Test, test_setup) {
    EXPECT_EQ(vec2_grid.width(), 3);
    EXPECT_EQ(vec2_grid.height(), 3);

    EXPECT_EQ(float_grid.width(), 2);
    EXPECT_EQ(float_grid.height(), 3);
}

//------------------------------------------------------------------------------
// Test float_grid
//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_float_grid_cell_centers) {
    EXPECT_FLOAT_EQ(1.5f, bilinear(float_grid, vec2(0.5f, 0.5f)) );
    EXPECT_FLOAT_EQ(3.5f, bilinear(float_grid, vec2(0.5f, 1.5f)) );
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_float_grid_clamp_to_zero_zero) {
    EXPECT_FLOAT_EQ(0.0f, bilinear(float_grid, vec2(-1.0f, 0.0f)) );
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_float_grid_clamp_to_one_one) {
    EXPECT_FLOAT_EQ(1.0f, bilinear(float_grid, vec2(2.0f, 0.0f)) );
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_float_grid_clamp_to_four_four) {
    EXPECT_FLOAT_EQ(4.0f, bilinear(float_grid, vec2(-1.0f, 4.3f)) );
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_float_grid_clamp_to_five_five) {
    EXPECT_FLOAT_EQ(5.0f, bilinear(float_grid, vec2(1.5f, 5.01f)) );
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_float_grid_midpoint) {
    EXPECT_FLOAT_EQ(2.5f, bilinear(float_grid, vec2(0.5f, 1.0f)) );
}



//------------------------------------------------------------------------------
// Test vec2_grid
//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_vec2_grid_cell_centers) {
    vec2 result = bilinear(vec2_grid, vec2(0.5f, 0.5f));
    EXPECT_FLOAT_EQ(0.5f, result.x);
    EXPECT_FLOAT_EQ(0.5f, result.y);

    result = bilinear(vec2_grid, vec2(1.5f, 0.5f));
    EXPECT_FLOAT_EQ(1.5f, result.x);
    EXPECT_FLOAT_EQ(0.5f, result.y);

    result = bilinear(vec2_grid, vec2(0.5f, 1.5f));
    EXPECT_FLOAT_EQ(0.5f, result.x);
    EXPECT_FLOAT_EQ(1.5f, result.y);

    result = bilinear(vec2_grid, vec2(1.5f, 1.5f));
    EXPECT_FLOAT_EQ(1.5f, result.x);
    EXPECT_FLOAT_EQ(1.5f, result.y);
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_vec2_grid_clamp_to_zero_zero) {
    vec2 result = bilinear(vec2_grid, vec2(-0.5f, -0.5f));
    EXPECT_FLOAT_EQ(0.0f, result.x);
    EXPECT_FLOAT_EQ(0.0f, result.y);
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_vec2_grid_clamp_to_two_two) {
    vec2 result = bilinear(vec2_grid, vec2(2.1f, 2.2f));
    EXPECT_FLOAT_EQ(2.0f, result.x);
    EXPECT_FLOAT_EQ(2.0f, result.y);
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_vec2_grid_clamp_to_two_three_halves) {
    vec2 result = bilinear(vec2_grid, vec2(2.1f, 1.5f));
    EXPECT_FLOAT_EQ(2.0f, result.x);
    EXPECT_FLOAT_EQ(1.5f, result.y);
}

//------------------------------------------------------------------------------
TEST_F(Interp_Test, bilinear_vec2_grid_midpoint) {
    vec2 result = bilinear(vec2_grid, vec2(1.0f, 1.0f));
    EXPECT_FLOAT_EQ(1.0f, result.x);
    EXPECT_FLOAT_EQ(1.0f, result.y);
}

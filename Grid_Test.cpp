/**
* Grid_Test.cpp
*
* @author Dustin Biser
*/

#include "gtest/gtest.h"
#include "Grid.hpp"
#include "NumericTypes.hpp"

#include <utility>

using namespace FluidSim;


namespace {  // limit class visibility to this file.

class Grid_Test : public ::testing::Test {
protected:
    static constexpr float32 kCellLength = 1.0f;

    // Ran before each test.
    virtual void SetUp() {

    }

};

} // end namespace


//------------------------------------------------------------------------------
TEST_F(Grid_Test, move_constructor) {
    Grid<int32> a(2,2,kCellLength,vec2(0.0));

    a(0,0) = 1;
    a(0,1) = 2;
    a(1,0) = 3;
    a(1,1) = 4;

    Grid<int32> b(std::move(a));

    EXPECT_EQ(a.width(), 0);
    EXPECT_EQ(a.height(), 0);

    EXPECT_EQ(b.width(), 2);
    EXPECT_EQ(b.height(), 2);

    EXPECT_EQ(b(0,0), 1);
    EXPECT_EQ(b(0,1), 2);
    EXPECT_EQ(b(1,0), 3);
    EXPECT_EQ(b(1,1), 4);
}

//------------------------------------------------------------------------------
TEST_F(Grid_Test, move_assignment) {
    Grid<int32> a(1,2,kCellLength,vec2(0.0));

    a(0,0) = 1;
    a(0,1) = 2;

    Grid<int32> b = std::move(a);

    EXPECT_EQ(a.width(), 0);
    EXPECT_EQ(a.height(), 0);

    EXPECT_EQ(b.width(), 1);
    EXPECT_EQ(b.height(), 2);

    EXPECT_EQ(b(0,0), 1);
    EXPECT_EQ(b(0,1), 2);
}




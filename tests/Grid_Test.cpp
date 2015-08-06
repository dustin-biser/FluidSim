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

class GridSpec_Test : public ::testing::Test {
	// empty
};

} // end namespace

//------------------------------------------------------------------------------
// GridSpec Tests
//------------------------------------------------------------------------------
TEST_F(GridSpec_Test, equals) {
	GridSpec a;
	a.width = 1;
	a.height = 1;
	a.cellLength = 0.5f;
	a.origin = vec2(0.0f);

	GridSpec b;
	b.width = 1;
	b.height = 1;
	b.cellLength = 0.5f;
	b.origin = vec2(0.0f);

	EXPECT_TRUE(a == b);
}

//------------------------------------------------------------------------------
TEST_F(GridSpec_Test, not_equals1) {
	GridSpec a;
	a.width = 2; // <-- difference
	a.height = 1;
	a.cellLength = 0.5f;
	a.origin = vec2(0.0f);

	GridSpec b;
	b.width = 1;
	b.height = 1;
	b.cellLength = 0.5f;
	b.origin = vec2(0.0f);

	EXPECT_TRUE(a != b);
}

//------------------------------------------------------------------------------
TEST_F(GridSpec_Test, not_equals2) {
	GridSpec a;
	a.width = 1;
	a.height = 3; // <-- difference
	a.cellLength = 0.5f;
	a.origin = vec2(0.0f);

	GridSpec b;
	b.width = 1;
	b.height = 1;
	b.cellLength = 0.5f;
	b.origin = vec2(0.0f);

	EXPECT_TRUE(a != b);
}

//------------------------------------------------------------------------------
TEST_F(GridSpec_Test, not_equals3) {
	GridSpec a;
	a.width = 1;
	a.height = 1;
	a.cellLength = 1.5f; // <-- difference
	a.origin = vec2(0.0f);

	GridSpec b;
	b.width = 1;
	b.height = 1;
	b.cellLength = 0.5f;
	b.origin = vec2(0.0f);

	EXPECT_TRUE(a != b);
}

//------------------------------------------------------------------------------
TEST_F(GridSpec_Test, not_equals4) {
	GridSpec a;
	a.width = 1;
	a.height = 1;
	a.cellLength = 1.5f;
	a.origin = vec2(0.0f);

	GridSpec b;
	b.width = 1;
	b.height = 1;
	b.cellLength = 0.5f;
	b.origin = vec2(0.0f, -1.0f); // <-- difference

	EXPECT_TRUE(a != b);
}


//------------------------------------------------------------------------------
// Grid Tests
//------------------------------------------------------------------------------
TEST_F(Grid_Test, move_constructor) {
    Grid<int32> a(2,2,kCellLength,vec2(0.0f));

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
    Grid<int32> a(1,2,kCellLength,vec2(0.0f));

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

//------------------------------------------------------------------------------
TEST_F(Grid_Test, divide_equals) {
	GridSpec gridSpec;
	gridSpec.width = 2;
	gridSpec.height = 2;
	gridSpec.cellLength = 1.0f;
	gridSpec.origin = vec2(0.0f);

	Grid<float32> a(gridSpec);
	Grid<float32> b(gridSpec);

	a.setAll(4.0f);
	b.setAll(2.0f);

	a /= b;

	EXPECT_FLOAT_EQ(2.0f, a(0,0));
	EXPECT_FLOAT_EQ(2.0f, a(1,0));
	EXPECT_FLOAT_EQ(2.0f, a(0,1));
	EXPECT_FLOAT_EQ(2.0f, a(1,1));

	EXPECT_FLOAT_EQ(2.0f, b(0,0));
	EXPECT_FLOAT_EQ(2.0f, b(1,0));
	EXPECT_FLOAT_EQ(2.0f, b(0,1));
	EXPECT_FLOAT_EQ(2.0f, b(1,1));
}




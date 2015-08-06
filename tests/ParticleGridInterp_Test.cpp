/**
* ParticleGridInterp_Test.cpp
*
* @author Dustin Biser
*/

#include "gtest/gtest.h"

#include "FluidSim/ParticleGridInterp.hpp"
#include "FluidSim/Utils.hpp"
using namespace FluidSim;
using namespace std;


namespace {  // limit class visibility to this file.

const float32 epsilon = 1.0e-7f;


// Linear interpolation kernel
static float32 linear(const vec2 & x, float32 h) {
	float32 inv_h = 1.0f / h;
	float32 a = std::max(1.0f - abs(x.x * inv_h), 0.0f);
	float32 b = std::max(1.0f - abs(x.y * inv_h), 0.0f);

	return a * b;
}

//----------------------------------------------------------------------------------------
// Test Fixture
//----------------------------------------------------------------------------------------
class ParticleGridInterp_Test : public ::testing::Test {
protected:
	static const float32 kCellLength;

	Grid<float32> grid2x2;
	Grid<float32> weights2x2;

	// Ran before each test.
	virtual void SetUp() {
		//--Initialize grid2x2:
		{
			GridSpec grid2x2_spec;
			grid2x2_spec.width = 2;
			grid2x2_spec.height = 2;
			grid2x2_spec.cellLength = kCellLength;
			grid2x2_spec.origin = vec2(0.0f);

			grid2x2 = Grid<float32>(grid2x2_spec);
			grid2x2.setAll(0.0f);

			weights2x2 = Grid<float32>(grid2x2_spec);
			weights2x2.setAll(0.0f);

		}
	}

};

const float32 ParticleGridInterp_Test::kCellLength = 1.0f;

} // end namespace


//----------------------------------------------------------------------------------------
// Tests
//----------------------------------------------------------------------------------------
// One particle at grid center.
TEST_F(ParticleGridInterp_Test, grid2x2_test1) {
	vector<vec2> positions = { vec2(0.5f,0.5f) };

	vector<float32> attributes = { 1.0f };

	interpParticleToGrid(
			grid2x2, weights2x2, positions, attributes, linear, kCellLength
	);

	EXPECT_FLOAT_EQ(grid2x2(0,0), 1.0f);
	EXPECT_FLOAT_EQ(grid2x2(1,0), 1.0f);
	EXPECT_FLOAT_EQ(grid2x2(0,1), 1.0f);
	EXPECT_FLOAT_EQ(grid2x2(1,1), 1.0f);
}

//----------------------------------------------------------------------------------------
// One particle at bottom middle of grid.
TEST_F(ParticleGridInterp_Test, grid2x2_test2) {
	vector<vec2> positions = { vec2(0.5f, 0.0f) };

	vector<float32> attributes = { 1.0f };

	interpParticleToGrid(
			grid2x2, weights2x2, positions, attributes, linear, kCellLength
	);

	EXPECT_FLOAT_EQ(grid2x2(0,0), 1.0f);
	EXPECT_FLOAT_EQ(grid2x2(1,0), 1.0f);
	EXPECT_FLOAT_EQ(grid2x2(0,1), 0.0f);
	EXPECT_FLOAT_EQ(grid2x2(1,1), 0.0f);
}

//----------------------------------------------------------------------------------------
// One particle at top middle of grid.
TEST_F(ParticleGridInterp_Test, grid2x2_test3) {
	vector<vec2> positions = { vec2(0.5f, 1.0f - epsilon) };

	vector<float32> attributes = { 1.0f };

	interpParticleToGrid(
			grid2x2, weights2x2, positions, attributes, linear, kCellLength
	);

	EXPECT_TRUE(approxEqual(grid2x2(0,0), 0.0f));
	EXPECT_TRUE(approxEqual(grid2x2(1,0), 0.0f));
	EXPECT_FLOAT_EQ(grid2x2(0,1), 1.0f);
	EXPECT_FLOAT_EQ(grid2x2(1,1), 1.0f);
}

//----------------------------------------------------------------------------------------
// One particle at left middle of grid.
TEST_F(ParticleGridInterp_Test, grid2x2_test4) {
	vector<vec2> positions = { vec2(0.0f, 0.5f) };

	vector<float32> attributes = { 1.0f };

	interpParticleToGrid(
			grid2x2, weights2x2, positions, attributes, linear, kCellLength
	);

	EXPECT_FLOAT_EQ(grid2x2(0,0), 1.0f);
	EXPECT_FLOAT_EQ(grid2x2(1,0), 0.0f);
	EXPECT_FLOAT_EQ(grid2x2(0,1), 1.0f);
	EXPECT_FLOAT_EQ(grid2x2(1,1), 0.0f);
}

//----------------------------------------------------------------------------------------
// One particle at right middle of grid.
TEST_F(ParticleGridInterp_Test, grid2x2_test5) {
	vector<vec2> positions = { vec2(1.0f - epsilon, 0.5f) };

	vector<float32> attributes = { 1.0f };

	interpParticleToGrid(
			grid2x2, weights2x2, positions, attributes, linear, kCellLength
	);

	EXPECT_NEAR(grid2x2(0,0), 0.0f, epsilon);
	EXPECT_FLOAT_EQ(grid2x2(1,0), 1.0f);
	EXPECT_NEAR(grid2x2(0,1), 0.0f, epsilon);
	EXPECT_FLOAT_EQ(grid2x2(1,1), 1.0f);
}

//----------------------------------------------------------------------------------------
// Two particles with different values.
TEST_F(ParticleGridInterp_Test, grid2x2_test6) {
	vector<vec2> positions = {
			vec2(0.25f, 0.5f), // particle 1
			vec2(0.75f, 0.5f), // particle 2
	};

	vector<float32> attributes = {
			1.0f,  // particle 1
			2.0f   // particle 2
	};

	interpParticleToGrid(
			grid2x2, weights2x2, positions, attributes, linear, kCellLength
	);

	EXPECT_NEAR(grid2x2(0,0), 1.25f, epsilon);
	EXPECT_NEAR(grid2x2(1,0), 1.75f, epsilon);
	EXPECT_NEAR(grid2x2(0,1), 1.25f, epsilon);
	EXPECT_NEAR(grid2x2(1,1), 1.75f, epsilon);
}


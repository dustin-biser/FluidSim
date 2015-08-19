/**
* ParticleGridInterp_Test.cpp
*
* @author Dustin Biser
*/

#include <gtest/gtest.h>

#include "FluidSim/Interp.hpp"
#include "FluidSim/ParticleGridInterp.hpp"
using namespace FluidSim;
using namespace std;


namespace {  // limit class visibility to this file.

const float32 epsilon = 1.0e-7f;
const float32 half_eps = 0.5*epsilon;


//----------------------------------------------------------------------------------------
// Test Fixture
//----------------------------------------------------------------------------------------
class ParticleGridInterp_Test : public ::testing::Test {
protected:
	static const float32 kCellLength;

	Grid<float32> grid2x2;
	Grid<float32> weights2x2;

	Grid<float32> grid3x2;
	Grid<float32> weights3x2;

	Grid<float32> grid4x3;
	Grid<float32> weights4x3;

	// Ran before each test.
	virtual void SetUp() {
		//--Initialize grid2x2 and weights2x2:
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

		//--Initialize grid3x2 and weights3x2:
		{
			GridSpec grid3x2_spec;
			grid3x2_spec.width = 3;
			grid3x2_spec.height = 2;
			grid3x2_spec.cellLength = kCellLength;
			grid3x2_spec.origin = vec2(0.0f);

			grid3x2 = Grid<float32>(grid3x2_spec);
			grid3x2.setAll(0.0f);

			weights3x2 = Grid<float32>(grid3x2_spec);
			weights3x2.setAll(0.0f);

		}

		//--Initialize grid4x3 and weights4x3:
		{
			GridSpec grid4x3_spec;
			grid4x3_spec.width = 4;
			grid4x3_spec.height = 3;
			grid4x3_spec.cellLength = kCellLength;
			grid4x3_spec.origin = vec2(0.0f);

			grid4x3 = Grid<float32>(grid4x3_spec);
			grid4x3.setAll(0.0f);

			weights4x3 = Grid<float32>(grid4x3_spec);
			weights4x3.setAll(0.0f);

		}
	}

};

const float32 ParticleGridInterp_Test::kCellLength = 1.0f;

} // end namespace


//----------------------------------------------------------------------------------------
// Test splatParticlesToGrid
//----------------------------------------------------------------------------------------
// One particle at grid center.
TEST_F(ParticleGridInterp_Test, splatParticlesToGrid_test1) {
	vector<vec2> positions = { vec2(0.5f,0.5f) };

	vector<float32> attributes = { 1.0f };

	splatParticlesToGrid(
			grid2x2, weights2x2, positions, attributes, InterpKernel::linear, kCellLength
	);

	EXPECT_NEAR(grid2x2(0,0), 1.0f, epsilon);
	EXPECT_NEAR(grid2x2(1,0), 1.0f, epsilon);
	EXPECT_NEAR(grid2x2(0,1), 1.0f, epsilon);
	EXPECT_NEAR(grid2x2(1,1), 1.0f, epsilon);
}

//----------------------------------------------------------------------------------------
// One particle at bottom middle of grid.
TEST_F(ParticleGridInterp_Test, splatParticlesToGrid_test2) {
	vector<vec2> positions = { vec2(0.5f, 0.0f) };

	vector<float32> attributes = { 1.0f };

	splatParticlesToGrid(
			grid2x2, weights2x2, positions, attributes, InterpKernel::linear, kCellLength
	);

	EXPECT_NEAR(grid2x2(0,0), 1.0f, epsilon);
	EXPECT_NEAR(grid2x2(1,0), 1.0f, epsilon);
	EXPECT_NEAR(grid2x2(0,1), 0.0f, epsilon);
	EXPECT_NEAR(grid2x2(1,1), 0.0f, epsilon);
}

//----------------------------------------------------------------------------------------
// One particle at top middle of grid.
TEST_F(ParticleGridInterp_Test, splatParticlesToGrid_test3) {
	vector<vec2> positions = { vec2(0.5f, 1.0f - epsilon) };

	vector<float32> attributes = { 1.0f };

	splatParticlesToGrid(
			grid2x2, weights2x2, positions, attributes, InterpKernel::linear, kCellLength
	);

	EXPECT_NEAR(grid2x2(0,0), 0.0f, epsilon);
	EXPECT_NEAR(grid2x2(1,0), 0.0f, epsilon);
	EXPECT_NEAR(grid2x2(0,1), 1.0f, epsilon);
	EXPECT_NEAR(grid2x2(1,1), 1.0f, epsilon);
}

//----------------------------------------------------------------------------------------
// One particle at left middle of grid.
TEST_F(ParticleGridInterp_Test, splatParticlesToGrid_test4) {
	vector<vec2> positions = { vec2(0.0f, 0.5f) };

	vector<float32> attributes = { 1.0f };

	splatParticlesToGrid(
			grid2x2, weights2x2, positions, attributes, InterpKernel::linear, kCellLength
	);

	EXPECT_NEAR(grid2x2(0,0), 1.0f, epsilon);
	EXPECT_NEAR(grid2x2(1,0), 0.0f, epsilon);
	EXPECT_NEAR(grid2x2(0,1), 1.0f, epsilon);
	EXPECT_NEAR(grid2x2(1,1), 0.0f, epsilon);
}

//----------------------------------------------------------------------------------------
// One particle at right middle of grid.
TEST_F(ParticleGridInterp_Test, splatParticlesToGrid_test5) {
	vector<vec2> positions = { vec2(1.0f - epsilon, 0.5f) };

	vector<float32> attributes = { 1.0f };

	splatParticlesToGrid(
			grid2x2, weights2x2, positions, attributes, InterpKernel::linear, kCellLength
	);

	EXPECT_NEAR(grid2x2(0,0), 0.0f, epsilon);
	EXPECT_NEAR(grid2x2(1,0), 1.0f, epsilon);
	EXPECT_NEAR(grid2x2(0,1), 0.0f, epsilon);
	EXPECT_NEAR(grid2x2(1,1), 1.0f, epsilon);
}

//----------------------------------------------------------------------------------------
// Two particles with different values.
TEST_F(ParticleGridInterp_Test, splatParticlesToGrid_test6) {
	vector<vec2> positions = {
			vec2(0.25f, 0.5f), // particle 1
			vec2(0.75f, 0.5f), // particle 2
	};

	vector<float32> attributes = {
			1.0f,  // particle 1
			2.0f   // particle 2
	};

	splatParticlesToGrid(
			grid2x2, weights2x2, positions, attributes, InterpKernel::linear, kCellLength
	);

	EXPECT_NEAR(grid2x2(0,0), 1.25f, epsilon);
	EXPECT_NEAR(grid2x2(1,0), 1.75f, epsilon);
	EXPECT_NEAR(grid2x2(0,1), 1.25f, epsilon);
	EXPECT_NEAR(grid2x2(1,1), 1.75f, epsilon);
}

//----------------------------------------------------------------------------------------
// Two particles, one at each grid cell center of non-symmetric grid.
TEST_F(ParticleGridInterp_Test, splatParticlesToGrid_test7) {
	vector<vec2> positions = {
			vec2(0.5f, 0.5f),  // particle 1
			vec2(1.5f, 0.5f)   // particle 2
	};

	vector<float32> attributes = {
			1.0f,  // particle 1
			2.0f   // particle 2
	};

	splatParticlesToGrid (
			grid3x2, weights3x2, positions, attributes, InterpKernel::linear, kCellLength
	);

	EXPECT_NEAR(grid3x2(0,0), 1.0f, epsilon);
	EXPECT_NEAR(grid3x2(1,0), 1.5f, epsilon);
	EXPECT_NEAR(grid3x2(2,0), 2.0f, epsilon);

	EXPECT_NEAR(grid3x2(0,1), 1.0f, epsilon);
	EXPECT_NEAR(grid3x2(1,1), 1.5f, epsilon);
	EXPECT_NEAR(grid3x2(2,1), 2.0f, epsilon);
}

//----------------------------------------------------------------------------------------
// Two particles, grid4x3
TEST_F(ParticleGridInterp_Test, splatParticlesToGrid_test8) {
	vector<vec2> positions = {
			vec2(0.5f, 0.5f),  // particle 1
			vec2(1.5f, 1.5f)   // particle 2
	};

	vector<float32> attributes = {
			-1.0f,  // particle 1
			 1.0f   // particle 2
	};

	// Put garbage in grid.
	// All grid values should be overwritten even for cells not affected by particles.
	grid4x3.setAll(99.9f);

	splatParticlesToGrid(
			grid4x3, weights4x3, positions, attributes, InterpKernel::linear, kCellLength
	);

	EXPECT_NEAR(grid4x3(0,0), -1.0f, epsilon);
	EXPECT_NEAR(grid4x3(1,0), -1.0f, epsilon);
	EXPECT_NEAR(grid4x3(2,0), 0.0f, epsilon);
	EXPECT_NEAR(grid4x3(3,0), 0.0f, epsilon);

	EXPECT_NEAR(grid4x3(0,1), -1.0f, epsilon);
	EXPECT_NEAR(grid4x3(1,1), 0.0f, epsilon);
	EXPECT_NEAR(grid4x3(2,1), 1.0f, epsilon);
	EXPECT_NEAR(grid4x3(3,1), 0.0f, epsilon);

	EXPECT_NEAR(grid4x3(0,2), 0.0f, epsilon);
	EXPECT_NEAR(grid4x3(1,2), 1.0f, epsilon);
	EXPECT_NEAR(grid4x3(2,2), 1.0f, epsilon);
	EXPECT_NEAR(grid4x3(3,2), 0.0f, epsilon);
}




//----------------------------------------------------------------------------------------
// Test interpGridToParticles
//----------------------------------------------------------------------------------------
TEST_F(ParticleGridInterp_Test, interpGridToParticles_test1) {
	grid2x2(0,0) = 1.0f;
	grid2x2(1,0) = 1.0f;
	grid2x2(0,1) = 2.0f;
	grid2x2(1,1) = 2.0f;

	vector<vec2> positions = {
			vec2(0.5f, 0.5f)*kCellLength
	};

	vector<float32> attributes;
	attributes.reserve(positions.size());

	interpGridToParticles<float32> (
			attributes,
			positions,
			grid2x2,
			GridInterp::linear<float32>
	);

	EXPECT_NEAR(attributes[0], 1.5f, epsilon);
}

//---------------------------------------------------------------------------------------
TEST_F(ParticleGridInterp_Test, interpGridToParticles_test2) {
	grid2x2(0,0) = -1.0f;
	grid2x2(1,0) = 1.0f;
	grid2x2(0,1) = 1.0f;
	grid2x2(1,1) = -1.0f;

	vector<vec2> positions = {
			vec2(0.5f, 0.5f)*kCellLength
	};

	vector<float32> attributes;
	attributes.reserve(positions.size());

	interpGridToParticles<float32> (
			attributes,
			positions,
			grid2x2,
			GridInterp::linear<float32>
	);

	EXPECT_NEAR(attributes[0], 0.0f, epsilon);
}

//---------------------------------------------------------------------------------------
TEST_F(ParticleGridInterp_Test, interpGridToParticles_test3) {
	grid2x2(0,0) = 1.0f;
	grid2x2(1,0) = 2.0f;
	grid2x2(0,1) = 3.0f;
	grid2x2(1,1) = 4.0f;

	vector<vec2> positions = {
			vec2(0.5f, 0.0f)*kCellLength,
			vec2(0.5f, 1.0f - half_eps)*kCellLength,
	};

	vector<float32> attributes;
	attributes.reserve(positions.size());

	interpGridToParticles<float32> (
			attributes,
			positions,
			grid2x2,
			GridInterp::linear<float32>
	);

	EXPECT_NEAR(attributes[0], 1.5f, epsilon);
	EXPECT_NEAR(attributes[1], 3.5f, epsilon);
}

//---------------------------------------------------------------------------------------
TEST_F(ParticleGridInterp_Test, interpGridToParticles_test4) {
	grid2x2(0,0) = 1.0f;
	grid2x2(1,0) = 2.0f;
	grid2x2(0,1) = 3.0f;
	grid2x2(1,1) = 4.0f;

	vector<vec2> positions = {
			vec2(0.0f, 0.5f)*kCellLength,
			vec2(1.0f - half_eps, 0.5f)*kCellLength,
	};

	vector<float32> attributes;
	attributes.reserve(positions.size());

	interpGridToParticles<float32> (
			attributes,
			positions,
			grid2x2,
			GridInterp::linear<float32>
	);

	EXPECT_NEAR(attributes[0], 2.0f, epsilon);
	EXPECT_NEAR(attributes[1], 3.0f, epsilon);
}

//---------------------------------------------------------------------------------------
TEST_F(ParticleGridInterp_Test, interpGridToParticles_test5) {
	grid3x2(0,0) = -1.0f;
	grid3x2(1,0) = -1.0f;
	grid3x2(2,0) = -1.0f;

	grid3x2(0,1) = 1.0f;
	grid3x2(1,1) = 1.0f;
	grid3x2(2,1) = 1.0f;

	vector<vec2> positions = {
			vec2(0.5f, 0.5f)*kCellLength,
			vec2(1.5f, 0.5f)*kCellLength,
			vec2(2.5f, 0.5f)*kCellLength,
	};

	vector<float32> attributes;
	attributes.reserve(positions.size());

	interpGridToParticles<float32> (
			attributes,
			positions,
			grid3x2,
			GridInterp::linear<float32>
	);

	EXPECT_NEAR(attributes[0], 0.0f, epsilon);
	EXPECT_NEAR(attributes[1], 0.0f, epsilon);
	EXPECT_NEAR(attributes[2], 0.0f, epsilon);
}

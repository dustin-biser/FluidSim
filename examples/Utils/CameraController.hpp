/**
 * @brief CameraController
 */

#pragma once

#include "Synergy/Core/Settings.hpp"

// Forward Declaration
namespace Synergy {
    class Camera;
}


class CameraController {
public:
	CameraController();
	~CameraController();

	void registerCamera(Synergy::Camera * camera);

	void keyInput(int key, int action, int mods);
	void mouseButton(int button, int actions, int mods);
	void mouseScroll(double xOffSet, double yOffSet);
	void cursorPosition(double xPos, double yPos);
	void updateCamera();
	void reset();

	void setRollScaleFactor(float scaleFactor);
	void setPitchScaleFactor(float scaleFactor);
	void setYawScaleFactor(float scaleFactor);
	void setUpScaleFactor(float upScale);
	void setForwardScaleFactor(float forwardScale);
	void setSideStrafeScaleFactor(float sideStrafeScale);

private:
	void updateLookAt();
	void updateTranslation();
	void updateRoll();
	void rotateYawAxis(float angle, glm::vec3 axis);

private:
	Synergy::Camera * camera;
	glm::vec3 yawAxis;

	// Screen Coordinates
	double xCursorPos_prev;
	double yCursorPos_prev;
	double xCursorPos;
	double yCursorPos;

	// Input Key States
	bool key_r_down = false;
	bool key_f_down = false;
	bool key_q_down = false;
	bool key_e_down = false;
	bool key_w_down = false;
	bool key_s_down = false;
	bool key_a_down = false;
	bool key_d_down = false;
	bool key_left_shift_down = false;

	struct TransformScaleFactors {
		float rollAngle;
		float pitchAngle;
		float yawAngle;
		float forwardDelta;
		float sideStrafeDelta;
		float upDelta;
	};
	TransformScaleFactors scaleFactors;

	bool flagRotation;
};


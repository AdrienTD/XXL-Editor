#pragma once

#include "vecmat.h"

struct Camera {
	Vector3 position, orientation;
	float aspect;
	Matrix sceneMatrix, viewMatrix, projMatrix;
	Vector3 direction;
	void updateMatrix();
};
#pragma once

#include "vecmat.h"

struct Camera {
	Vector3 position, orientation;
	float aspect, nearDist = 0.1f, farDist = 1000.0f;
	bool orthoMode = false;
	Matrix sceneMatrix, viewMatrix, projMatrix;
	Vector3 direction;
	void updateMatrix();
};
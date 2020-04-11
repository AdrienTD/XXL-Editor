#include "Camera.h"
#include "window.h"

void Camera::updateMatrix() {
	this->direction = Vector3(
		-sin(orientation.y)*cos(orientation.x),
		sin(orientation.x),
		cos(orientation.y)*cos(orientation.x)
	);
	projMatrix = Matrix::getRHPerspectiveMatrix(0.9, aspect, 0.75f, 280.0f);
	viewMatrix = Matrix::getRHLookAtViewMatrix(position, position + direction, Vector3(0, 1, 0));
	this->sceneMatrix = viewMatrix * projMatrix;
}
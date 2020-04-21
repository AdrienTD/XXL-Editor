#include "Camera.h"
#include "window.h"

void Camera::updateMatrix() {
	this->direction = Vector3(
		-sin(orientation.y)*cos(orientation.x),
		sin(orientation.x),
		cos(orientation.y)*cos(orientation.x)
	);
	if (orthoMode)
		projMatrix = Matrix::getRHOrthoMatrix(position.y*aspect, position.y, nearDist, farDist);
	else
		projMatrix = Matrix::getRHPerspectiveMatrix(0.9, aspect, nearDist, farDist);
	viewMatrix = Matrix::getRHLookAtViewMatrix(position, position + direction, Vector3(0, 1, 0));
	this->sceneMatrix = viewMatrix * projMatrix;
}
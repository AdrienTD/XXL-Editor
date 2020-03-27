#include "Camera.h"
#include "window.h"

void Camera::updateMatrix() {
	Matrix pers, cammat;
	this->direction = Vector3(
		-sin(orientation.y)*cos(orientation.x),
		sin(orientation.x),
		cos(orientation.y)*cos(orientation.x)
	);
	pers = Matrix::getRHPerspectiveMatrix(0.9, aspect, 1.0f, 250.0f);
	cammat = Matrix::getRHLookAtViewMatrix(position, position + direction, Vector3(0, 1, 0));
	this->sceneMatrix = cammat * pers;
}
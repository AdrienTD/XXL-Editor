#include "vecmat.h"

Matrix Matrix::getTranslationMatrix(const Vector3 & translation)
{
	Matrix m = getIdentity();
	m._41 = translation.x;
	m._42 = translation.y;
	m._43 = translation.z;
	return m;
}

Matrix Matrix::getRHPerspectiveMatrix(float fovy, float aspect, float zn, float zf)
{
	float ys = 1 / tan(fovy / 2);
	float xs = ys / aspect;
	Matrix m = getZeroMatrix();
	m.m[0][0] = xs;
	m.m[1][1] = ys;
	m.m[2][2] = zf / (zn - zf);
	m.m[2][3] = -1;
	m.m[3][2] = zn * zf / (zn - zf);
	return m;
}

Matrix Matrix::getRHLookAtViewMatrix(const Vector3 & eye, const Vector3 & at, const Vector3 & up)
{
	Vector3 ax, ay, az;
	az = (eye - at).normal();
	ax = up.cross(az).normal();
	ay = az.cross(ax);

	Matrix m = getZeroMatrix();
	m.m[0][0] = ax.x; m.m[0][1] = ay.x; m.m[0][2] = az.x;
	m.m[1][0] = ax.y; m.m[1][1] = ay.y; m.m[1][2] = az.y;
	m.m[2][0] = ax.z; m.m[2][1] = ay.z; m.m[2][2] = az.z;
	m.m[3][0] = -ax.dot(eye);
	m.m[3][1] = -ay.dot(eye);
	m.m[3][2] = -az.dot(eye);
	m.m[3][3] = 1.0f;
	return m;
}

Matrix Matrix::multiplyMatrices(const Matrix & a, const Matrix & b)
{
	Matrix m;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			m.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j]
				+ a.m[i][2] * b.m[2][j] + a.m[i][3] * b.m[3][j];
		}
	}
	return m;
}

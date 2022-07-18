#include "vecmat.h"
#include <array>

template<int N, typename TNum>
constexpr std::enable_if_t<N >= 1, TNum> TpMatDeterminant(std::array<TNum, N* N> m) {
	if constexpr (N == 1) {
		return m[0];
	}
	else {
		TNum rs = 0;
		for (int i = 0; i < N; ++i) {
			std::array<TNum, (N - 1)* (N - 1)> sub;
			for (int a = 0; a < N - 1; ++a)
				for (int b = 0; b < N - 1; ++b)
					sub[a * (N - 1) + b] = m[(a + 1) * N + ((b < i) ? b : (b + 1))];
			TNum sign = (TNum)((i & 1) ? -1 : 1);
			rs += sign * m[i] * TpMatDeterminant<N - 1>(sub);
		}
		return rs;
	}
}

template<int N, typename TNum>
constexpr std::enable_if_t<N >= 1, std::array<TNum, N* N>> TpMatInverse(std::array<TNum, N* N> m) {
	if constexpr (N == 1) {
		return { (TNum)1 / m[0] };
	}
	else {
		std::array<TNum, N* N> result;
		float maindet = TpMatDeterminant<N>(m);
		for (int i = 0; i < N; ++i) {
			for (int j = 0; j < N; ++j) {
				std::array<TNum, (N - 1)* (N - 1)> sub;
				for (int a = 0; a < N - 1; ++a) {
					for (int b = 0; b < N - 1; ++b) {
						sub[a * (N - 1) + b] = m[((a < i) ? a : (a + 1)) * N + ((b < j) ? b : (b + 1))];
					}
				}
				float subdet = TpMatDeterminant<N - 1>(sub);
				TNum sign = (TNum)(((i + j) & 1) ? -1 : 1);
				result[j * N + i] = (sign * subdet) / maindet;
			}
		}
		return result;
	}
}

Matrix Matrix::getTranslationMatrix(const Vector3 & translation)
{
	Matrix m = getIdentity();
	m._41 = translation.x;
	m._42 = translation.y;
	m._43 = translation.z;
	return m;
}

Matrix Matrix::getRotationXMatrix(float radians)
{
	Matrix m = getIdentity();
	m.m[1][1] = m.m[2][2] = cos(radians);
	m.m[1][2] = sin(radians);
	m.m[2][1] = -m.m[1][2];
	return m;
}

Matrix Matrix::getRotationYMatrix(float radians)
{
	Matrix m = getIdentity();
	m.m[0][0] = m.m[2][2] = cos(radians);
	m.m[2][0] = sin(radians);
	m.m[0][2] = -m.m[2][0];
	return m;
}

Matrix Matrix::getRotationZMatrix(float radians)
{
	Matrix m = getIdentity();
	m.m[0][0] = m.m[1][1] = cos(radians);
	m.m[0][1] = sin(radians);
	m.m[1][0] = -m.m[0][1];
	return m;
}

Matrix Matrix::getScaleMatrix(const Vector3 & scale)
{
	Matrix m = getZeroMatrix();
	m._11 = scale.x;
	m._22 = scale.y;
	m._33 = scale.z;
	m._44 = 1.0f;
	return m;
}

Matrix Matrix::getRHOrthoMatrix(float w, float h, float zn, float zf)
{
	Matrix m = getZeroMatrix();
	m._11 = 2.0f / w;
	m._22 = 2.0f / h;
	m._33 = 1.0f / (zn - zf);
	m._43 = zn / (zn - zf);
	m._44 = 1.0f;
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

Vector3 Matrix::getTranslationVector() const
{
	return Vector3(_41, _42, _43);
}

Vector3 Matrix::getScalingVector() const
{
	// NOTE: Result is positive, what about negative scaling?
	Vector3 rvec;
	for (int i = 0; i < 3; i++) {
		Vector3 axis{ m[0][i], m[1][i], m[2][i] };
		rvec.coord[i] = axis.len3();
	}
	return rvec;
}

Matrix Matrix::getInverse4x3() const
{
	Matrix inv = Matrix::getIdentity();
	for (int i = 0; i < 3; i++) {
		int i_1 = (i + 1) % 3, i_2 = (i + 2) % 3;
		for (int j = 0; j < 3; j++) {
			int j_1 = (j + 1) % 3, j_2 = (j + 2) % 3;
			inv.m[j][i] = m[i_1][j_1] * m[i_2][j_2] - m[i_1][j_2] * m[i_2][j_1];
		}
	}
	return Matrix::getTranslationMatrix(-getTranslationVector()) * inv;
}

Matrix Matrix::getInverse4x4() const
{
	Matrix m;
	std::array<float, 16> arr;
	std::copy(std::begin(v), std::end(v), std::begin(arr));
	std::array<float, 16> res = TpMatInverse<4>(arr);
	std::copy(std::begin(res), std::end(res), std::begin(m.v));
	return m;
}

void Matrix::setTranslation(const Vector3& translation)
{
	_41 = translation.x; _42 = translation.y; _43 = translation.z;
}

Vector3 Vector3::transform(const Matrix & m) const
{
	const Vector3 &a = *this;
	Vector3 v;
	v.x = a.x * m.m[0][0] + a.y * m.m[1][0] + a.z * m.m[2][0] + m.m[3][0];
	v.y = a.x * m.m[0][1] + a.y * m.m[1][1] + a.z * m.m[2][1] + m.m[3][1];
	v.z = a.x * m.m[0][2] + a.y * m.m[1][2] + a.z * m.m[2][2] + m.m[3][2];
	return v;
}

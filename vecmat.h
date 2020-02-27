// wkbre - WK (Battles) recreated game engine
// Copyright (C) 2015-2016 Adrien Geets
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <cmath>

struct Matrix;
struct Vector3;

// Matrix creation
void CreateIdentityMatrix(Matrix *m);
void CreateZeroMatrix(Matrix *m);
void CreateTranslationMatrix(Matrix *m, float x, float y, float z);
void CreateScaleMatrix(Matrix *m, float x, float y, float z);
void CreateRotationXMatrix(Matrix *m, float a);
void CreateRotationYMatrix(Matrix *m, float a);
void CreateRotationZMatrix(Matrix *m, float a);
void CreatePerspectiveMatrix(Matrix *m, float fovy, float aspect, float zn, float zf);
void CreateLookAtLHViewMatrix(Matrix *m, const Vector3 *eye, const Vector3 *at, const Vector3 *up);
void CreateWorldMatrix(Matrix *mWorld, const Vector3 &is, const Vector3 &ir, const Vector3 &it);
void CreateRotationYXZMatrix(Matrix *m, float y, float x, float z);

// Matrix operations
void MultiplyMatrices(Matrix *m, const Matrix *a, const Matrix *b);
void TransposeMatrix(Matrix *m, const Matrix *a);
//void InverseMatrix(Matrix *o, const Matrix *i);

// Vector operations
void TransformVector3(Vector3 *v, const Vector3 *a, const Matrix *m);
void TransformNormal3(Vector3 *v, const Vector3 *a, const Matrix *m);
void TransformCoord3(Vector3 *r, const Vector3 *v, const Matrix *m);
void TransformBackFromViewMatrix(Vector3 *r, const Vector3 *o, const Matrix *m);
void NormalizeVector3(Vector3 *o, const Vector3 *i);
//bool SphereIntersectsRay(const Vector3 *sphPos, float radius, const Vector3 *raystart, const Vector3 *raydir);
bool RayIntersectsSphere(const Vector3 &raystart, const Vector3 &raydir, const Vector3 &center, float radius);

// 4x4 matrix structure
struct alignas(16) Matrix
{
	union {
		float v[16];
		float m[4][4];
		struct {
			float _11, _12, _13, _14, _21, _22, _23, _24, 
				_31, _32, _33, _34, _41, _42, _43, _44;
		};
	};
	Matrix operator*(const Matrix &a)
	{
		Matrix m;
		MultiplyMatrices(&m, this, &a);
		return m;
	}
	void operator*=(const Matrix &a)
	{
		Matrix m = *this;
		MultiplyMatrices(this, &m, &a);
	}
};

// Three-dimensional vector
struct alignas(16) Vector3
{
	float x, y, z, w; // w used to align size.
	Vector3() {x = y = z = 0;}
	Vector3(float a, float b) {x = a; y = b; z = 0;}
	Vector3(float a, float b, float c) {x = a; y = b; z = c;}

	Vector3 operator+(const Vector3 &a) const {return Vector3(x + a.x, y + a.y, z + a.z);}
	Vector3 operator-(const Vector3 &a) const {return Vector3(x - a.x, y - a.y, z - a.z);}
	Vector3 operator*(const Vector3 &a) const {return Vector3(x * a.x, y * a.y, z * a.z);}
	Vector3 operator/(const Vector3 &a) const {return Vector3(x / a.x, y / a.y, z / a.z);}
	Vector3 operator+(float a) const {return Vector3(x + a, y + a, z + a);}
	Vector3 operator-(float a) const {return Vector3(x - a, y - a, z - a);}
	Vector3 operator*(float a) const {return Vector3(x * a, y * a, z * a);}
	Vector3 operator/(float a) const {return Vector3(x / a, y / a, z / a);}
	Vector3 operator-() const {return Vector3(-x, -y, -z);}

	Vector3 &operator+=(const Vector3 &a) {x += a.x; y += a.y; z += a.z; return *this;}
	Vector3 &operator-=(const Vector3 &a) {x -= a.x; y -= a.y; z -= a.z; return *this;}
	Vector3 &operator*=(const Vector3 &a) {x *= a.x; y *= a.y; z *= a.z; return *this;}
	Vector3 &operator/=(const Vector3 &a) {x /= a.x; y /= a.y; z /= a.z; return *this;}
	Vector3 &operator+=(float a) {x += a; y += a; z += a; return *this;}
	Vector3 &operator-=(float a) {x -= a; y -= a; z -= a; return *this;}
	Vector3 &operator*=(float a) {x *= a; y *= a; z *= a; return *this;}
	Vector3 &operator/=(float a) {x /= a; y /= a; z /= a; return *this;}

	bool operator==(const Vector3 &a) const {return (x==a.x) && (y==a.y) && (z==a.z);}
	bool operator!=(const Vector3 &a) const {return !( (x==a.x) && (y==a.y) && (z==a.z) );}

	//void print() const {printf("(%f, %f, %f)\n", x, y, z);}
	float len2xy() const {return sqrt(x*x + y*y);}
	float sqlen2xy() const {return x*x + y*y;}
	float len2xz() const {return sqrt(x*x + z*z);}
	float sqlen2xz() const {return x*x + z*z;}
	float len3() const {return sqrt(x*x + y*y + z*z);}
	float sqlen3() const {return x*x + y*y + z*z;}
	Vector3 normal() const {float l = len3(); if(l != 0.0f) return Vector3(x/l, y/l, z/l); else return Vector3(0,0,0);}
	Vector3 normal2xz() const {float l = len2xz(); if(l != 0.0f) return Vector3(x/l, 0, z/l); else return Vector3(0,0,0);}
	float dot(const Vector3 &a) const {return a.x * x + a.y * y + a.z * z;}
	float dot2xz(const Vector3 &a) const {return a.x * x + a.z * z;}
	Vector3 cross(Vector3 &v) const {return Vector3(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);}
};

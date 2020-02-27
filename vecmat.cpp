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

#include "vecmat.h"

#define determinant33(a, b, c, d, e, f, g, h, i) ((a)*(e)*(i) + (b)*(f)*(g) + (c)*(d)*(h) - (g)*(e)*(c) - (h)*(f)*(a) - (i)*(d)*(b))

void Vec3Cross(Vector3 *r, const Vector3 *a, const Vector3 *b)
{
	r->x = a->y * b->z - a->z * b->y;
	r->y = a->z * b->x - a->x * b->z;
	r->z = a->x * b->y - a->y * b->x;
}

float Vec3Dot(const Vector3 *a, const Vector3 *b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

void NormalizeVector3(Vector3 *o, const Vector3 *i)
{
	float l = sqrt(i->x*i->x + i->y*i->y + i->z*i->z);
	o->x = i->x / l;
	o->y = i->y / l;
	o->z = i->z / l;
}

void CreateIdentityMatrix(Matrix *m)
{
	for(int i = 0; i < 4; i++)
	for(int j = 0; j < 4; j++)
		m->m[i][j] = (i==j) ? 1.0f : 0.0f;
}

void CreateZeroMatrix(Matrix *m)
{
	for (int i = 0; i < 16; i++)
		m->v[i] = 0.0f;
}

void CreateTranslationMatrix(Matrix *m, float x, float y, float z)
{
	CreateIdentityMatrix(m);
	m->m[3][0] = x;
	m->m[3][1] = y;
	m->m[3][2] = z;
}

void CreateScaleMatrix(Matrix *m, float x, float y, float z)
{
	CreateIdentityMatrix(m);
	m->m[0][0] = x;
	m->m[1][1] = y;
	m->m[2][2] = z;
}

void CreateRotationXMatrix(Matrix *m, float a)
{
	CreateIdentityMatrix(m);
	m->m[1][1] = m->m[2][2] = cos(a);
	m->m[1][2] = sin(a);
	m->m[2][1] = -m->m[1][2];
}

void CreateRotationYMatrix(Matrix *m, float a)
{
	CreateIdentityMatrix(m);
	m->m[0][0] = m->m[2][2] = cos(a);
	m->m[2][0] = sin(a);
	m->m[0][2] = -m->m[2][0];
}

void CreateRotationZMatrix(Matrix *m, float a)
{
	CreateIdentityMatrix(m);
	m->m[0][0] = m->m[1][1] = cos(a);
	m->m[0][1] = sin(a);
	m->m[1][0] = -m->m[0][1];
}

void MultiplyMatrices(Matrix *m, const Matrix *a, const Matrix *b)
{
	for(int i = 0; i < 4; i++)
	for(int j = 0; j < 4; j++)
	{
		m->m[i][j] = a->m[i][0] * b->m[0][j] + a->m[i][1] * b->m[1][j]
				+ a->m[i][2] * b->m[2][j] + a->m[i][3] * b->m[3][j];
	}
}

void TransposeMatrix(Matrix *m, const Matrix *a)
{
	for(int i = 0; i < 4; i++)
	for(int j = 0; j < 4; j++)
		m->m[i][j] = a->m[j][i];
}

void TransformVector3(Vector3 *v, const Vector3 *a, const Matrix *m)
{
	v->x = a->x * m->m[0][0] + a->y * m->m[1][0]
		 + a->z * m->m[2][0] + m->m[3][0];
	v->y = a->x * m->m[0][1] + a->y * m->m[1][1]
		 + a->z * m->m[2][1] + m->m[3][1];
	v->z = a->x * m->m[0][2] + a->y * m->m[1][2]
		 + a->z * m->m[2][2] + m->m[3][2];
}

/*void TransformVector3to4(D3DXVECTOR4 *v, const Vector3 *a, const Matrix *m)
{
	v->x = a->x * m->m[0][0] + a->y * m->m[1][0]
		 + a->z * m->m[2][0] + m->m[3][0];
	v->y = a->x * m->m[0][1] + a->y * m->m[1][1]
		 + a->z * m->m[2][1] + m->m[3][1];
	v->z = a->x * m->m[0][2] + a->y * m->m[1][2]
		 + a->z * m->m[2][2] + m->m[3][2];
	v->w = a->x * m->m[0][3] + a->y * m->m[1][3]
		 + a->z * m->m[2][3] + m->m[3][3];
}*/

void TransformNormal3(Vector3 *v, const Vector3 *a, const Matrix *m)
{
	v->x = a->x * m->m[0][0] + a->y * m->m[1][0]
		 + a->z * m->m[2][0];
	v->y = a->x * m->m[0][1] + a->y * m->m[1][1]
		 + a->z * m->m[2][1];
	v->z = a->x * m->m[0][2] + a->y * m->m[1][2]
		 + a->z * m->m[2][2];
}

void CreatePerspectiveMatrix(Matrix *m, float fovy, float aspect, float zn, float zf)
{
	float ys = 1 / tan(fovy/2);
	float xs = ys / aspect;
	//float xs = 1 / tan(fovy / 2);
	//float ys = xs * aspect;
	CreateZeroMatrix(m);
	m->m[0][0] = xs;
	m->m[1][1] = ys;
	m->m[2][2] = zf / (zf-zn);
	m->m[2][3] = 1;
	m->m[3][2] = -zn*zf / (zf-zn);
}

void CreateLookAtLHViewMatrix(Matrix *m, const Vector3 *eye, const Vector3 *at, const Vector3 *up)
{
	Vector3 ax, ay, az, t;
	NormalizeVector3(&az, &(*at - *eye));
	//ax = (*up).cross(az).normal();
	Vec3Cross(&t, up, &az);
	NormalizeVector3(&ax, &t);
	//ay = az.cross(ax);
	Vec3Cross(&ay, &az, &ax);

	CreateIdentityMatrix(m);
	m->m[0][0] = ax.x; m->m[0][1] = ay.x; m->m[0][2] = az.x;
	m->m[1][0] = ax.y; m->m[1][1] = ay.y; m->m[1][2] = az.y;
	m->m[2][0] = ax.z; m->m[2][1] = ay.z; m->m[2][2] = az.z;
	m->m[3][0] = -Vec3Dot(&ax, eye); // -ax.dot(*eye);
	m->m[3][1] = -Vec3Dot(&ay, eye); // -ay.dot(*eye);
	m->m[3][2] = -Vec3Dot(&az, eye); // -az.dot(*eye);
}

void CreateRotationYXZMatrix(Matrix *m, float y, float x, float z)
{
	Matrix a, b, c;
	CreateRotationYMatrix(&a, y);
	CreateRotationXMatrix(&b, x);
	MultiplyMatrices(&c, &a, &b);
	CreateRotationZMatrix(&a, z);
	MultiplyMatrices(m, &c, &a);
}

void TransformCoord3(Vector3 *r, const Vector3 *v, const Matrix *m)
	{float w = v->x * m->_14 + v->y * m->_24 + v->z * m->_34 + m->_44;
	r->x = (v->x * m->_11 + v->y * m->_21 + v->z * m->_31 + m->_41) / w;
	r->y = (v->x * m->_12 + v->y * m->_22 + v->z * m->_32 + m->_42) / w;
	r->z = (v->x * m->_13 + v->y * m->_23 + v->z * m->_33 + m->_43) / w;}

// for LineIntersectsCircle, see wkbre21
bool LineIntersectsSquareRad(float cx, float cy, float r, float sx, float sy, float dx, float dy)
{
	//if(dx == 0.0f) dx = 0.0001f;
	//if(dy == 0.0f) dy = 0.0001f;
	if(dx == 0.0f) if(abs(sy-cy) < r) return true;
	if(dy == 0.0f) if(abs(sx-cx) < r) return true;
	float p = sy - dy * sx / dx;
	float t;
	t = dy * (cx+r) / dx + p;
	if(abs(t-cy) < r) return true;
	t = dy * (cx-r) / dx + p;
	if(abs(t-cy) < r) return true;
	t = dx * (cy+r - p) / dy;
	if(abs(t-cx) < r) return true;
	t = dx * (cy-r - p) / dy;
	if(abs(t-cx) < r) return true;
	return false;
}
//bool SphereIntersectsRay(const Vector3 *sphpos, float r, const Vector3 *raystart, const Vector3 *raydir)
//{
//	if(LineIntersectsSquareRad(sphpos->x, sphpos->z, r, raystart->x, raystart->z, raydir->x, raydir->z))
//	if(LineIntersectsSquareRad(sphpos->x, sphpos->y, r, raystart->x, raystart->y, raydir->x, raydir->y))
//	if(LineIntersectsSquareRad(sphpos->y, sphpos->z, r, raystart->y, raystart->z, raydir->y, raydir->z))
//		return true;
//	return false;
//}

bool RayIntersectsSphere(const Vector3 &raystart, const Vector3 &raydir, const Vector3 &center, float radius)
{
	//float a, b, c;
	//Vector3 smc = raystart - center;
	//c = smc.sqlen3() - radius * radius;
	//b = 2 * smc.dot(raydir);
	//a = raydir.sqlen3();
	//float d = b * b - 4 * a*c;
	//return b >= 0;
	Vector3 rdnorm = raydir.normal();
	Vector3 smc = raystart - center;
	float ddt = rdnorm.dot(smc);
	float delta = ddt * ddt - rdnorm.sqlen3()*(smc.sqlen3() - radius * radius);
	if (delta < 0.0f)
		return false;
	float k1 = -ddt + sqrtf(delta),
		  k2 = -ddt - sqrtf(delta);
	if (k1 < 0.0f && k2 < 0.0f)
		return false;
	return true;
}

void TransformBackFromViewMatrix(Vector3 *r, const Vector3 *o, const Matrix *m)
{
	Vector3 xa, ya, za;
	xa.x = m->_11; ya.x = m->_12; za.x = m->_13;
	xa.y = m->_21; ya.y = m->_22; za.y = m->_23;
	xa.z = m->_31; ya.z = m->_32; za.z = m->_33;

	float ex, ey, ez;
	ex = o->x;
	ey = o->y;
	ez = o->z;

	float dt = determinant33(xa.x, xa.y, xa.z, ya.x, ya.y, ya.z, za.x, za.y, za.z);
	//if(!dt) ferr("That's not possible! The determinant is null!");

	r->x = determinant33(ex, xa.y, xa.z, ey, ya.y, ya.z, ez, za.y, za.z) / dt;
	r->y = determinant33(xa.x, ex, xa.z, ya.x, ey, ya.z, za.x, ez, za.z) / dt;
	r->z = determinant33(xa.x, xa.y, ex, ya.x, ya.y, ey, za.x, za.y, ez) / dt;
}

void CreateWorldMatrix(Matrix *mWorld, const Vector3 &is, const Vector3 &ir, const Vector3 &it)
{
	Matrix mscale, mrot, mtrans;
	CreateScaleMatrix(&mscale, is.x, is.y, is.z);
	CreateRotationYXZMatrix(&mrot, ir.y, ir.x, ir.z);
	CreateTranslationMatrix(&mtrans, it.x, it.y, it.z);
	*mWorld = mscale*mrot*mtrans;
}

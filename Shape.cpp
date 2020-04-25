#include "Shape.h"
#include "File.h"

void BoundingSphere::merge(const BoundingSphere & other)
{
	if (center == other.center) {
		if (other.radius > radius)
			radius = other.radius;
	}
	else {
		Vector3 n_t2o = (other.center - center).normal();
		Vector3 ext_t = center - n_t2o * radius;
		Vector3 ext_o = other.center + n_t2o * other.radius;
		if ((ext_o - center).len3() <= radius)
			;
		else if ((ext_t - other.center).len3() <= other.radius) {
			center = other.center;
			radius = other.radius;
		}
		else {
			center = (ext_t + ext_o) * 0.5f;
			radius = 0.5f * (ext_o - ext_t).len3();
		}
	}
}

void BoundingSphere::mergePoint(const Vector3 & point)
{
	if ((point - center).len3() <= radius)
		return;
	Vector3 c2p = (point - center).normal();
	Vector3 ext = center - c2p * radius;
	center = (ext + point) * 0.5f;
	radius = (point - center).len3();
}

void BoundingSphere::deserialize(File * file, bool withSquare)
{
	for (float &c : center)
		c = file->readFloat();
	radius = file->readFloat();
	if (withSquare)
		file->readFloat();
}

void BoundingSphere::serialize(File * file, bool withSquare)
{
	for (float &c : center)
		file->writeFloat(c);
	file->writeFloat(radius);
	if (withSquare)
		file->writeFloat(radius*radius);
}

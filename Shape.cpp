#include "Shape.h"
#include "File.h"
#include <algorithm>

bool BoundingSphere::containsPoint(const Vector3& point) const
{
	return (point - center).len3() <= radius;
}

bool BoundingSphere::intersectsWithSphere(const BoundingSphere& other) const
{
	return (other.center - center).len3() <= radius + other.radius;
}

float BoundingSphere::distanceToPoint(const Vector3& point) const
{
	return std::max(0.0f, (point - center).len3() - radius);
}

float BoundingSphere::distanceToSphere(const BoundingSphere& other) const
{
	return std::max(0.0f, (other.center - center).len3() - radius - other.radius);
}

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

void AABoundingBox::mergePoint(const Vector3 & point)
{
	if (point.x > highCorner.x) highCorner.x = point.x;
	if (point.y > highCorner.y) highCorner.y = point.y;
	if (point.z > highCorner.z) highCorner.z = point.z;
	if (point.x < lowCorner.x) lowCorner.x = point.x;
	if (point.y < lowCorner.y) lowCorner.y = point.y;
	if (point.z < lowCorner.z) lowCorner.z = point.z;
}

void AABoundingBox::merge(const AABoundingBox & box)
{
	mergePoint(box.highCorner);
	mergePoint(box.lowCorner);
}

bool AABoundingBox::containsPoint(const Vector3& point) const
{
	return point.x >= lowCorner.x && point.x <= highCorner.x &&
		point.y >= lowCorner.y && point.y <= highCorner.y &&
		point.z >= lowCorner.z && point.z <= highCorner.z;
}

std::optional<AABoundingBox> AABoundingBox::intersectionWith(const AABoundingBox& box) const
{
	AABoundingBox intersection;
	for (int i = 0; i < 3; ++i) {
		intersection.lowCorner.coord[i] = std::max(lowCorner.coord[i], box.lowCorner.coord[i]);
		intersection.highCorner.coord[i] = std::min(highCorner.coord[i], box.highCorner.coord[i]);
		if (intersection.lowCorner.coord[i] >= intersection.highCorner.coord[i]) {
			return std::nullopt;
		}
	}
	return intersection;
}

void AABoundingBox::deserialize(File * file)
{
	for (float &f : highCorner)
		f = file->readFloat();
	for (float &f : lowCorner)
		f = file->readFloat();
}

void AABoundingBox::serialize(File * file)
{
	for (float &f : highCorner)
		file->writeFloat(f);
	for (float &f : lowCorner)
		file->writeFloat(f);
}

void AABoundingBox::deserializeLC(File* file)
{
	for (float& f : lowCorner)
		f = file->readFloat();
	for (float& f : highCorner)
		f = file->readFloat();
}

void AABoundingBox::serializeLC(File* file)
{
	for (float& f : lowCorner)
		file->writeFloat(f);
	for (float& f : highCorner)
		file->writeFloat(f);
}

void AACylinder::deserialize(File * file)
{
	for (float &f : center)
		f = file->readFloat();
	radius = file->readFloat();
	height = file->readFloat();
}

void AACylinder::serialize(File * file)
{
	for (float &f : center)
		file->writeFloat(f);
	file->writeFloat(radius);
	file->writeFloat(height);
}

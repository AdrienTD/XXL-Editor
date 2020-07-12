#pragma once

#include "vecmat.h"

struct File;

struct BoundingSphere {
	Vector3 center;
	float radius;

	BoundingSphere() : center(Vector3(0, 0, 0)), radius(0) {}
	BoundingSphere(const Vector3 &center, float radius) : center(center), radius(radius) {}

	bool containsPoint(const Vector3 &point) { return (point - center).len3() <= radius; }
	void merge(const BoundingSphere &other);
	void mergePoint(const Vector3 &point);

	void deserialize(File *file, bool withSquare = false);
	void serialize(File *file, bool withSquare = false);
};

struct AABoundingBox {
	Vector3 highCorner, lowCorner;

	void deserialize(File *file);
	void serialize(File *file);
};

struct AACylinder {
	Vector3 center;
	float radius, height;

	void deserialize(File *file);
	void serialize(File *file);
};
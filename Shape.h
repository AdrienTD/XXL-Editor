#pragma once

#include "vecmat.h"
#include <limits>
#include <cstdint>
#include <optional>

struct File;

struct BoundingSphere {
	Vector3 center;
	float radius;

	BoundingSphere() : center(Vector3(0, 0, 0)), radius(0) {}
	BoundingSphere(const Vector3 &center, float radius) : center(center), radius(radius) {}

	bool containsPoint(const Vector3& point) const;
	bool intersectsWithSphere(const BoundingSphere& other) const;
	float distanceToPoint(const Vector3& point) const;
	float distanceToSphere(const BoundingSphere& other) const;

	void merge(const BoundingSphere &other);
	void mergePoint(const Vector3 &point);

	void deserialize(File *file, bool withSquare = false);
	void serialize(File *file, bool withSquare = false);
};

struct AABoundingBox {
	Vector3 highCorner, lowCorner;

	AABoundingBox() : highCorner(-vecinf), lowCorner(vecinf) {}
	AABoundingBox(const Vector3 &point) : highCorner(point), lowCorner(point) {}
	AABoundingBox(const Vector3 &highCorner, const Vector3 &lowCorner) : highCorner(highCorner), lowCorner(lowCorner) {}

	void mergePoint(const Vector3 &point);
	void merge(const AABoundingBox &box);
	bool containsPoint(const Vector3& point) const;
	std::optional<AABoundingBox> intersectionWith(const AABoundingBox& box) const;

	void deserialize(File *file);
	void serialize(File *file);
	void deserializeLC(File *file);
	void serializeLC(File *file);

private:
	static constexpr auto fltinf = std::numeric_limits<float>::infinity();
	static constexpr auto vecinf = Vector3(fltinf, fltinf, fltinf);
};

struct AACylinder {
	Vector3 center;
	float radius, height;

	void deserialize(File *file);
	void serialize(File *file);
};

struct AARectangle {
	Vector3 center;
	float length1 = 1.0f, length2 = 1.0f;
	uint8_t direction = 0;

	AARectangle() = default;
	AARectangle(Vector3 center) : center(center) {}
};

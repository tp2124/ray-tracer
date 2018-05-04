#pragma once

#include "Vector3.h"

struct Geometry;

class Ray
{
public:
	explicit Ray(const Vector3<>& direction, const Vector3<>& origin = Vector3<>());

	// Raycasts, return -1 if no hit
	double getTimeToHit(Geometry& g) const;

	Vector3<> getPosAtTime(float time)		const;

	Vector3<> getOrigin()		const;
	Vector3<> getDirection()	const;
private:
	Vector3<> m_vOrigin;
	Vector3<> m_vDirection;
};
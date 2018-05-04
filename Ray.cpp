#include "Ray.h"
#include "3DGeometry.h"
#include <stdio.h>
#include <windows.h>

Ray::Ray(const Vector3<>& direction, const Vector3<>& origin) 
	: m_vDirection(direction.getNormalizedCopy())
	, m_vOrigin(origin)
{

}

double Ray::getTimeToHit(Geometry& g) const
{
	return g.getTimeToHit(*this);
}

Vector3<> Ray::getPosAtTime(float time)	const
{
	if(time < 0)
	{
		printf("\nWarning: getting position of ray at negative time!");
	}
	return (m_vOrigin + m_vDirection * time);
}

Vector3<> Ray::getOrigin() const
{
	return m_vOrigin;
}
Vector3<> Ray::getDirection() const
{
	return m_vDirection;
}
#include "3DGeometry.h"
#include "Ray.h"
#include "Utility.h"
#include <stdio.h>
#include <float.h>
#include <windows.h>


MaterialInfo::MaterialInfo()
{
	translucent = false;
	indexOfRefraction = 1.0;
}

//----Triangle---------------------------------------------------------------------------------------------------

Triangle::Triangle()
{

}

Triangle::Triangle(Vector3<> v1, Vector3<> v2, Vector3<> v3) 
{
	v[0].position = v1;
	v[1].position = v2;
	v[2].position = v3;
}

void Triangle::calculatePlaneNormal()
{
	Vector3<double> AB = v[1].position - v[0].position;
	Vector3<double> AC = v[2].position - v[0].position;

	// cache value for efficiency
	m_vPlaneNormal  =  (AB.cross(AC)).getNormalizedCopy();
}

Vector3<double> Triangle::getPlaneNormal() const
{
	return m_vPlaneNormal;
}

bool Triangle::isPointInside(const Vector3<>& point) 
{
	return calculateBarycentric(point);
}

bool Triangle::calculateBarycentric(const Vector3<>& point) 
{
	return calculateBarycentric(point, m_CachedBarycentric);
}
bool Triangle::calculateBarycentric(const Vector3<>& point, BarycentricCoordinates& result) const
{
	static double maxDiscrep = 0.000001;

	Triangle tCC1C2(point, v[1].position, v[2].position);
	Triangle tC0C1C2(v[0].position, v[1].position, v[2].position);

	Axis axis2 = YAxis;
	if(Vector3<>(0,0, 1).dot(m_vPlaneNormal) == 0)
		axis2 = ZAxis;

	double a = tCC1C2.getArea(XAxis, axis2) / tC0C1C2.getArea(XAxis, axis2);
	if(isBetween(0, a, 1, maxDiscrep))
	{
		Triangle tC0CC2(v[0].position, point,v[2].position);
		//tC0C1C2

		double b = tC0CC2.getArea(XAxis, axis2) / tC0C1C2.getArea(XAxis, axis2);
		if(isBetween(0, b, 1, maxDiscrep))
		{
			Triangle tC0C1C(v[0].position,v[1].position, point);
			//tC0C1C2

			double y = tC0C1C.getArea(XAxis, axis2) / tC0C1C2.getArea(XAxis, axis2);
			if(isBetween(0, y, 1, maxDiscrep))
			{
				double sum = a + b + y;
				if(isEqual(sum, 1, maxDiscrep))
				{
					result.m_vPoint = point;
					result.m_vBarycentricCoords = Vector3<>(a, b, y);

					//m_CachedBarycentric.m_vBarycentricCoords.normalize();
					return true;
				}
			}
		}
	}
	return false;
}

Vector3<> Triangle::getNormalAtPoint(const Vector3<>& point)
{

	Vector3<> vBaryCentric;
	Vector3<> normal;

	if(!m_CachedBarycentric.isValid(point))
	{
		calculateBarycentric(point);
	}
	vBaryCentric = m_CachedBarycentric.m_vBarycentricCoords;

	for(int i = 0; i < 3; i++)
	{
		normal += (v[i].normal * vBaryCentric[i]);
	}
	normal.normalize();

	return normal;
}

MaterialInfo Triangle::getMaterialInfoAtPoint(const Vector3<>& point)
{
	MaterialInfo info;
	info.translucent = v[0].material.translucent;
	info.colorTranslucency = v[0].material.colorTranslucency;

	if(!m_CachedBarycentric.isValid(point))
	{
		calculateBarycentric(point);
	}

	double baryCentric[3];
	baryCentric[0] = m_CachedBarycentric.m_vBarycentricCoords.getX();
	baryCentric[1] = m_CachedBarycentric.m_vBarycentricCoords.getY();
	baryCentric[2] = m_CachedBarycentric.m_vBarycentricCoords.getZ();

	for(int i = 0; i < 3; i++)
	{
		info.color_diffuse[i] = 0;
		info.color_specular[i] = 0;
		for(int j = 0; j< 3; j++)
		{
			info.color_diffuse[i] += (v[j].material.color_diffuse[i] *baryCentric[j]);
			info.color_specular[i] += (v[j].material.color_specular[i] *baryCentric[j]);
		}
	}
	return info;
}

double Triangle::getTimeToHit(const Ray& ray) 
{
	Vector3<>& tV = v[0].position;

	double denominator = ray.getDirection().dot(m_vPlaneNormal);

	// assuming the intersection point is not right on the screen/ray origin
	if(denominator!= 0)
	{
		double numerator = (tV - ray.getOrigin()).dot(m_vPlaneNormal);
		double time = numerator / denominator;

		if(time >= 0)
		{
			if(isPointInside(ray.getPosAtTime(time)))
			{
				return time;
			}
		}

		return -1;
	}

	// no hit
	return -1;
}

//--------Triangle-----------------------------------------------------------------------------------------------


//--------Sphere-----------------------------------------------------------------------------------------------

Vector3<> Sphere::getNormalAtPoint(const Vector3<>& point)
{
	Vector3<> normal = point - this->position;
	normal.normalize();
	return normal;
}

MaterialInfo Sphere::getMaterialInfoAtPoint(const Vector3<>& point)
{
	return this->material;
}

double Sphere::getTimeToHit(const Ray& ray) 
{
	//checking via projection for any collision
	Vector3<double> projVector;
	Vector3<double> sphereCenter = Vector3<double>(position);
	projVector = ray.getDirection() * ((sphereCenter - ray.getOrigin()).dot(ray.getDirection()));
	if(((ray.getOrigin() + projVector) - sphereCenter).getLenghtSq() >= radius * radius)
	{
		return -1;
	}

	double dif[3];
	
	// maybe will unroll loop later, but probably will not impact efficiency that much
	for(int i = 0; i< 3; i ++)
	{
		dif[i] = ray.getOrigin()[i]-position[i];
	}


	double b = 2 * (ray.getDirection().getX() * (dif[0]) + ray.getDirection().getY() * (dif[1]) + ray.getDirection().getZ() * (dif[2]));
	double bSq=b*b;
	double root = bSq - 4 * (pow(dif[0], 2) + pow(dif[1], 2) + pow(dif[2], 2) - pow(radius, 2));

	root = sqrt(root);

	double t1, t2;
	t1 = t2 = root/2;

	t1 =-b/2 + t1;
	t2 =-b/2 - t2;

	if(t1 < 0.01)
	{
		t1 = DBL_MAX;
	}
	if (t2 < 0.01)
	{
		t2 = DBL_MAX;
	}

	return min(t1, t2);	
}

//-------------------------------------------------------------------------------------------------------
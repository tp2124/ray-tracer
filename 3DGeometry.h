#pragma once

#include "Vector3.h"
//#include <unordered_map>
#include <map>

class Ray;

struct MaterialInfo
{
	MaterialInfo();

	double color_diffuse[3];
	double color_specular[3];
	double shininess;

	bool translucent;
	double colorTranslucency;
	double indexOfRefraction;
};

struct BarycentricCoordinates
{
	Vector3<> m_vPoint;
	Vector3<> m_vBarycentricCoords;

	bool isValid(Vector3<> point) 
	{
		return (m_vPoint == point);
	}

	double& operator[](int index)
	{
		return (m_vBarycentricCoords[index]);
	}
};

struct Geometry
{
	virtual Vector3<> getNormalAtPoint(const Vector3<>& point) = 0;
	virtual MaterialInfo getMaterialInfoAtPoint(const Vector3<>& point) = 0;

	virtual double getTimeToHit(const Ray& ray) = 0;
};

typedef enum  {XAxis, YAxis, ZAxis} Axis;

struct Vertex
{
	Vector3<double> position;
	Vector3<double> normal;
	MaterialInfo material;
};

struct Triangle : Geometry
{
	Triangle();

	// constructor for quick geometric tests (isPointInside)
	Triangle(Vector3<> v1, Vector3<> v2, Vector3<> v3);

	Vertex v[3];

	void calculatePlaneNormal();
	Vector3<double> getPlaneNormal() const;

	double getArea(Axis axis1, Axis axis2) const
	{
		if(axis1 == axis2) {
		//	printf("Warning: calculating 2D area of triangle using same axis");
			return -1;
		}

		double area = 0.5;
		double val1 = (v[1].position.getAxis(axis1) - v[0].position.getAxis(axis1)) * (v[2].position.getAxis(axis2) - v[0].position.getAxis(axis2));
		double val2 = (v[2].position.getAxis(axis1)- v[0].position.getAxis(axis1)) * (v[1].position.getAxis(axis2) - v[0].position.getAxis(axis2));
		area *= (val1 - val2);

		return area;
	}

	virtual Vector3<> getNormalAtPoint(const Vector3<>& point);
	virtual MaterialInfo getMaterialInfoAtPoint(const Vector3<>& point);

	virtual double getTimeToHit(const Ray& ray);

private:
	bool isPointInside(const Vector3<>& point);
	bool calculateBarycentric(const Vector3<>& point);
	bool calculateBarycentric(const Vector3<>& point, BarycentricCoordinates& result) const;

	BarycentricCoordinates m_CachedBarycentric;

	//std::unordered_map<Vector3<>, BarycentricCoordinates, Vector3Hasher<double>> m_mapBarycentrics;
	std::map<Vector3<>, BarycentricCoordinates> m_mapBarycentrics;

	Vector3<> m_vPlaneNormal;
};

struct Sphere : Geometry
{
	double position[3];
	Vector3<double> vPos;
	MaterialInfo material;
	double radius;

	virtual Vector3<> getNormalAtPoint(const Vector3<>& point);
	virtual MaterialInfo getMaterialInfoAtPoint(const Vector3<>& point);

	virtual double getTimeToHit(const Ray& ray);
};

typedef struct _Light
{
	//double position[3];
	Vector3<> position;
	double color[3];
} Light;
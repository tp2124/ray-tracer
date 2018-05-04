#pragma once
#include <cmath>
// cannot seperate templates into .h and .cpp apperantly

template <class T = double>
class Vector3
{
public:
	Vector3(T X, T Y, T Z)
	{
		setCoords(X, Y, Z);
	}
	Vector3()
	{
		setCoords(0, 0, 0);
	}
	Vector3(Vector3<T> const & other )
	{
		setValues(other.getValues());
	}
	Vector3(const T(&val)[3])
	{
		setValues(val);
	}

	~Vector3()
	{
		//delete values;
	}

	T getAxis(int index) const
	{
		return (values[index]);
	}

	T getX() const
	{
		return (values[0]);
	}
	T getY() const
	{
		return (values[1]);
	}
	T getZ() const
	{
		return (values[2]);
	}

	void setX(T X)
	{
		values[0] = X;
	}

	void setY(T Y)
	{
		values[1] = Y;
	}

	void setZ(T Z)
	{
		values[2] = Z;
	}

	void setCoords(T X, T Y, T Z)
	{
		setX(X);
		setY(Y);
		setZ(Z);
	}

	T getLenghtSq() const
	{
		T sum = 0;
		for(unsigned int i = 0; i < 3; i ++)
		{
			sum+= (values[i] * values[i]);
		}

		return sum;
	}

	T getLength() const
	{
		return sqrt(getLenghtSq());
	}

	void normalize()
	{
		T length = getLength();
		for(int i = 0; i < 3; i++)
		{
			values[i] /= length;
		}
	}

	Vector3<T> getNormalizedCopy()  const
	{
		Vector3<T> copy = Vector3<T>(*this);
		copy.normalize();
		return copy;
	}

	T dot(const Vector3<T> &other) const
	{
		return getX() * other.getX() + getY() * other.getY() + getZ()* other.getZ();
	}

	Vector3<T> cross(const Vector3<T> &other) const
	{
		Vector3<T> result;

		result[0] = values[1] * other.values[2] - values[2] * other.values[1];
		result[1] = values[2] * other.values[0] - values[0] * other.values[2];
		result[2] = values[0] * other.values[1] - values[1] * other.values[0];

		return result;
	}

	Vector3<T> getReflectedVector(const Vector3<T>& surfaceNormal, bool areAllNormalized = false) const
	{
		Vector3<T> reflection = ((*this * -1) + surfaceNormal * (this->dot(surfaceNormal) ) * 2);
		if(!areAllNormalized)
		{
			reflection.normalize();
		}
		return reflection;
	}

	T angleTo(Vector3<T> &other)
	{
		Vector3<T> norm1(getNormalizedCopy());
		Vector3<T> norm2(other.getNormalizedCopy());

		return (acos((norm1.dot(norm2))));
	}

	T signedAngleTo(Vector3<T> &other)
	{
		// TO DO
	}

	bool operator==(const Vector3<T> &other) const
	{
		return(other.getX() == getX() && other.getY() == getY() && other.getZ() == getZ());
	}

	bool operator!=(const Vector3<T> &other) const
	{
		return !(*this==other);
	}

	// This was causing issues with rendering Triangles starting with 02 optimization
	//Vector3<T>& operator=(const Vector3<T> &other)
	//{
	//	setValues(other.getValues());

	//	return *this;
	//}

	Vector3<T>& operator+=(const Vector3<T> &other)
	{
		values[0]+=other.getX();
		values[1]+=other.getY();
		values[2]+=other.getZ();

		return *this;
	}
	Vector3<T>& operator-=(const Vector3<T> &other)
	{
		values[0]-=other.getX();
		values[1]-=other.getY();
		values[2]-=other.getZ();

		return *this;
	}

	Vector3<T> operator+(const Vector3<T> &other) const
	{
		Vector3<T> result = *this;
		result+=other;
		return result;
	}
	Vector3<T> operator-(const Vector3<T> &other) const
	{
		Vector3<T> result = *this;
		result-=other;
		return result;
	}

	friend Vector3<T> operator-(const Vector3<T> &other);

	Vector3<T>& operator*=(T num)
	{
		values[0]*=num;
		values[1]*=num;
		values[2]*=num;

		return *this;
	}
	Vector3<T>& operator/=(T num)
	{
		values[0]/=num;
		values[1]/=num;
		values[2]/=num;

		return *this;
	}
	Vector3<T> operator*(T num) const
	{
		Vector3<T> result = *this;
		result*=num;
		return result;
	}
	Vector3<T> operator/(T num) const
	{
		Vector3<T> result = *this;
		result/=num;
		return result;
	}

	// ------------------------------------
	//Using these operators to store vectors in a map. I could not figure out the errors when storing in an unordered map
	bool operator <(const Vector3<T> &other) const
	{
		return (this->getValueSum() < other.getValueSum());
	}


	// ------------------------------------

	T& operator[](int index)
	{
		return (values[index]);
	}

	const T(&getValues() const) [3]
	{
		return values;
	}

	void setValues(const T(&val)[3])
	{
		for(int i = 0; i < 3; i ++)
		{
			values[i] = val[i];
		}
	}

	T getValueSum() const
	{
		T sum = 0;
		for(int i = 0; i < 3; i ++)
		{
			sum += values[i];
		}

		return sum;
	}

	T values [3];

private:

};

#pragma once

template <class T>
bool isBetween(T min, T val, T max, T maxDiscrepancy)
{
	return (val > min - maxDiscrepancy && val < max + maxDiscrepancy);
}

template <class S, class T>
bool isBetween(S min, T val, S max, T maxDiscrepancy)
{
	return isBetween((T)min, val, (T)max, maxDiscrepancy);
}

template <class T>
bool isEqual(T val1, T val2, T maxDiscrepancy)
{
	return(isBetween(val1, val2, val1, maxDiscrepancy));
}

template <class S, class T>
bool isEqual(T val1, S val2, T maxDiscrepancy)
{
	return (isEqual(val1, (T)val2, maxDiscrepancy));
}

#define PI 3.14159265
template <class T>
T radianToDegree(T radian)
{
    T degree = 0;
    degree = radian * (180/PI);
    return degree;
}

template <class T>
T degreeToRadian(T degree)
{
    T radian = 0;
    radian = degree * (PI/180);
    return radian;
}

template <class T>
T getRandomInRange(T minVal, T maxVal)
{
	T random = ((T) rand()) / (T) RAND_MAX;

    // generate (in your case) a float between 0 and (4.5-.78)
    // then add .78, giving you a float between .78 and 4.5
    T range = maxVal - minVal;  
    return (random*range) + minVal;
}

template <class T>
T clamp(T value, T minVal, T maxVal)
{
	if(value < minVal)
	{
		return minVal;
	}
	if(value > maxVal)
	{
		return maxVal;
	}
	return value;
}

template <class S, class T>
T clamp(T value, S minVal, S maxVal)
{
	return (clamp(value, (T) minVal, (T) maxVal));
}
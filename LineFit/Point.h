#ifndef _point_h_
#define _point_h_


#include <stdio.h>
#define	_USE_MATH_DEFINES
#include <math.h>
#undef	_USE_MATH_DEFINES
#include <iostream>
#include <vector>

using namespace std;

template <class T>
class Point
{
public:
	Point() {};
	Point(T xInput,T yInput): x(xInput), y(yInput) {};
	T x;
	T y;
	inline Point& operator=(const Point &rhs);	
};

template <class T> 
Point<T>& Point<T>::operator=(const Point &rhs)
{
	x = rhs.x;
	y = rhs.y;
	return (*this);
}

#endif
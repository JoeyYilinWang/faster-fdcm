#ifndef _lf_line_segment_h_
#define _lf_line_segment_h_

#include <stdio.h>
#define	_USE_MATH_DEFINES
#include <math.h>
#undef	_USE_MATH_DEFINES

//#include <cxcore.h>
#include <iostream>
#include <vector>
#include "LineFit/Point.h"

using namespace std;

class LFLineSegment
{
public:
	LFLineSegment() {};
	~LFLineSegment() {};

	double sx_,sy_,ex_,ey_; // define the start and end points of line segment
	int nSupport_; // number of supporting points of line segment
	double len_; // length of line segment
	Point<double> normal_; //该直线段的法向量，由Point类型表示

	void Read(FILE* fin); 	
	void Center(double *center); // the center of line segment
	void Translate(double *vec); // translation of line segment
	
	double Theta(); 
	
	void Aspect(double a);
	void Rotate(double theta); // 旋转元素
	void Scale(double s);
	double Length();


	inline LFLineSegment& operator=(const LFLineSegment &rhs);	
	inline bool operator==(const LFLineSegment &other) const;
	inline bool operator!=(const LFLineSegment &other) const;
	inline bool operator>=(const LFLineSegment &other) const;
	inline bool operator<=(const LFLineSegment &other) const;
	inline bool operator>(const LFLineSegment &other) const;
	inline bool operator<(const LFLineSegment &other) const;
	inline friend ostream &operator<<(ostream &output,LFLineSegment &e); // 友元函数
};


inline LFLineSegment& LFLineSegment::operator=(const LFLineSegment &rhs)
{
	sx_ = rhs.sx_;
	sy_ = rhs.sy_;
	ex_ = rhs.ex_;
	ey_ = rhs.ey_;
	len_ = rhs.len_;

	nSupport_ = rhs.nSupport_;
	normal_ = rhs.normal_;
	return (*this);
}

ostream &operator<<(ostream &output,LFLineSegment &e)
{
	output<< "(" <<  e.sx_ <<", " << e.sy_ <<"), (" << e.ex_ <<", " << e.ey_ <<"), ";
    return output;
}


bool LFLineSegment::operator==(const LFLineSegment &other) const
{
	return len_ == other.len_;
}

bool LFLineSegment::operator!=(const LFLineSegment &other) const
{
	return len_ != other.len_;
}

bool LFLineSegment::operator>=(const LFLineSegment &other) const
{
	return len_ >= other.len_;
}

bool LFLineSegment::operator<=(const LFLineSegment &other) const
{
	return len_ <= other.len_;
}

bool LFLineSegment::operator>(const LFLineSegment &other) const
{
	return len_ > other.len_;
}

bool LFLineSegment::operator<(const LFLineSegment &other) const
{
	return len_ < other.len_;
}


#endif
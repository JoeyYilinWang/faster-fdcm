#ifndef _lm_det_wind_h_
#define _lm_det_wind_h_

#include <iostream>
#include <fstream>

using namespace std;

class LMDetWind
{
public:
	int x_;
	int y_;
	int width_;
	int height_;
	double cost_;
	int count_;

	inline LMDetWind &operator=(const LMDetWind &rhs);
	inline bool operator==(const LMDetWind &other) const;
	inline bool operator!=(const LMDetWind &other) const;
	inline bool operator>=(const LMDetWind &other) const;
	inline bool operator<=(const LMDetWind &other) const;
	inline bool operator>(const LMDetWind &other) const;
	inline bool operator<(const LMDetWind &other) const;
	inline friend ostream &operator<<(ostream &output,LMDetWind &e);
};


ostream &operator<<(ostream &output,LMDetWind &e)
{
	//output<< "(" <<  e.x_ <<", " << e.y_ <<", " << e.width_ <<", " << e.height_ <<", " << e.cost_ <<", " << e.count_ <<")";
	output<< e.x_ <<" "<< e.y_ <<" "<< e.width_ <<" " << e.height_ <<" " << e.cost_ <<" " << e.count_;
    return output;
}


LMDetWind &LMDetWind::operator=(const LMDetWind &rhs)
{
	x_ = rhs.x_;
	y_ = rhs.y_;
	width_ = rhs.width_;
	height_ = rhs.height_;
	cost_ = rhs.cost_;
	count_ = rhs.count_;
	return *this;
}

bool LMDetWind::operator==(const LMDetWind &other) const
{
	return cost_ == other.cost_;
}

bool LMDetWind::operator!=(const LMDetWind &other) const
{
	return cost_ != other.cost_;
}

bool LMDetWind::operator>=(const LMDetWind &other) const
{
	return cost_ >= other.cost_;
}

bool LMDetWind::operator<=(const LMDetWind &other) const
{
	return cost_ <= other.cost_;
}

bool LMDetWind::operator>(const LMDetWind &other) const
{
	return cost_ > other.cost_;
}

bool LMDetWind::operator<(const LMDetWind &other) const
{
	return cost_ < other.cost_;
}

#endif
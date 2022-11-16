#ifndef _matching_cost_map_h_
#define _matching_cost_map_h_

#include "LineFit/MMFunction.h"
#include <vector>
using namespace std;

class MatchingCostMap
{
public:
	MatchingCostMap();
	~MatchingCostMap();
	void Release();
	void Init(int nCostMap);

	vector<double> *costMap_;
	int nCostMap_;

	vector<int> width_;
	vector<int> height_;
	vector<int> templateWidth_;
	vector<int> templateHeight_;
	vector<int> stepSize_;
	vector<int> x0_;
	vector<int> y0_;
	vector<double> scale_;
	vector<double> aspect_;


};


#endif
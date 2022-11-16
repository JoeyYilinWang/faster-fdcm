#ifndef _lm_non_maximum_suppression_h_
#define _lm_non_maximum_suppression_h_

#include "MatchingCostMap.h"
#include "LMDetWind.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
using namespace std;

class LMNonMaximumSuppression
{
public:

	// Use the detection threshold and the overlapping threshold to find the local minima in the matching cost map.
	static void ComputeDetection(MatchingCostMap &matchingCostMap,double threshold,double overlapThresh,vector<LMDetWind> &wind,int varyingQuerySize=1);

	// Check if the current window ( curWind ) has a significant overlap with any previous windows stored in wind.
	static bool IsOverlapping(LMDetWind &curWind,vector<LMDetWind> &wind,double overlapThresh);

	// Compute the overlapping ratio, the intersection region over the union region, between the two detection windows.
	static double OverlapRatio(LMDetWind &curWind,LMDetWind &refWind);

	// Check which windows are valid detections and construct the window dimension for varying query image size.
	static void ComputeValidWindVaryingQuerySize(MatchingCostMap &matchingCostMap,vector<LMDetWind> &detWinds,double threshold);

	// Check which windows are valid detections and construct the window dimension for varying template size.
	static void ComputeValidWindVaryingTemplateSize(MatchingCostMap &matchingCostMap,vector<LMDetWind> &detWinds,double threshold);

};

#endif
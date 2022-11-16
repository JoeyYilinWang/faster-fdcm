#ifndef _lm_distance_image_h_
#define _lm_distance_image_h_

#include "Image/Image.h"
#include "Image/DistanceTransform.h"
#include "EIEdgeImage.h"
#include "LMDirectionalIntegralDistanceImage.h"


class LMDistanceImage 
{
public:

	LMDistanceImage();
	~LMDistanceImage();
	void Configure(float directionCost,double maxCost){ directionCost_ = directionCost; maxCost_ = maxCost; };
	void SetImage(EIEdgeImage& ei);
	

private:

	void SafeRelease();
	void ConstructDTs(EIEdgeImage& ei);
	void UpdateCosts();
	void ConstructDIntegrals();
	
	vector<LMDirectionalIntegralDistanceImage> idtImages_;
	int nDirections_;
	int width_;
	int height_;
	
	vector< Image<float> > dtImages_;
	float directionCost_;
	double maxCost_;
	friend class LMLineMatcher;
};

#endif
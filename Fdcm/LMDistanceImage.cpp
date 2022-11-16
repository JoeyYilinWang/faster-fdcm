#include "LMDistanceImage.h"

LMDistanceImage::LMDistanceImage()
{

}

LMDistanceImage::~LMDistanceImage()
{
	SafeRelease();
}


void LMDistanceImage::SafeRelease()
{
	dtImages_.clear();
	idtImages_.clear();
}


void LMDistanceImage::SetImage(EIEdgeImage& ei)
{
	SafeRelease();
	width_ = ei.width_;
	height_ = ei.height_;
	nDirections_ = ei.nDirections_;

	ConstructDTs(ei);
	UpdateCosts();
	ConstructDIntegrals();
}

void LMDistanceImage::ConstructDTs(EIEdgeImage& ei)
{
	Image<uchar> image(width_,height_,false);
	dtImages_.resize(nDirections_);

	for (int i=0 ; i<ei.nDirections_ ; i++)
	{
		dtImages_[i].Resize(width_,height_,false);
		ei.ConstructDirectionImage(i, &image);
		DistanceTransform::CompDT(&image, &dtImages_[i], false);
	}
}


void LMDistanceImage::UpdateCosts()
{
	float* costs;
	costs = new float[nDirections_];

	float **buffers = new float*[nDirections_];
	for (int i=0;i<nDirections_ ; i++)
	{
		buffers[i] = (float*) dtImages_[i].data;
	}
	
	
	int wh = width_*height_;
	for (int k=0 ; k<wh ; k++)
	{
		for (int i=0 ; i<nDirections_ ; i++)
		{
			costs[i] = buffers[i][k];
			if (costs[i] > maxCost_)
				costs[i] = (float)maxCost_;
		}

		//forward pass
		if (costs[0] > costs[nDirections_-1] + directionCost_)
			costs[0] = costs[nDirections_-1] + directionCost_;
		for (int i=1 ; i<nDirections_ ; i++)
		{
			if (costs[i] > costs[i-1] + directionCost_)
				costs[i] = costs[i-1] + directionCost_;
		}

		if (costs[0] > costs[nDirections_-1] + directionCost_)
			costs[0] = costs[nDirections_-1] + directionCost_;
		for (int i=1 ; i<nDirections_ ; i++)
		{
			if (costs[i] > costs[i-1] + directionCost_)
				costs[i] = costs[i-1] + directionCost_;
			else
				break;
		}

		//backward pass
		if (costs[nDirections_-1] > costs[0] + directionCost_)
			costs[nDirections_-1] = costs[0] + directionCost_;
		for (int i=nDirections_-1 ; i>0 ; i--)
		{
			if (costs[i-1] > costs[i] + directionCost_)
				costs[i-1] = costs[i] + directionCost_;
		}

		if (costs[nDirections_-1] > costs[0] + directionCost_)
			costs[nDirections_-1] = costs[0] + directionCost_;
		for (int i=nDirections_-1 ; i>0 ; i--)
		{
			if (costs[i-1] > costs[i] + directionCost_)
				costs[i-1] = costs[i] + directionCost_;
			else
				break;
		}

		for (int i=0 ; i<nDirections_ ; i++)
		{
			buffers[i][k] =  costs[i];
		}

	}

	delete[] costs;
	delete[] buffers;
}


void LMDistanceImage::ConstructDIntegrals()
{
	double theta;
	idtImages_.resize(nDirections_);
	for (int i=0 ; i<nDirections_ ; i++)
	{
		theta = (i*M_PI)/nDirections_ + M_PI/(2*nDirections_);
		idtImages_[i].CreateImage(width_,height_);		
		idtImages_[i].Construct(&dtImages_[i], (float)cos(theta), (float)sin(theta));
	}
}
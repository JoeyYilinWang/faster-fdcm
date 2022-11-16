#include "MatchingCostMap.h"

MatchingCostMap::MatchingCostMap()
{
	costMap_ = NULL;
	nCostMap_ = 0;

}

MatchingCostMap::~MatchingCostMap()
{
	Release();	
}
	
void MatchingCostMap::Release()
{
	if(costMap_)
	{
		for(int i=0;i<nCostMap_;i++)
			costMap_[i].clear();
		delete [] costMap_;
		nCostMap_ = 0;
	}
	templateWidth_.clear();
	templateHeight_.clear();
	width_.clear();
	height_.clear();
	stepSize_.clear();
	x0_.clear();
	y0_.clear();
	scale_.clear();
	aspect_.clear();
}

void MatchingCostMap::Init(int nCostMap)
{
	nCostMap_ = nCostMap;
	costMap_ = new vector<double> [nCostMap_];
	templateWidth_.resize(nCostMap_);
	templateHeight_.resize(nCostMap_);
	width_.resize(nCostMap_);
	height_.resize(nCostMap_);
	stepSize_.resize(nCostMap_);
	x0_.resize(nCostMap_);
	y0_.resize(nCostMap_);
	scale_.resize(nCostMap_);
	aspect_.resize(nCostMap_);
}
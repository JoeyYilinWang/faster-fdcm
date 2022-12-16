/*
Copyright 2022, Yi-Lin Wang
All Rights Reserved 
Permission to use, copy, modify, and distribute this software and 
its documentation for any non-commercial purpose is hereby granted 
without fee, provided that the above copyright notice appear in 
all copies and that both that copyright notice and this permission 
notice appear in supporting documentation, and that the name of 
the author not be used in advertising or publicity pertaining to 
distribution of the software without specific, written prior 
permission. 
THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
ANY PARTICULAR PURPOSE. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR 
ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN 
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING 
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
*/

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

	// 构建针对query map的二维距离图
	ConstructDTs(ei);
	// 构建针对query map的三维距离图
	UpdateCosts();
	// 构建针对query map的三维距离积分图
	ConstructDIntegrals();
}


// 构建针对query map的三维距离变换图，而且是针对不同方向的
void LMDistanceImage::ConstructDTs(EIEdgeImage& ei)
{
	Image<uchar> image(width_,height_,false);

	// 针对每个量化方向建立距离变换图
	dtImages_.resize(nDirections_);

	for (int i=0 ; i<ei.nDirections_ ; i++)
	{
		dtImages_[i].Resize(width_,height_,false);
		ei.ConstructDirectionImage(i, &image);

		// dtImages_[i]保存着每个像素到相邻边的最小距离， image的背景颜色为白色255，直线段为黑色0
		DistanceTransform::CompDT(&image, &dtImages_[i], false);
	}
}


void LMDistanceImage::UpdateCosts()
{
	float* costs;

	// 针对每个方向，都定义一个cost
	costs = new float[nDirections_];

	// 分配新的储存空间，其内部保存着float*数据
	float **buffers = new float*[nDirections_];
	for (int i=0;i<nDirections_ ; i++)
	{
		// buffers[i]储存某个方向distanceTransformImage的数据头指针
		buffers[i] = (float*) dtImages_[i].data;
	}
	

	int wh = width_*height_;
	for (int k=0 ; k<wh ; k++)
	{
		// 固定像素位置，遍历不同方向对应的二维距离变换图
		for (int i=0 ; i<nDirections_ ; i++)
		{
			// costs保存了第k个像素位置上，方向[i]上对应的最小距离，作为论文提到的初始值
			costs[i] = buffers[i][k]; // buffers[i][k]表示第i个方向，第k个像素与最近边的最小距离

			// 如果第i方向、第k个像素上的距离大于一个阈值maxCost，则costs[i]赋值为该阈值
			if (costs[i] > maxCost_)
				costs[i] = (float)maxCost_;
		}

		// 前向遍历，directionCost_为连续两个方向之间的方向误差
		// 同一位置上，若到具有方向i的直线段距离大于到具有方向i-1的直线段距离+directionCost_，则该位置上到具有方向i的直线段距离将被赋予此值
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

		//后向遍历
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
		
		// 经过前后两次遍历后，dtImages_[i]保存了各方向上最小的距离（包括角度误差）。三维距离变换图构造完成
		for (int i=0 ; i<nDirections_ ; i++)
		{
			buffers[i][k] =  costs[i];
		}

	}

	delete[] costs;
	delete[] buffers;
}


// 根据三维距离变换图构建三维距离变换积分图
void LMDistanceImage::ConstructDIntegrals()
{
	double theta;
	idtImages_.resize(nDirections_);

	// 对每个方向都构造距离变换积分图
	for (int i=0 ; i<nDirections_ ; i++)
	{	
		// 设定某个方向
		theta = (i*M_PI)/nDirections_ + M_PI/(2*nDirections_);
		idtImages_[i].CreateImage(width_,height_);		
		// 使用2D distance transformed image[i]进行处理							
		idtImages_[i].Construct(&dtImages_[i], (float)cos(theta), (float)sin(theta));
	}
}
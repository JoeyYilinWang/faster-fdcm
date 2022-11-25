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

#ifndef _ei_edge_image_h_
#define _ei_edge_image_h_

#include "LineFit/LineSegment.h"
#include "LineFit/LineFitter.h"
#include "LineFit/Point.h"
#include "Image/Image.h"
#include "Image/ImageDraw.h"
#include <vector>
using namespace std;



class EIEdgeImage {
public:
	EIEdgeImage();
	~EIEdgeImage();
	void SafeRelease();	
	void Read(const char* fileName);
	void Read(LFLineFitter &lf);
	
	void Read(double *lineRep,int nLine);

	void SetNumDirections(const int nDirections) {nDirections_=nDirections;};
	void Scale(double s);
	void Aspect(double a);
	void Tight(double &minx,double &miny,double &maxx,double &maxy);
	double Length();
	void ConstructDirectionImage(int index,Image<uchar>* image);
	void operator=(EIEdgeImage& ei);
	void Boundary(double &minx, double &miny, double &maxx, double &maxy);
	void SetDirectionIndices();


	// Display
	void ConstructImage(Image<RGBMap> *image, int thickness = 1);

	void ConstructOrientedImage(Image<RGBMap> *image,Image<double> *orientImage, int thickness = 1);

	void SetLines2Grid();
	void SetDirections();
	int Theta2Index(double theta);
	double Index2Theta(int index);

	// 模板图像宽度
	int width_;
	// 模板图像高度
	int height_;
	// 模板图像中包含的直线段数量
	int	nLines_;
	// 包含的方向
	int nDirections_;

	// 指向直线段列表的头指针
	LFLineSegment* lines_;

	/** 建立指向vector的指针。而该vector中的元素都是指向LFLineSegment的指针，本质上vector中保存着直线段列表（而该列表表示为列表头元素的指针），
	 * vector中的元素实际上都对应着各方向对应的直线段列表。
	 */
	vector<LFLineSegment*>* directions_;

	int* directionIndices_;

};
#endif
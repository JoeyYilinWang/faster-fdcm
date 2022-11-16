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

	// This is for reading line representation from MATLAB so the indexing is column by column.
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

	int width_;
	int height_;
	int	nLines_;
	int nDirections_;

	LFLineSegment* lines_;
	vector<LFLineSegment*>* directions_;
	int* directionIndices_;

};
#endif
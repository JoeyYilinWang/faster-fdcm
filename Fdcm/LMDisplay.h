#ifndef _lm_display_h_
#define _lm_display_h_

#include "Image/Image.h"
#include "Image/ImageIO.h"
#include "Image/ImageDraw.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include "LMDetWind.h"
#include "EIEdgeImage.h"
using namespace std;

class LMDisplay
{
public:

	static void DrawDetWind(Image<RGBMap> *image,int x,int y,int detWindWidth,int detWindHeight,const RGBMap scalar,int thickness=1);
	static void DrawDetWindCost(Image<RGBMap> *image,LMDetWind &wind,const RGBMap scalar,int thickness=2);
	static void DrawDetWindWind(Image<RGBMap> *image,LMDetWind &wind,const RGBMap scalar,int thickness=2);
	static void DrawMatchTemplate(Image<RGBMap> *image,EIEdgeImage &ei,int x,int y,double scale,const RGBMap scalar,int thickness=1);


	static void StoreDetWind(const char *filename,vector< vector<LMDetWind> > &detWindArrays);

};


#endif
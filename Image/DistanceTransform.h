#ifndef _distance_transform_h_
#define _distance_transform_h_

#include <algorithm>
#include <cmath>
#include "Image.h"

using namespace std;

#define INF 1e10

template <class T> inline T square(const T &x) { return x*x; };

//
// The distance transform result is the same as the result computed with MATLAB. 
// The neareast neighbor map is the same as the map computed with MATLAB.
//
class DistanceTransform
{
public:
	static void CompDT(const Image<uchar> *input,Image<float> *output,bool onEqualOne=true,Image<int> *nn=NULL);

private:

	static void Update2DDTCost(Image<float> *output);
	static float *Update1DDTCost(float *f, const int n);

	// update cost and store nearest neighbors
	static void Update2DDTCostNN(Image<float> *output,Image<int> *nn);
	static float *Update1DDTCostNN(float *f, const int n, int *ind);

};

#endif
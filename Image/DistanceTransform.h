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

#ifndef _distance_transform_h_
#define _distance_transform_h_

#include <algorithm>
#include <cmath>
#include "Image.h"

using namespace std;

#define INF 1e10

// define template class to compute square of x
template <class T> 
inline T square(const T &x) { return x*x; };

// 距离变换积分图类定义
class DistanceTransform
{
public:
	// 输入图像中各像素保存的值是unsigned char，不是RGBMap。
	// 输出有两个，一个是保存各像素与最近边的距离值，一个是保存各像素与最近边上像素的索引
	static void CompDT(const Image<uchar> *input, Image<float> *output, bool onEqualOne=true, Image<int> *nn=NULL);
	static float *Update1DDTCost(float *f, const int n);

private:

	static void Update2DDTCost(Image<float> *output);
	// static float *Update1DDTCost(float *f, const int n);

	// update cost and store nearest neighbors
	static void Update2DDTCostNN(Image<float> *output,Image<int> *nn);
	static float *Update1DDTCostNN(float *f, const int n, int *ind);

};

#endif
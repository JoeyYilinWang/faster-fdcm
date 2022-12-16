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

#include "DistanceTransform.h"

// dt of binary image using squared distance
void DistanceTransform::CompDT(const Image<uchar> *input,Image<float> *output,bool onEqualOne,Image<int> *nn)
{

	uchar on;
	on = (onEqualOne) ? 255 : 0;

	int imageWidth = input->width();
	int imageHeight = input->height();

	// 2D距离变换图初始化，原始位置上有直线段的设置为0，其他位置为INF，表示无穷大
	for(int y=0;y<imageHeight;y++)
	{
		for(int x=0;x<imageWidth;x++)
		{	
			// each pixel location on input image can only be assigned 255 or 0, just a label
			if( imRef(input,x,y) == on )
				// the same pixel location on output image as input image can only be assigned 0 if the same pixel location on input image is assigned "on"
				imRef(output,x,y) = 0;
			else // otherwise, to be assigned INF
				imRef(output,x,y) = INF;
		}
	}

	if(nn)
	{
		Update2DDTCostNN(output,nn);
	}
	else
	{
		Update2DDTCost(output);
	}

	// square root
	for (int y = 0; y < imageHeight; y++) 
	{
		for (int x = 0; x < imageWidth; x++) 
		{
			//cvSetReal2D( output, y, x, sqrt( cvGetReal2D( output, y, x) ) );
			imRef(output,x,y) = sqrt( imRef(output,x,y) ) ;
		}
	}
}


/* dt of 2d function using squared distance */
void DistanceTransform::Update2DDTCostNN(Image<float> *output,Image<int> *nn)
{
	int imageWidth = output->width();
	int imageHeight = output->height();
	float *f = new float[ max( imageWidth, imageHeight ) ];
	int *ind = new int[ max( imageWidth, imageHeight ) ]; 

	// transform along columns
	for (int x = 0; x < imageWidth; x++) 
	{
		for (int y = 0; y < imageHeight; y++) 
		{
			//f[ y ] = (double)cvGetReal2D( output, y, x);
			f[ y ] = imRef(output,x,y);
			ind[ y ] = y*imageWidth + x;
		}
		float *d = Update1DDTCostNN(f, imageHeight, ind);
		for (int y = 0; y < imageHeight; y++) 
		{
			//cvSetReal2D( output, y, x, d[y]);
			imRef(output,x,y) = d[y];
			//cvSetReal2D( nn, y, x, (double)ind[ y ]);
			imRef(nn,x,y) = ind[y];
		}
		delete [] d;
	}                       


	// transform along rows
	for (int y=0; y<imageHeight; y++) 
	{
		for( int x = 0; x<imageWidth; x++) 
		{
			//f[ x ] = (double)cvGetReal2D( output, y, x);
			f[y] = imRef(output,x,y);
			//ind[ x ] = (int) cvGetReal2D( nn, y, x);
			ind[ x ] = imRef(nn,x,y);
		}
		float *d = Update1DDTCostNN(f, imageWidth, ind);
		for (int x = 0; x < imageWidth; x++) 
		{
			//cvSetReal2D( output, y, x, d[x]);
			imRef( output, x, y) = d[x];
			//cvSetReal2D( nn, y, x, (double)ind[ x ]);
			imRef( nn, x, y) = ind[x];
		}
		delete [] d;
	}


	delete [] ind;
	delete [] f;

}


// dt of 1d function using squared distance
float* DistanceTransform::Update1DDTCostNN(float *f, const int n, int *ind)
{
	float *d = new float [n];
	int *v = new int[n];
	float *z = new float[n+1];

	int *tmpInd = new int [n];

	int k = 0;
	v[0] = 0;
	z[0] = -INF;
	z[1] = +INF;

	for (int q=1; q<=n-1; q++) 
	{
		float s = ((f[q]+square(q)) - (f[v[k]]+square(v[k]))) / (2*q - 2*v[k]);
		while ( s <= z[k] ) 
		{
			k--;
			s = ((f[q]+square(q)) - (f[v[k]]+square(v[k]))) / (2*q - 2*v[k]);
		}
		k++;
		v[k] = q;
		z[k] = s; // z[k]一直保存最大的
		z[k+1] = INF;
		
	}

	k = 0;
	for (int q=0; q<=n-1; q++) 
	{
		while ( z[k+1] < q )
		{
			k++;
		}
		d[q] = square(q-v[k]) + f[v[k]];
		tmpInd[q] = ind[v[k]];
	}
	memcpy(ind,tmpInd,sizeof(int)*n);

	delete [] tmpInd;
	delete [] v;
	delete [] z;
	return d;
}



// dt of 2d function using squared distance
void DistanceTransform::Update2DDTCost(Image<float> *output)
{
	int imageWidth = output->width();
	int imageHeight = output->height();
	float *f = new float[ max( imageWidth, imageHeight ) ];
	int *ind = NULL;


	// transform along columns
	for (int x = 0; x < imageWidth; x++) 
	{
		for (int y = 0; y < imageHeight; y++) 
		{
			f[y] = imRef(output,x,y);
		}
		float *d = Update1DDTCost(f, imageHeight);
		for (int y = 0; y < imageHeight; y++) 
		{
			//cvSetReal2D( output, y, x, d[y]);
			imRef(output,x,y) = d[y];
		}
		delete [] d;
	}
	// transform along rows
	for (int y=0; y<imageHeight; y++) 
	{
		for( int x = 0; x<imageWidth; x++) 
		{
			//f[ x ] = (double)cvGetReal2D( output, y, x);
			f[ x ] = imRef(output,x,y);
		}
		// f保存着通过列遍历得到的最小距离的平方。通过再对行遍历，就可以得到点到最近边的距离了。
		float *d = Update1DDTCost(f, imageWidth);
		for (int x = 0; x < imageWidth; x++) 
		{
			//cvSetReal2D( output, y, x, d[x]);
			imRef( output, x, y) = d[x];
		}
		delete [] d;
	}

	delete [] f;

}

// 计算一维点到最近线点的最近距离
float* DistanceTransform::Update1DDTCost(float *f, const int n)
{
	// 真实输出
	float *d = new float [n];
	int *v = new int[n];
	float *z = new float[n+1];

	int k = 0;
	v[0] = 0;
	z[0] = -INF;
	z[1] = +INF;

	for (int q=1; q<=n-1; q++) 
	{
		// f[q]表示当前循环的像素上的值，只有0或INF。v[k]表示另外的索引，且一直小于q。
		// f[q]和f[v[k]]若都为INF则两者会抵消。f[]不贡献距离值的计算，只用来判定。
		// 
		float s = ((f[q]+square(q)) - (f[v[k]]+square(v[k]))) / (2*q - 2*v[k]); 
		while ( s <= z[k] ) 
		{
			k--;
			s = ((f[q]+square(q)) - (f[v[k]]+square(v[k]))) / (2*q - 2*v[k]);
		}
		k++;
		v[k] = q;
		z[k] = s;
		z[k+1] = INF;

	}

	k = 0;
	for (int q=0; q<=n-1; q++) 
	{
		while ( z[k+1] < q )
		{
			k++;
		}
		// 计算距离的平方
		d[q] = square(q-v[k]) + f[v[k]];

	}

	delete [] v;
	delete [] z;
	return d;
}

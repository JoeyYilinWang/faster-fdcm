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

#include "LineFitter.h"


LFLineFitter::LFLineFitter()
{
	localWindSize_ = 200; //局部滑窗尺寸
	smallLocalWindowSize_ = max(localWindSize_ / 2, 5); 
	nMinEdges_ = 50;
	nMaxWindPoints_ = 4 * (localWindSize_ + 1) * (localWindSize_ + 1); //最大窗口点的数量，默认为4*51*51
	minLength_ = 30;

	nLinesToFitInStage_[0] = 300;
	nLinesToFitInStage_[1] = 3000;
	nTrialsPerLineInStage_[0] = 300;
	nTrialsPerLineInStage_[1] = 1;
	sigmaFitALine_ = 0.75;
	sigmaFindSupport_ = 0.75;
	maxGap_ = 1.5;

	outEdgeMap_ = NULL;
	rpoints_ = NULL;
	rProjection_ = NULL;
	absRProjection_ = NULL; // with reprojection 
	idx_ = NULL;
}

// 直线段拟合参数配置
void LFLineFitter::Configure(double sigmaFitALine, double sigmaFindSupport, double maxGap, int nLayer, int *nLinesToFitInStage, int *nTrialsPerLineInStage)
{
	sigmaFitALine_ = sigmaFitALine;
	sigmaFindSupport_ = sigmaFindSupport;	maxGap_ = maxGap;
	for (int i = 0; i < nLayer; i++)
	{
		nLinesToFitInStage_[i] = nLinesToFitInStage[i];
		nTrialsPerLineInStage_[i] = nTrialsPerLineInStage[i];
	}
}

LFLineFitter::~LFLineFitter()
{
	SafeRelease();
}

void LFLineFitter::SafeRelease()
{
	if (outEdgeMap_)
		delete[] outEdgeMap_;
	if (rpoints_)
		delete[] rpoints_;
	if (rProjection_)
		delete[] rProjection_;
	if (absRProjection_)
		delete[] absRProjection_;
	if (idx_)
		delete[] idx_;
	outEdgeMap_ = NULL;
	rpoints_ = NULL;
	rProjection_ = NULL;
	absRProjection_ = NULL;
	idx_ = NULL;
}

// 直线段拟合类对象初始化
void LFLineFitter::Init()
{
	outEdgeMap_ = new LFLineSegment[nLinesToFitInStage_[0] + nLinesToFitInStage_[1]]; //一共300+3000=3300个直线段
	rpoints_ = new point<int>[nMaxWindPoints_];
	rProjection_ = new double[nMaxWindPoints_];
	absRProjection_ = new double[nMaxWindPoints_];
	idx_ = new int[nMaxWindPoints_];
}

/**
 * @brief 直线段拟合
 * @param inputImage {Image<unsigned char> *}保存边像素的图像,存在边缘的像素值非0，否则为0
 */
void LFLineFitter::FitLine(Image<unsigned char> *inputImage)
{
	// LARGE_INTEGER t1, t2, f;
	// QueryPerformanceFrequency(&f);
	// QueryPerformanceCounter(&t1);

	width_ = inputImage->width();
	height_ = inputImage->height();

	map<int, point<int>> edgeMap; // edgeMap由map储存

	int i, j, k;
	int x0, y0;
	int width, height;
	int index = 0;
	int nPixels = 0;
	int nEdges = 0; // 表示边缘像素点个数
	int maxSupport = 0;
	LFLineSegment tmpLs, bestLs; //全称为temporary lines与best lines
	point<double> lnormal;
	int nWindPoints = 0, nWaitingKillingList = 0, nProposedKillingList = 0;
	point<int> *windPoints, *waitingKillingList, *proposedKillingList;
	windPoints = new point<int>[nMaxWindPoints_];
	waitingKillingList = new point<int>[nMaxWindPoints_];
	proposedKillingList = new point<int>[nMaxWindPoints_];

	width = inputImage->width();
	height = inputImage->height();
	nPixels = width * height;

	// edgeMap保存边缘像素点，nEdges统计所有边缘像素点的数量
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			i = x + y * width;
			// if(cvGetReal1D(inputImage,i)!=0)
			// 被边缘覆盖的部分的像素位置上的值是不为0的
			if (imRef(inputImage, x, y) != 0)
			{
				// edgeMap利用Map数据结构保存了所有输入图像的所有边缘点信息，每个元素都是一个pair结构，key为经换算得到的图像像素位置，value为图像像素位置
				edgeMap.insert(pair<int, point<int>>(i, point<int>(x, y))); // i并不是序数，而是图像上的像素位置被换算罢了
				nEdges++;
			}
		}
	}


	nInputEdges_ = nEdges;
	nLineSegments_ = 0;

	// 分为2个阶段去拟合直线段
	for (k = 0; k < 2; k++)
	{
		// 这种情况一般不会出现，因为输入图像的边缘像素点个数不可能小于5
		if (nEdges < nMinEdges_)
			break;
		// 不同阶段拟合的直线段数量不同
		// 一共两个阶段，第一阶段拟合300条，第二阶段拟合3000条
		for (i = 0; i < nLinesToFitInStage_[k]; i++)
		{
			maxSupport = 0;
			// 寻找一条拟合最好的直线段，每个阶段拟合次数不同，第一阶段是300次，第二阶段是1次
			// 之所以这么设置，是因为大部分直线段都需要在第一阶段准确拟合，第二阶段之所以仅需要一次是因为剩余边缘点不足，过多的拟合次数会造成浪费。
			for (j = 0; j < nTrialsPerLineInStage_[k]; j++)
			{
				// Sample a point, 返回被挑选点的像素位置索引，是一个整数， 但实际上该函数只有第一个参数才被用到，后面两个都是没用的
				index = SampleAPixel(&edgeMap, inputImage, nPixels);
				
				// 恢复被选择像素的位置
				y0 = index / width;
				x0 = index - y0 * width;

				// 定位子窗口，这里使用较小的局部窗口大小，该函数的实际目的是取得较小局部窗口中边缘点的相对坐标
				Find(x0, y0, windPoints, nWindPoints, inputImage, smallLocalWindowSize_); //smallLocalWindowSize_比localWindSize_更小一点
				
				// 对较小的局部窗口中的边缘点进行直线段拟合
				FitALine(nWindPoints, windPoints, sigmaFitALine_, lnormal);

				// Locate the subwindow，该函数的实际目的是获取局部窗口中边缘点的相对坐标
				Find(&edgeMap, x0, y0, windPoints, nWindPoints, inputImage, localWindSize_);

				// Find the support
				FindSupport(nWindPoints, windPoints, lnormal, sigmaFindSupport_, maxGap_, tmpLs, proposedKillingList, nProposedKillingList, x0, y0);

				// Check if need to update
				if (tmpLs.nSupport_ > maxSupport)
				{
					maxSupport = tmpLs.nSupport_;
					nWaitingKillingList = nProposedKillingList;
					memcpy(waitingKillingList, proposedKillingList, sizeof(point<int>) * nWaitingKillingList);
					bestLs = tmpLs;
				}
			}

			// 在每一轮的直线拟合之后，需要把相应的点删掉，这里删掉是指将对应像素位置上的值赋为0
			for (j = 0; j < maxSupport; j++)
			{
				// cvSetReal2D(inputImage,waitingKillingList[j].y,waitingKillingList[j].x,0.0);
				imRef(inputImage, waitingKillingList[j].x, waitingKillingList[j].y) = 0;
				edgeMap.erase(waitingKillingList[j].y * width + waitingKillingList[j].x);
			}
			nEdges -= bestLs.nSupport_;
			bestLs.len_ = sqrt((bestLs.sx_ - bestLs.ex_) * (bestLs.sx_ - bestLs.ex_) + (bestLs.sy_ - bestLs.ey_) * (bestLs.sy_ - bestLs.ey_));

			// LFLineFitter的成员变量，类型为LFlineSegment*
			outEdgeMap_[nLineSegments_] = bestLs;
			nLineSegments_++;
			
			// 当丢弃多余点之后，如果剩余点数量小于nMinEdges则退出循环。
			// 其意义在于如果在拟合过程中，剩余点过少了，则后面就没必要拟合了。
			if (nEdges < nMinEdges_)
				break;
		}
	}

	// 将直线段按照长度降序排列
	MMFunctions::Sort(outEdgeMap_, nLineSegments_, 0);
	delete[] windPoints;
	delete[] waitingKillingList;
	delete[] proposedKillingList;
	edgeMap.clear();
	////////QueryPerformanceCounter(&t2);
	// cout<<"[DO] Fit "<<nLineSegments_<<" lines taking "<<setiosflags(ios::fixed)<<setprecision(6)<<(t2.QuadPart - t1.QuadPart)/(1.0*f.QuadPart)<<"seconds"<<endl;
}

// 将EdgeMap保存到文件中
void LFLineFitter::SaveEdgeMap(const char *filename)
{
	FILE *fp;
	MMFunctions::Sort(outEdgeMap_, nLineSegments_, 0);
	fp = fopen(filename, "wt");

	fprintf(fp, "%d %d\n", width_, height_);
	fprintf(fp, "%d\n", nLineSegments_);

	double ratio = 0;
	double count = 0;
	for (int i = 0; i < nLineSegments_; i++)
	{
		count += (double)outEdgeMap_[i].nSupport_;
		// ratio为直线段支持边缘像素点数量占所有边缘点像素点的比例
		ratio = count / nInputEdges_;
		fprintf(fp, "%d %d %d %d\n", (int)outEdgeMap_[i].sx_, (int)outEdgeMap_[i].sy_, (int)outEdgeMap_[i].ex_, (int)outEdgeMap_[i].ey_);
	}
	fclose(fp);
}

// 从文件中读取直线段信息，并赋值给outEdgeMap
void LFLineFitter::LoadEdgeMap(const char *filename)
{
	SafeRelease();
	FILE *fp = NULL;
	fp = fopen(filename, "rt");
	if (fp == NULL)
	{
		cerr << "Cannot read " << filename << endl;
		exit(-1);
	}

	fscanf(fp, "%d %d\n", &width_, &height_);
	fscanf(fp, "%d\n", &nLineSegments_);

	outEdgeMap_ = new LFLineSegment[nLineSegments_];
	for (int i = 0; i < nLineSegments_; i++)
	{
		fscanf(fp, "%lf %lf %lf %lf\n", &outEdgeMap_[i].sx_, &outEdgeMap_[i].sy_, &outEdgeMap_[i].ex_, &outEdgeMap_[i].ey_);
	}
	fclose(fp);
}

// 将直线段信息保存到Image<uchar>对象中
Image<uchar> *LFLineFitter::ComputeOuputLineImage(Image<uchar> *inputImage)
{

	Image<uchar> *debugImage = new Image<uchar>(inputImage->width(), inputImage->height());
	for (int i = 0; i < nLineSegments_; i++)
	{
		ImageDraw<uchar>::Line(debugImage, (int)outEdgeMap_[i].sx_, (int)outEdgeMap_[i].sy_, (int)outEdgeMap_[i].ex_, (int)outEdgeMap_[i].ey_, 255);
	}
	return debugImage;
}

// 将直线段信息保存到Image<uchar>图像中，并将其保存至PGM格式的图像中
void LFLineFitter::DisplayEdgeMap(Image<uchar> *inputImage, const char *outputImageName)
{
	// for debug
	// Image *debugImage = cvCreateImage( cvSize(inputImage->width,inputImage->height), IPL_DEPTH_8U,1);
	// cvZero(debugImage);

	Image<uchar> debugImage(inputImage->width(), inputImage->height());

	for (int i = 0; i < nLineSegments_; i++)
	{
		// cvLine(debugImage,cvPoint((int)outEdgeMap_[i].sx_,(int)outEdgeMap_[i].sy_),cvPoint((int)outEdgeMap_[i].ex_,(int)outEdgeMap_[i].ey_),cvScalar(255));
		ImageDraw<uchar>::Line(&debugImage, (int)outEdgeMap_[i].sx_, (int)outEdgeMap_[i].sy_, (int)outEdgeMap_[i].ex_, (int)outEdgeMap_[i].ey_, 255);
	}

	if (outputImageName != NULL)
	{
		printf("Save Image %s\n\n", outputImageName);
		// cvSaveImage(outputImageName,debugImage);
		ImageIO::SavePGM(&debugImage, outputImageName);
	}
	// else
	//{
	//	cvNamedWindow("debug",1);
	//	cvShowImage("debug",debugImage);
	//	cvWaitKey(0);
	// }
	// cvReleaseImage(&debugImage);
}


/**
 * @brief 根据smallLocalWindow计算得到的lnormal找寻其获得最大支持的
 * @param nWindPoints 局部窗口上中所有边缘点的数量
 * @param windPoints 局部窗口中的所有边缘点
 * @param lnormal 通过smalllocalwindow计算出的法向量
 * @param sigmaFindSupport 形成直线段的最大支持
 * @param maxGap 保持直线段连续性的最大Gap
 * @param ls {output, LFLineSegment &}
 * @param proposedKillingList 将被丢弃的边缘点
 * @param nProposedKillingList 将被丢弃边缘点的个数
 * @param x0 局部窗口左上角像素在整张图像的像素横坐标
 * @param y0 局部窗口左上角像素在整张图像的像素纵坐标
 */
void LFLineFitter::FindSupport(const int nWindPoints, point<int> *windPoints, point<double> &lnormal,
							   double sigmaFindSupport, double maxGap, LFLineSegment &ls,
							   point<int> *proposedKillingList, int &nProposedKillingList, int x0, int y0)
{
	int i, j;
	int nRindices = 0;
	int zeroIndex = 0;
	double residuals;
	point<double> ldirection; // line direction 

	// Find the point within the threshold by taking dot product
	for (i = 0; i < nWindPoints; i++)
	{
		// lnormal是通过smallLocalWindow内部的边缘点计算出来的
		// 计算LocalWindow中所有边缘点与通过lnormal的内积，其值越小则表示以x0，y0为起点，当前边缘点为终点的直线段与lnormal越垂直
		// 因为LocalWindow与small都是以x0,y0为原点建立局部坐标系，所以这样计算残差是合理且正确的
		residuals = abs(windPoints[i].x * lnormal.x + windPoints[i].y * lnormal.y);
		if (residuals < sigmaFindSupport)
		{
			// rpoints保存着复合上述条件的边缘点
			rpoints_[nRindices] = windPoints[i];
			nRindices++;
		}
	}

	ldirection.x = -lnormal.y; //change back to line direction
	ldirection.y = lnormal.x;

	// minLength设置为2，算很小的阈值了
	if (nRindices < minLength_) // if the support number of points is less than minLenth_, 
	{
		ls.nSupport_ = -1; // then give up and return. 
		return;
	}

	// Project to the line
	for (i = 0; i < nRindices; i++)
	{
		// 将supporting points向方向向量投影，得到距离
		rProjection_[i] = rpoints_[i].x * ldirection.x + rpoints_[i].y * ldirection.y; 
		idx_[i] = i; 
	}

	// 根据投影距离对rProjection、idx_里的元素升序排列
	MMFunctions::ISort(rProjection_, nRindices, idx_);

	for (i = 0; i < nRindices; i++)
		absRProjection_[i] = abs(rProjection_[i]);

	for (i = 0; i < nRindices; i++)
	{
		// 只有方向向量与被检测边缘点形成向量垂直或者被检测边缘点形成向量是0向量时才会出现此状况。
		// 但第一种情形并不会出现，因为前面已经通过法向量滤除大部分边缘点了，现在留下的都是方向一致的边缘点
		// 只能是第二种情形才出现
		if (absRProjection_[i] == 0)
		{
			zeroIndex = i;
			break;
		}
	}

	// 向右拟合
	int maxIndex = nRindices - 1;
	for (i = zeroIndex; i < (nRindices - 1); i++)
	{
		// 这个保证要被拟合成直线段边缘点的连续性
		if ((rProjection_[i + 1] - rProjection_[i]) > maxGap)
		{
			maxIndex = i;
			break;
		}
	}

	// 向左拟合
	int minIndex = 0;
	for (i = zeroIndex; i > 0; i--)
	{
		if ((rProjection_[i] - rProjection_[i - 1]) > maxGap)
		{
			minIndex = i;
			break;
		}
	}

	// 计算出根据lnormal拟合的边缘点数量作为直线段拟合的最大支持
	ls.nSupport_ = maxIndex - minIndex + 1;
	// 以最左侧点和最右侧点分别作为该直线段的起点和终点，并恢复其在inputImage上的绝对坐标
	ls.sx_ = (double)rpoints_[idx_[minIndex]].x + x0;
	ls.sy_ = (double)rpoints_[idx_[minIndex]].y + y0;
	ls.ex_ = (double)rpoints_[idx_[maxIndex]].x + x0;
	ls.ey_ = (double)rpoints_[idx_[maxIndex]].y + y0;

	j = 0;
	// 由于已经通过起点和终点定义直线段了，那么中间的边缘点就可以被丢弃了
	for (i = minIndex; i <= maxIndex; i++)
	{	
		proposedKillingList[j].x = rpoints_[idx_[i]].x + x0;
		proposedKillingList[j].y = rpoints_[idx_[i]].y + y0;
		j++;
	}

	nProposedKillingList = j;

	ls.normal_ = lnormal;
}

/**
 * @brief Fit a line in using RANSAC algorithm
 * @param WindPoints edge points in small window 
 * @param nWindPoints the number of edge points in small window 
 * @param sigmaFitALine
 * @param lnormal the normal of line 
 * @return  the score of fit best line 
 */
int LFLineFitter::FitALine(const int nWindPoints, point<int> *windPoints, const double sigmaFitALine, point<double> &lnormal)
{
	// RANSAC algorithm parameter
	double inlierRatio = 0.9;
	double outlierRatio = 0.9;
	double gamma = 0.05;

	// RANSAC的每一轮尝试次数都是计算出来的
	int nMaxTry = 29; // ceil(log(0.05)/log(0.9))


	int i = 0, j = 0, index = 0;
	int cscore;
	double tmpScore;
	double norm;
	int bestscore = -1;
	point<double> cdirection, cnormal;

	while (i < nMaxTry) //使用ransac算法进行直线段拟合
	{
		index = (int)floor(rand() / (double)RAND_MAX * (double)nWindPoints); // radomly pick one point every loop 
		norm = sqrt(1.0 * windPoints[index].x * windPoints[index].x + windPoints[index].y * windPoints[index].y); 

		if (norm > 0)
		{
			cdirection.x = windPoints[index].x / norm; // compute the cosin of the angle between the hypotenuse constructed by edge point and corresponding x axis 
			cdirection.y = windPoints[index].y / norm; // compute the sin of the angle between the hypotenuse constructed by edge point and corresponding y axis

			cnormal.x = -cdirection.y; // Acoording the othogonal property of vectors, we can get the normal vector of this line which is made up by origin and current point in local window
			cnormal.y = cdirection.x; 

			cscore = 0;

			// 对局部窗口中的所有边缘点进行遍历，计算内点的数量
			for (j = 0; j < nWindPoints; j++) // loop on entire edge point in local window 
			{
				//tmpScore = abs(winPoints[j].x * (-windPoint[index].y / norm) + windPoints[j].y * windPoint[index].x / norm)	
				// 这里使用利用向量点乘判断两向量是否垂直的思想
				tmpScore = abs(windPoints[j].x * cnormal.x + windPoints[j].y * cnormal.y);
				if (tmpScore < sigmaFitALine)
					cscore++; // inliner points counter
			}

			// if the ratio of inliners is larger than inlinerRatio
			if (((double)cscore) / nWindPoints > inlierRatio)
			{
				bestscore = cscore; 
				lnormal = cnormal;
				return bestscore;
			}

			if ((1.0 - ((double)cscore) / nWindPoints) < outlierRatio) // if the ratio of outliers is less than theshold, then 
			{
				outlierRatio = 1.0 - ((double)cscore) / nWindPoints; // then make outlinerRation smaller
				nMaxTry = (int)ceil(log(gamma) / log(outlierRatio)); // and then make nMaxTry bigger
			}
			
			if (cscore > bestscore)
			{
				bestscore = cscore;
				lnormal = cnormal;
			}
		}
		i = i + 1;
	}

	return bestscore;
}

// 该函数还有一个重载版本，但功能与该版本完全相同。
// 唯一不同在于遍历方式不同，该版本是对窗口内的所有像素位置进行遍历，根据遍历像素位置上的值来判断是否是边缘点，满足边缘点条件后再进行处理
// 而被重载的版本则是直接使用edgeMap进行遍历，因此并不需要判断。
void LFLineFitter::Find(int x0, int y0, point<int> *windPoints, int &nWindPoints, Image<unsigned char> *inputImage, int localWindSize)
{
	int x, y;
	nWindPoints = 0;

	// 在一个小窗口内循环
	for (y = max(y0 - localWindSize, 0); y < min(y0 + localWindSize, inputImage->height()); y++)
		for (x = max(x0 - localWindSize, 0); x < min(x0 + localWindSize, inputImage->width()); x++)
		{
			// if(cvGetReal2D(inputImage,y,x)!=0)
			// 这里再次判断该像素点是否是边缘点，实际上该函数被调用之前，调用函数已经挑选出边缘点了。
			if (imRef(inputImage, x, y) != 0)
			{
				// windPoints中保存的像素位置是一个相对位置，原点是local window的左上方
				windPoints[nWindPoints].x = x - x0;
				windPoints[nWindPoints].y = y - y0;
				nWindPoints++;
			}
		}
}

/**
 * @brief Get the coordinates of edge pixels in local frame of window 
 * @param {map<int, Point<int>>} *edgeMap 
 * @param {int} x0
 * @param {int} y0
 * @param {Point<int>} *windPoints
 * @param {int} &nWindPoints
 * @param {Image<unsigned char>} *inputImage
 * @param {int} localWindSize
 * @return {*}
 */
void LFLineFitter::Find(map<int, point<int>> *edgeMap, int x0, int y0, point<int> *windPoints, int &nWindPoints, Image<unsigned char> *inputImage, int localWindSize)
{
	// 重新赋值
	nWindPoints = 0;
	map<int, point<int>>::iterator it;

	int left = max(x0 - localWindSize, 0);
	int right = min(x0 + localWindSize, inputImage->width());
	int top = max(y0 - localWindSize, 0);
	int bottom = min(y0 + localWindSize, inputImage->height());

	for (it = edgeMap->begin(); it != edgeMap->end(); ++it)
	{
		if (it->second.x > left && it->second.x < right &&
			it->second.y > top && it->second.y < bottom)
		{
			windPoints[nWindPoints].x = it->second.x - x0; //get coordinates of edge pixel in frame of local window by substracting x0 and y0
			windPoints[nWindPoints].y = it->second.y - y0; 
			nWindPoints++;
		}
	}
}

/**
 * @brief Randomly sample one point of edgeMap.
 * @param {map<int,Point<int> >} *edgeMap
 * @param {Image<unsigned char>} *inputImage
 * @param {int} nPixels
 * @return {int} index of sampled point
 */
int LFLineFitter::SampleAPixel(map<int, point<int>> *edgeMap, Image<unsigned char> *inputImage, int nPixels)
{
	int index = (int)(floor(rand() / (double)RAND_MAX * (double)(edgeMap->size() - 1)));
	map<int, point<int>>::iterator it;
	it = edgeMap->begin();
	for (int i = 0; i < index; i++)
		it++;
	return (*it).first;
}

/**
 * @brief 从配置文件中读取LFLineFitter对象配置
 * @param filename {const char *}配置文件名
 */
void LFLineFitter::Configure(const char *filename)
{
	ifstream file;
	file.open(filename);
	string line;
	string column(":");
	size_t found;

	if (!file.is_open())
	{
		cerr << "Cannot open file " << filename << endl;
		exit(-1);
	}
	
	while (getline(file, line))
	{
		if (string::npos != line.find("SIGMA_FIT_A_LINE"))
		{
			found = line.find(column);
			// substring可能包含空格，但并不影响atof函数的使用，因为atof函数会跳过空格及任意非数字的字符
			string substring = line.substr(found + 1, line.length() - found).c_str();
			sigmaFitALine_ = atof(substring.c_str()); //把字符串转化为浮点数
		}
		else if (string::npos != line.find("SIGMA_FIND_SUPPORT"))
		{
			found = line.find(column);
			string substring = line.substr(found + 1, line.length() - found).c_str();
			sigmaFindSupport_ = atof(substring.c_str());
		}
		else if (string::npos != line.find("MAX_GAP"))
		{
			found = line.find(column);
			string substring = line.substr(found + 1, line.length() - found).c_str();
			maxGap_ = atof(substring.c_str());
		}
		else if (string::npos != line.find("N_LINES_TO_FIT_IN_STAGE_1"))
		{
			found = line.find(column);
			string substring = line.substr(found + 1, line.length() - found).c_str();
			nLinesToFitInStage_[0] = atoi(substring.c_str());
		}
		else if (string::npos != line.find("N_TRIALS_PER_LINE_IN_STAGE_1"))
		{
			found = line.find(column);
			string substring = line.substr(found + 1, line.length() - found).c_str();
			nTrialsPerLineInStage_[0] = atoi(substring.c_str());
		}
		else if (string::npos != line.find("N_LINES_TO_FIT_IN_STAGE_2"))
		{
			found = line.find(column);
			string substring = line.substr(found + 1, line.length() - found).c_str();
			nLinesToFitInStage_[1] = atoi(substring.c_str());
		}
		else if (string::npos != line.find("N_TRIALS_PER_LINE_IN_STAGE_2"))
		{
			found = line.find(column);
			string substring = line.substr(found + 1, line.length() - found).c_str();
			nTrialsPerLineInStage_[1] = atoi(substring.c_str());
		}
	}
	file.close();

	PrintParameter();
}

// 打印配置参数
void LFLineFitter::PrintParameter()
{
	cout << "/* ==========================================================" << endl;
	cout << "* LFLineFitting parameters " << endl;
	cout << "* ===========================================================" << endl;
	cout << "* SIGMA_FIT_A_LINE:"
		 << "\t" << sigmaFitALine_ << endl;
	cout << "* SIGMA_FIND_SUPPORT:"
		 << "\t" << sigmaFindSupport_ << endl;
	cout << "* MAX_GAP:"
		 << "\t\t" << maxGap_ << endl;
	cout << "* N_LINES_TO_FIT_IN_STAGE_1:"
		 << "\t" << nLinesToFitInStage_[0] << endl;
	cout << "* N_TRIALS_PER_LINE_IN_STAGE_1:"
		 << "\t" << nTrialsPerLineInStage_[0] << endl;
	cout << "* N_LINES_TO_FIT_IN_STAGE_2:"
		 << "\t" << nLinesToFitInStage_[1] << endl;
	cout << "* N_TRIALS_PER_LINE_IN_STAGE_2:"
		 << "\t" << nTrialsPerLineInStage_[1] << endl;
	cout << "* ===========================================================" << endl;
	cout << "*/" << endl;
}
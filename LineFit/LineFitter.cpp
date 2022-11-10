#include "LineFitter.h"

LFLineFitter::LFLineFitter()
{
	localWindSize_ = 50; //using local window size
	smallLocalWindowSize_ = max(localWindSize_ / 10, 5); // why should we define a small local window size?
	nMinEdges_ = 5;
	nMaxWindPoints_ = 4 * (localWindSize_ + 1) * (localWindSize_ + 1); //maximum num of points，默认为4*51*51
	minLength_ = 2;

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

void LFLineFitter::Configure(double sigmaFitALine, double sigmaFindSupport, double maxGap, int nLayer, int *nLinesToFitInStage, int *nTrialsPerLineInStage)
{
	sigmaFitALine_ = sigmaFitALine;
	sigmaFindSupport_ = sigmaFindSupport;
	maxGap_ = maxGap;
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

void LFLineFitter::Init()
{
	outEdgeMap_ = new LFLineSegment[nLinesToFitInStage_[0] + nLinesToFitInStage_[1]]; //一共300+3000=3300个直线段
	rpoints_ = new Point<int>[nMaxWindPoints_];
	rProjection_ = new double[nMaxWindPoints_];
	absRProjection_ = new double[nMaxWindPoints_];
	idx_ = new int[nMaxWindPoints_];
}

/**
 * @description: According
 * @param {Image<unsigned char>} *inputImage
 * @return {*}
 */
void LFLineFitter::FitLine(Image<unsigned char> *inputImage)
{
	// LARGE_INTEGER t1, t2, f;
	// QueryPerformanceFrequency(&f);
	// QueryPerformanceCounter(&t1);

	width_ = inputImage->width();
	height_ = inputImage->height();

	map<int, Point<int>> edgeMap; // edgeMap由map储存

	int i, j, k;
	int x0, y0;
	int width, height;
	int index = 0;
	int nPixels = 0;
	int nEdges = 0;
	int maxSupport = 0;
	LFLineSegment tmpLs, bestLs; //全称为temporary lines与best lines
	Point<double> lnormal;
	int nWindPoints = 0, nWaitingKillingList = 0, nProposedKillingList = 0;
	Point<int> *windPoints, *waitingKillingList, *proposedKillingList;
	windPoints = new Point<int>[nMaxWindPoints_];
	waitingKillingList = new Point<int>[nMaxWindPoints_];
	proposedKillingList = new Point<int>[nMaxWindPoints_];

	width = inputImage->width();
	height = inputImage->height();
	nPixels = width * height;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			i = x + y * width;
			// if(cvGetReal1D(inputImage,i)!=0)
			if (imRef(inputImage, x, y) != 0)
			{
				edgeMap.insert(pair<int, Point<int>>(i, Point<int>(x, y))); // i并不是序数，而是图像上的像素位置被换算罢了
				nEdges++;
			}
		}
	}


	nInputEdges_ = nEdges;
	nLineSegments_ = 0;
	for (k = 0; k < 2; k++)
	{
		if (nEdges < nMinEdges_)
			break;
		for (i = 0; i < nLinesToFitInStage_[k]; i++)
		{
			maxSupport = 0;
			for (j = 0; j < nTrialsPerLineInStage_[k]; j++)
			{
				// Sample a point
				index = SampleAPixel(&edgeMap, inputImage, nPixels);
				y0 = index / width;
				x0 = index - y0 * width;

				// Locate the subwindow
				Find(x0, y0, windPoints, nWindPoints, inputImage, smallLocalWindowSize_); //smallLocalWindowSize_比localWindSize_更小一点
				
				// Infer a line direction
				FitALine(nWindPoints, windPoints, sigmaFitALine_, lnormal);

				// Locate the subwindow
				Find(&edgeMap, x0, y0, windPoints, nWindPoints, inputImage, localWindSize_);

				// Find the support
				FindSupport(nWindPoints, windPoints, lnormal, sigmaFindSupport_, maxGap_, tmpLs, proposedKillingList, nProposedKillingList, x0, y0);

				// Check if need to update
				if (tmpLs.nSupport_ > maxSupport)
				{
					maxSupport = tmpLs.nSupport_;
					nWaitingKillingList = nProposedKillingList;
					memcpy(waitingKillingList, proposedKillingList, sizeof(Point<int>) * nWaitingKillingList);
					bestLs = tmpLs;
				}
			}

			// Remove points
			for (j = 0; j < maxSupport; j++)
			{
				// cvSetReal2D(inputImage,waitingKillingList[j].y,waitingKillingList[j].x,0.0);
				imRef(inputImage, waitingKillingList[j].x, waitingKillingList[j].y) = 0;
				edgeMap.erase(waitingKillingList[j].y * width + waitingKillingList[j].x);
			}
			nEdges -= bestLs.nSupport_;
			bestLs.len_ = sqrt((bestLs.sx_ - bestLs.ex_) * (bestLs.sx_ - bestLs.ex_) + (bestLs.sy_ - bestLs.ey_) * (bestLs.sy_ - bestLs.ey_));
			outEdgeMap_[nLineSegments_] = bestLs;
			nLineSegments_++;

			if (nEdges < nMinEdges_)
				break;
		}
	}
	MMFunctions::Sort(outEdgeMap_, nLineSegments_, 0);
	delete[] windPoints;
	delete[] waitingKillingList;
	delete[] proposedKillingList;
	edgeMap.clear();
	////////QueryPerformanceCounter(&t2);
	// cout<<"[DO] Fit "<<nLineSegments_<<" lines taking "<<setiosflags(ios::fixed)<<setprecision(6)<<(t2.QuadPart - t1.QuadPart)/(1.0*f.QuadPart)<<"seconds"<<endl;
}

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
		ratio = count / nInputEdges_;
		fprintf(fp, "%d %d %d %d\n", (int)outEdgeMap_[i].sx_, (int)outEdgeMap_[i].sy_, (int)outEdgeMap_[i].ex_, (int)outEdgeMap_[i].ey_);
	}
	fclose(fp);
}

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

Image<uchar> *LFLineFitter::ComputeOuputLineImage(Image<uchar> *inputImage)
{

	Image<uchar> *debugImage = new Image<uchar>(inputImage->width(), inputImage->height());
	for (int i = 0; i < nLineSegments_; i++)
	{
		ImageDraw<uchar>::Line(debugImage, (int)outEdgeMap_[i].sx_, (int)outEdgeMap_[i].sy_, (int)outEdgeMap_[i].ex_, (int)outEdgeMap_[i].ey_, 255);
	}
	return debugImage;
}

void LFLineFitter::DisplayEdgeMap(Image<uchar> *inputImage, const char *outputImageName)
{
	// for debug
	// Image *debugImage = cvCreateImage( cvSize(inputImage->width,inputImage->height), IPL_DEPTH_8U,1);
	// cvZero(debugImage);

	Image<uchar> debugImage(inputImage->width(), inputImage->height());
	;

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


void LFLineFitter::FindSupport(const int nWindPoints, Point<int> *windPoints, Point<double> &lnormal,
							   double sigmaFindSupport, double maxGap, LFLineSegment &ls,
							   Point<int> *proposedKillingList, int &nProposedKillingList, int x0, int y0)
{
	int i, j;
	int nRindices = 0;
	int zeroIndex = 0;
	double residuals;
	Point<double> ldirection; // line direction 

	// Find the point within the threshold by taking dot product
	for (i = 0; i < nWindPoints; i++)
	{
		residuals = abs(windPoints[i].x * lnormal.x + windPoints[i].y * lnormal.y);
		if (residuals < sigmaFindSupport)
		{
			rpoints_[nRindices] = windPoints[i];
			nRindices++;
		}
	}
	ldirection.x = -lnormal.y; //change back to line direction
	ldirection.y = lnormal.x;

	if (nRindices < minLength_) // if the support number of points is less than minLenth_, 
	{
		ls.nSupport_ = -1; // then give up and return. 
		return;
	}

	// Project to the line
	for (i = 0; i < nRindices; i++)
	{
		rProjection_[i] = rpoints_[i].x * ldirection.x + rpoints_[i].y * ldirection.y; // project the supporting points onto line
		idx_[i] = i;
	}

	// Sort the projection and find the starting and ending points
	MMFunctions::ISort(rProjection_, nRindices, idx_);

	for (i = 0; i < nRindices; i++)
		absRProjection_[i] = abs(rProjection_[i]);

	for (i = 0; i < nRindices; i++)
	{
		if (absRProjection_[i] == 0)
		{
			zeroIndex = i;
			break;
		}
	}

	int maxIndex = nRindices - 1;
	for (i = zeroIndex; i < (nRindices - 1); i++)
	{
		if ((rProjection_[i + 1] - rProjection_[i]) > maxGap)
		{
			maxIndex = i;
			break;
		}
	}

	int minIndex = 0;
	for (i = zeroIndex; i > 0; i--)
	{
		if ((rProjection_[i] - rProjection_[i - 1]) > maxGap)
		{
			minIndex = i;
			break;
		}
	}

	ls.nSupport_ = maxIndex - minIndex + 1;
	ls.sx_ = (double)rpoints_[idx_[minIndex]].x + x0;
	ls.sy_ = (double)rpoints_[idx_[minIndex]].y + y0;
	ls.ex_ = (double)rpoints_[idx_[maxIndex]].x + x0;
	ls.ey_ = (double)rpoints_[idx_[maxIndex]].y + y0;

	j = 0;
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
 * @param lnormal line normal
 * @return the score of fit best line 
 */
int LFLineFitter::FitALine(const int nWindPoints, Point<int> *windPoints, const double sigmaFitALine, Point<double> &lnormal)
{
	double inlierRatio = 0.9;
	double outlierRatio = 0.9;
	double gamma = 0.05;
	int nMaxTry = 29; // ceil(log(0.05)/log(0.9))

	int i = 0, j = 0, index = 0;
	int cscore;
	double tmpScore;
	double norm;
	int bestscore = -1;
	Point<double> cdirection, cnormal;

	while (i < nMaxTry) // using ransac algorithm to fit a line.
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
			for (j = 0; j < nWindPoints; j++) // loop on entire edge point in local window 
			{
				//tmpScore = abs(winPoints[j].x * (-windPoint[index].y / norm) + windPoints[j].y * windPoint[index].x / norm)	
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

/**
 * @brief find the best line based 
 */
void LFLineFitter::Find(int x0, int y0, Point<int> *windPoints, int &nWindPoints, Image<unsigned char> *inputImage, int localWindSize)
{
	int x, y;
	nWindPoints = 0;

	for (y = max(y0 - localWindSize, 0); y < min(y0 + localWindSize, inputImage->height()); y++)
		for (x = max(x0 - localWindSize, 0); x < min(x0 + localWindSize, inputImage->width()); x++)
		{
			// if(cvGetReal2D(inputImage,y,x)!=0)
			if (imRef(inputImage, x, y) != 0)
			{
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
void LFLineFitter::Find(map<int, Point<int>> *edgeMap, int x0, int y0, Point<int> *windPoints, int &nWindPoints, Image<unsigned char> *inputImage, int localWindSize)
{
	nWindPoints = 0;
	map<int, Point<int>>::iterator it;

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
 * @description: Randomly sample one point of edgeMap.
 * @param {map<int,Point<int> >} *edgeMap
 * @param {Image<unsigned char>} *inputImage
 * @param {int} nPixels
 * @return {int} index of sampled point
 */
int LFLineFitter::SampleAPixel(map<int, Point<int>> *edgeMap, Image<unsigned char> *inputImage, int nPixels)
{
	int index = (int)(floor(rand() / (double)RAND_MAX * (double)(edgeMap->size() - 1)));
	map<int, Point<int>>::iterator it;
	it = edgeMap->begin();
	for (int i = 0; i < index; i++)
		it++;
	return (*it).first;
}

/**
 * @description: Initialize member variables of LFLineFitter object in using file
 * @param {char} *filename
 * @return {*}
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
			string substring = line.substr(found + 1, line.length() - found).c_str();
			sigmaFitALine_ = atof(substring.c_str()); //change string to float
		}
		else if (string::npos != line.find("SIGMA_FIND_SUPPORT"))
		{
			found = line.find(column);
			string substring = line.substr(found + 1, line.length() - found).c_str();
			sigmaFindSupport_ = atof(substring.c_str());
		}
		else if (string::npos != line.find("MAX_GAP:"))
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
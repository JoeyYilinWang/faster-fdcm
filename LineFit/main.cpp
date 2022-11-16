#include <iostream>
#include <string>
#include "Image/Image.h"
#include "Image/ImageIO.h"
#include "LineFitter.h"

using namespace std;

// fitline API using, just using not need to analyse
void main(int argc, char *argv[])
{

	Image<uchar> *inputImage=NULL; // initialize a input image

	// define line fitter
	LFLineFitter lf;
	
	if(argc != 4)
	{
		// how to use .pgm form image? how about png form image?
		std::cerr<<"[Syntax] fitline   input_edgeMap.pgm   output_line.txt   output_edgeMap.pgm"<<std::endl;
		exit(0);
	}

	string imageName(argv[1]);
	string outFilename(argv[2]);
	string outputImageName(argv[3]);
	
	//string imageName("data/hat_edges.pgm");
	//string outFilename("data/hat_edges.txt");
	//string outputImageName("data/hat_edges_display.pgm");

	// 使用PGM格式，PGM全称为Portable Grey Map，最终目的是将其转化为Image<uchar>格式的数据
	inputImage = ImageIO::LoadPGM(imageName.c_str());
	if(inputImage==NULL)
	{
		std::cerr<<"[ERROR] Fail in reading image "<<imageName<<std::endl;
		exit(0);
	}
	
	lf.Init(); // line fit
	
	// 配置文件读取，但该文件在哪？
	lf.Configure("para_template_line_fitter.txt");
	
	// 对输入图像进行边线拟合
	lf.FitLine(inputImage); 
    
	// 将被拟合出来的直线段进行图像化显示
	lf.DisplayEdgeMap(inputImage,outputImageName.c_str());	

	// 拟合成功的直线用一个.txt文件便可保存
	lf.SaveEdgeMap(outFilename.c_str());

	//cvReleaseImage(&inputImage);
	delete inputImage;
};
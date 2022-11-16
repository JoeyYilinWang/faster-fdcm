#include "Image/Image.h"
#include "Image/ImageIO.h"
#include "LineFit/LineFitter.h"
#include "LMLineMatcher.h"

#include <iostream>
#include <string>


void main(int argc, char *argv[])
{
	
	//if(argc < 4)
	//{
	//	//std::cerr<<"[Syntax] fdcm template.txt input_edgeMap.pgm input_realImage.jpg"<<std::endl;
	//	std::cerr<<"[Syntax] fdcm template.txt input_edgeMap.pgm input_realImage.ppm"<<std::endl;
	//	exit(0);
	//}

	// 定义直线段拟合对象
	LFLineFitter lf;

	// 定义匹配对象
	LMLineMatcher lm;

	// 直线段拟合对象读取直线段拟合配置文件
	lf.Configure("para_line_fitter.txt");
	// line matcher的配置文件
	lm.Configure("para_line_matcher.txt");


	//Image *inputImage=NULL;
	Image<uchar> *inputImage=NULL;

	// 对应于论文中的模板U
	string templateFileName(argv[1]);

	// 对应于论文中的query image edge map V
	string edgeMapName(argv[2]);

	// 对应于匹配成功后被检测目标在原始图像上的显示
	string displayImageName(argv[3]);
	
	// 不同使用方式
	//string templateFileName("Exp_Smoothness/template_list.txt");
	//string edgeMapName("Exp_Smoothness/device5-20_edge_cluttered.pgm");
	//string displayImageName("Exp_Smoothness/device5-20_edge_cluttered.pgm");	

	//string templateFileName("data/template_giraffe.txt");
	//string edgeMapName("data/looking_edges.pgm");
	//string displayImageName("data/looking.ppm");
	
	// 直接将被检测结果覆盖到jpg图像最符合demo要求
	//string templateFileName("data/template_applelogo.txt");
	//string edgeMapName("data/hat_edges.pgm");
	//string displayImageName("data/hat.jpg");

	
	//inputImage = cvLoadImage(edgeMapName.c_str(),0);
	// 使用portable grey map保存灰度图像，该灰度图像是由灰度像素组成的edge map
	inputImage = ImageIO::LoadPGM(edgeMapName.c_str());


	if(inputImage==NULL)
	{
		std::cerr<<"[ERROR] Fail in reading image "<<edgeMapName<<std::endl;
		exit(0);
	}
	
	// 初始化line fitter
	lf.Init();
	
	// line matcher读取模板，要注意的是该模板是一个.txt文件
	lm.Init(templateFileName.c_str());


	// line fitter对query edge map进行拟合，即对V进行直线段拟合
	lf.FitLine(inputImage);

	

	// FDCM Matching
	vector<LMDetWind> detWind;
	lm.Match(lf,detWind);
	//lm.MatchCostMap(lf,outputCostMapName.c_str());
	// Display best matcher in edge map
	if(displayImageName.c_str())
	{
		// 使用了query edge map的原始RGB图像作为结果显示的底板，即使用PPM格式进行显示
		Image<RGBMap> *debugImage = ImageIO::LoadPPM(displayImageName.c_str());

		// 在PPM格式的底板上进行画图，底板尺寸一般比模板尺寸大
		LMDisplay::DrawDetWind(debugImage,detWind[0].x_,detWind[0].y_,detWind[0].width_,detWind[0].height_,RGBMap(0,255,0),4);
		char outputname[256];
		// 将displayImageName名称与.output.ppm组合，并将其赋值给outputname
		sprintf(outputname,"%s.output.ppm",displayImageName.c_str());
		// 将包含检测结果的PPM图像保存，图像名称为outputname
		ImageIO::SavePPM(debugImage,outputname);
		delete debugImage;
	}

	//cvReleaseImage(&inputImage);
	delete inputImage;
};

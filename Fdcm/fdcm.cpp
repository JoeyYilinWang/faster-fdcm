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

#include "Image/Image.h"
#include "Image/ImageIO.h"
#include "LineFit/LineFitter.h"
#include "LMLineMatcher.h"

#include <iostream>
#include <string>

// 匹配测试用例
void main(int argc, char *argv[])
{

	// 验证参数给定正确性
	if(argc < 4)
	{
		//std::cerr<<"[Syntax] fdcm template.txt input_edgeMap.pgm input_realImage.jpg"<<std::endl;
		std::cerr<<"[Syntax] fdcm template.txt input_edgeMap.pgm input_realImage.ppm"<<std::endl;
		exit(0);
	}
	
	// 定义直线段拟合对象
	LFLineFitter lf;

	// 定义匹配对象
	LMLineMatcher lm;

	// 直线段拟合对象读取直线段拟合配置文件，也可以通过该函数的重载版本直接传参
	lf.Configure("para_line_fitter.txt");
	// line matcher的配置文件
	lm.Configure("para_line_matcher.txt");


	Image<uchar> *inputImage=NULL;

	// 对应于论文中的模板U
	string templateFileName(argv[1]);

	// 对应于论文中的query image edge map V
	string edgeMapName(argv[2]);

	// 对应于匹配成功后被检测目标在原始图像上的显示
	string displayImageName(argv[3]); 
	
	// 不同使用方式
	//string templateFileName("Template/template_list.txt");
	//string edgeMapName("Query/query_edges.pgm");
	//string displayImageName("Query/query.pgm");	

	//string templateFileName("Template/template_list.txt");
	//string edgeMapName("Query/query_edges.pgm");
	//string displayImageName("Query/query.ppm");
	
	// 直接将被检测结果覆盖到jpg图像最符合demo要求
	//string templateFileName("Template/template_list.txt");
	//string edgeMapName("Query/query_edges.pgm");
	//string displayImageName("Query/query.jpg");

	
	// 使用portable grey map保存灰度图像，该灰度图像是由灰度像素组成的edge map
	inputImage = ImageIO::LoadPGM(edgeMapName.c_str());


	if(inputImage==NULL)
	{
		std::cerr<<"[ERROR] Fail in reading image "<<edgeMapName<<std::endl;
		exit(0);
	}
	
	// 初始化line fitter
	lf.Init(); 
	
	// line matcher读取模板，要注意的是该模板是一个.txt文件，该文件保存着模板信息
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

	delete inputImage;
};

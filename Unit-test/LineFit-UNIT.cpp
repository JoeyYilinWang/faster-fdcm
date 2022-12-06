#include <iostream>
#include <string>
#include "../Image/Image.h"
#include "../Image/ImageIO.h"
#include "../LineFit/LineFitter.h"

using namespace std;

// 直线段拟合功能单元测试
int main(int argc, char *argv[])
{

	
	Image<uchar> *inputImage=NULL; // initialize a input image

	// define line fitter
	LFLineFitter lf;
	
    string imageNamePGM, transformedImagePGM, outFileName, outImageName;

	if(argc != 4)
	{
		// std::cerr<<"[Syntax] fitline   input_edgeMap.pgm   output_line.txt   output_edgeMap.pgm"<<std::endl;
		// exit(0);
        cout <<"[Syntax] LineFit-UNIT inputImage.pgm  output_line.txt output_edgeMap.pgm"<<std::endl;
        cout << "Switch to default parameters" << endl;
        imageNamePGM = ("/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/queryImages/shanghaiEdge.pgm");
        outFileName = ("/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/queryImages/shanghaiEdge.txt");
        outImageName = ("/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/queryImages/shanghaiEdgeSimplified.pgm");
	}
    else
    {
        imageNamePGM = argv[1];
        outFileName = argv[2];
        outImageName = argv[3];
    }

	inputImage = ImageIO::LoadPGM(imageNamePGM.c_str());
	if(inputImage==NULL)
	{
		std::cerr<<"[ERROR] Fail in reading image "<< imageNamePGM <<std::endl;
		exit(0);
	}
	
	lf.Init(); // line fit
	
	// 配置文件读取
	lf.Configure("../Config/LFlineFitterConfig.txt");
	
	// 对输入图像进行边线拟合
	lf.FitLine(inputImage); 
    
	// 将被拟合出来的直线段进行图像化显示
	lf.DisplayEdgeMap(inputImage,outImageName.c_str());	

	// 拟合成功的直线用一个.txt文件便可保存
	lf.SaveEdgeMap(outFileName.c_str());

	//cvReleaseImage(&inputImage);
	delete inputImage;
};
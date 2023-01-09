#include <iostream>
#include <string>
#include <filesystem>

#include "../Image/Image.h"
#include "../Image/ImageIO.h"
#include "../LineFit/LineFitter.h"

using namespace std;
namespace fs = std::filesystem;

int LineFitSingleImage(const char* inputImagePGM, const char* outputImagePGM, const char* outputTXT);
int LineFitBatch(const char* inputFolderPGM, const char* outputFolderPGM, const char* outputFolderTXT);

// 直线段拟合功能单元测试
int main(int argc, char *argv[])
{

	Image<uchar> *inputImage=NULL; // initialize a input image
	// define line fitter
	LFLineFitter lf;
	
    string imageNamePGMfolder, outFileNamefolder, outImageNamefolder;

	if(argc != 4)
	{
		// std::cerr<<"[Syntax] fitline   input_edgeMap.pgm   output_line.txt   output_edgeMap.pgm"<<std::endl;
		// exit(0);
        cout <<"[Syntax] LineFit-UNIT inputImage.pgm  output_line.txt output_edgeMap.pgm"<<std::endl;
        cout << "Switch to default parameters" << endl;
        imageNamePGMfolder = ("/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/tempImages/上海近崇明岛/PGMs");
        outFileNamefolder= ("/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/tempImages/上海近崇明岛/TXTs");
        outImageNamefolder = ("/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/tempImages/上海近崇明岛/LINEs");
	}
    else
    {
        imageNamePGMfolder = argv[1];
        outFileNamefolder = argv[2];
        outImageNamefolder = argv[3];
    }

	LineFitBatch(imageNamePGMfolder.c_str(), outImageNamefolder.c_str(), outFileNamefolder.c_str());
	return 0;
};

/**
 * @brief 单张图像的直线段拟合
 * @param inputImagePGM 输入图像的路径
 * @param outputImageLinesPGM 输出的线段拟合图像路径
 * @param outputTXT 拟合线段信息文件路径
 */
int LineFitSingleImage(const char* inputImagePGM, const char* outputImageLinesPGM, const char* outputTXT)
{
	string imageNamePGM, outFileName, outImageName;
	imageNamePGM = inputImagePGM;
	outImageName = outputImageLinesPGM;
	outFileName = outputTXT;

	Image<uchar> *inputImage=NULL; // initialize a input image
	LFLineFitter lf;

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
	return 0;

}

/**
 * @brief 批处理直线段拟合
 * @param inputFolderPGM 保存pgm原始图像集的地址
 * @param outputFolderPGM 保存直线段拟合后的pgm图像集合地址
 * @param outputFolderTXT 保存直线段拟合后的直线段信息集合地址
 */
int LineFitBatch(const char* inputFolderPGM, const char* outputFolderPGM, const char* outputFolderTXT)
{
	// 保存template原始pgm图像路径
	vector<string> paths;
	for (const auto & entry : fs::directory_iterator(inputFolderPGM))
	{
		paths.push_back(entry.path());
	}
	for (int i = 0; i < paths.size(); i++)
	{
		string outputPGM, outputTXT;

		stringstream infolderName(paths[i]);
		char split = '/';
		vector<string> results;
		string str;
		
		while (getline(infolderName, str, split))
		{
			results.push_back(str);
		}
		int npos = 0;
		npos = results[results.size()-1].find(".pgm");
		str = str.substr(0, npos);
		
		stringstream sa, sb;
		sa << outputFolderPGM << "/" << str << "Lines" << ".pgm";
		sb << outputFolderTXT << "/" << str << "Lines" << ".txt";
		
		outputPGM = sa.str();
		outputTXT = sb.str();
		LineFitSingleImage(paths[i].c_str(), outputPGM.c_str(), outputTXT.c_str());
	}

	return 0;
}

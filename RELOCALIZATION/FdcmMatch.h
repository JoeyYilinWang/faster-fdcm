#include "../Image/Image.h"
#include "../Image/ImageIO.h"
#include <filesystem>
#include "../LineFit/LineFitter.h"
#include "../Fdcm/LMLineMatcher.h"
#include "../Image/VideoCapture.h"

#include <iostream>
#include <string>
using namespace std;
namespace fs = std::filesystem;

int MatchForSingleTemp(const char* tempName, const char* queryEdgeName, const char* renderImageName, vector<int>& center_point);
int MatchForTempBatch(const char* templatesFolderName, const char* queryEdgeName, const char* renderImageName);
int MatchVideo(const char* videoFileName, const char* queryImagePGM, const char* queryImagePPM);

int MatchForSingleTemp(const char* tempName, const char* queryEdgeTXTName, const char* renderImageName, vector<int>& center_point)
{
    string templateName, queryEdgeMapTXTName, queryImageName;
    templateName = tempName;
    queryEdgeMapTXTName = queryEdgeTXTName;
    queryImageName = renderImageName;

    LFLineFitter lf;

    LMLineMatcher lm;

    lf.Configure("/home/joey/Projects/faster-fdcm/Unit-test/Config/LFlineFitterConfig.txt");
    lm.Configure("/home/joey/Projects/faster-fdcm/Unit-test/Config/LMlineMatcherConfig.txt");

    lf.Init();
    // Image<uchar> *inputImage = ImageIO::LoadPGM(templateName.c_str());
    // lf.FitLine(inputImage);

    lf.LoadEdgeMap(queryEdgeMapTXTName.c_str());
    lm.Init(templateName.c_str());

    vector<LMDetWind> detWind;
	lm.Match(lf,detWind);
    cout << "The Num of Matched successfully windows is: " << detWind.size() << endl;

    if (queryImageName.c_str())
    {
        Image<RGBMap> *debugImage = ImageIO::LoadPPM(queryImageName.c_str());
        LMDisplay::DrawDetWind(debugImage,detWind[0].x_,detWind[0].y_,detWind[0].width_,detWind[0].height_,RGBMap(255,0,0),4);
        center_point = {detWind[0].x_+detWind[0].width_/2, detWind[0].y_+detWind[0].height_/2};
        char outputname[256];
        sprintf(outputname,"%s.output.ppm",queryImageName.c_str());
        ImageIO::SavePPM(debugImage,outputname);
        delete debugImage;
    }

    // delete inputImage;
    
    return 0;
}



int MatchForTempBatch(const char* templatesFolderName, const char* queryEdgeTXTName, const char* renderImageName)
{
    vector<string> paths;
	for (const auto & entry : fs::directory_iterator(templatesFolderName))
	{
		paths.push_back(entry.path());
	}
    for (auto i:paths)
    {
        MatchForSingleTemp(i.c_str(), queryEdgeTXTName, renderImageName);
    }

    return 0;
}

/**
 * @brief 基于视频帧的匹配过程
 * @param videoFileName 路网分割视频文件名（即模板视频）
 * @param queryImagePGM queryImagePGM的pgm格式文件名
 * @param queryImagePPM 用于显示匹配结果的图像路径
 * @param center 生成包围框的中心像素坐标
 */
int MatchVideo(const char* videoFileName, const char* queryImagePGM, const char* queryImagePPM)
{
	// 对queryEdgeMapPGM进行直线段拟合
	string queryImage = queryImagePGM;
	string queryEdgeMap;
	Image<uchar> *inputImage = NULL; // initialize a input image 
	inputImage = ImageIO::LoadPGM(queryImage.c_str());

	if(inputImage==NULL)
	{
		std::cerr<<"[ERROR] Fail in reading image "<< queryImagePGM <<std::endl;
		exit(0);
	}

	LFLineFitter lf_queryImage, lf_template;

	// 针对queryImage的linefitter
	lf_queryImage.Init(); // line fit
	// 配置文件读取
	lf_queryImage.Configure("../Config/LFlineFitterConfig.txt");
	// 对输入图像进行边线拟合，最终结果是lf_queryImage内部的outEdgeMap_成员变量保存的直线段信息，width_保存图像宽度，height_保存图像高度
	lf_queryImage.FitLine(inputImage); 
	
	// 读取视频帧并进行模板匹配
	VideoCapture cap;
    cap.open(videoFileName);

	if (!cap.isOpened())//如果视频不能正常打开则返回
		return 0;

	Mat frame;
	int frameID = 0;

	long totalFrame = static_cast<int>(cap.get(CAP_PROP_FRAME_COUNT));
	while (frameID < totalFrame)
	{
        cap >> frame;
        Mat gray_image;
        cvtColor(frame, gray_image, CV_BGR2GRAY);
		if (gray_image.empty())//如果某帧为空则退出循环
            break;
		cout << "Current frameID = " << frameID << endl;

		int width = gray_image.cols;
		int height = gray_image.rows;

		// 需要将frame转化为PGM后才能继续处理，因此一个中间文件存储template PGM
		const char* templatePGM = "/home/joey/Projects/faster-fdcm/templatePGM.pgm";
		std::ofstream file(templatePGM, std::ios::out | std::ios::binary); // 一次IO操作
		file<< "P5\n" << width << " " << height << "\n" << UCHAR_MAX << "\n"; 
		file.write((char *)gray_image.ptr<uchar>(0, 0), width * height * sizeof(uchar));
		file.close();

		// 模板直线段拟合
		lf_template.Init();
		lf_template.Configure("../Config/LFlineFitterConfig.txt");
		Image<uchar> *inputTemplate = NULL; // initialize a input image 
		inputTemplate = ImageIO::LoadPGM(templatePGM);

		if(inputTemplate==NULL)
		{
			std::cerr<<"[ERROR] Fail in reading image "<< templatePGM <<std::endl;
			exit(0);
		}
		lf_template.FitLine(inputTemplate); 

		LMLineMatcher lm;
		lm.Configure("/home/joey/Projects/faster-fdcm/Unit-test/Config/LMlineMatcherConfig.txt");

		// lm的Init()工作
		int numOfTemplatesForEachTemplateImage = 1;
		lm.ndbImages_ = numOfTemplatesForEachTemplateImage;
		lm.dbImages_ = new EIEdgeImage[lm.ndbImages_];
		for (int i = 0; i < lm.ndbImages_; i++)
		{
			EIEdgeImage * edgeImage = &lm.dbImages_[i];
			edgeImage->SetNumDirections(lm.nDirections_);
			edgeImage->width_ = lf_template.rWidth();
			edgeImage->height_ = lf_template.rHeight();
			edgeImage->nLines_ = lf_template.rNLineSegments();
			edgeImage->lines_ = lf_template.rOutputEdgeMap();
			edgeImage->SetLines2Grid();
			edgeImage->SetDirections();
			edgeImage->Scale(lm.scale_ * lm.db_scale_);
		}

		vector<LMDetWind> detWind;
		lm.Match(lf_queryImage, detWind);
		cout << "The Num of Matched successfully windows is: " << detWind.size() << endl;
		if (queryImagePPM)
    	{
			string MatchedImages = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/MatchedImages/";
			stringstream ss;
			Image<RGBMap> *debugImage = ImageIO::LoadPPM(queryImagePPM);
			// 在PPM模式的渲染图像中进行框取，按照左上角的坐标和宽高进行框取
			LMDisplay::DrawDetWind(debugImage,detWind[0].x_,detWind[0].y_,detWind[0].width_,detWind[0].height_,RGBMap(255,0,0),4);
			ss << MatchedImages << frameID << ".output.ppm";
			string outputname_str = ss.str();
			ImageIO::SavePPM(debugImage,outputname_str.c_str());
			delete debugImage;
    	}
		frameID++;
	}
	return 0;
}


#ifndef _video_capture_h_
#define _video_capture_h_

#include <cstdlib>
#include <climits>
#include <cstring>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/imgproc/imgproc_c.h>
#include "../Image/Image.h"
#include "../Image/ImageIO.h"

using namespace cv;
using namespace std;
namespace fs = std::filesystem;


/**
 * @brief 读取视频（.mp4）信息，打印视频图像宽度、高度，视频总帧数，视频帧率
 */
void GetVideoInfo(const char* filename)
{
	VideoCapture cap;
	cap.open(filename);
	
	if (!cap.isOpened())
		exit(0);
	int frameRate = static_cast<int>(cap.get(CAP_PROP_FPS));
	int totalFrames = static_cast<int>(cap.get(CAP_PROP_FRAME_COUNT));
	int width = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
	int height = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));

	cout << "视频宽度 = " << width << endl;
	cout << "视频高度 = " << height << endl;
	cout << "视频总帧数 = " << totalFrames << endl;
	cout << "帧率 = " << frameRate << endl;
	cap.release();
}


/**
 * @brief 视频（.mp4）逐帧读取
 */
int GetFramesFromVideo(const char* filename)
{
	VideoCapture cap;
	cap.open(filename);
	if (!cap.isOpened())
		exit(0);

	Mat frame;
	
	int currentFrame = 0;

	while(1)
	{

		if (!cap.read(frame))
		{
			cout << "读取视频失败" << endl;
			return -1;
		}

	 	if (frame.empty())
			break;

		stringstream str;
		str << "/home/joey/Pictures/Segmentations/" << currentFrame << ".png" << endl;
		imwrite(str.str(), frame);
		
		currentFrame++;

	}
	return 0;
}

int GetFramefromVideo(const char* filename = "/home/joey/Videos/上海近崇明岛6300m-2022-12-13_09.57.23.mp4")
{
    VideoCapture cap;
    cap.open(filename);
    
    if (!cap.isOpened())//如果视频不能正常打开则返回
            return 0;
    
    Mat frame;
	int frameID = 0;
    
    double t = 0, fps = 0;

	long totalFrame = static_cast<int>(cap.get(CAP_PROP_FRAME_COUNT));
    
    while (frameID < totalFrame)
    {
        t = (double)cv::getTickCount();

        //方法1：
        //cap >> frame;
		
        //方法2：
        //cap.read(frame);
    
        //方法3：
        cap.grab();
        cap.retrieve(frame);
    
    
        if (frame.empty())//如果某帧为空则退出循环
            break;
        
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        fps = 1.0 / t;

		cout << "Current frameID = " << frameID << endl;
    
        stringstream str;
        str << "/home/joey/Pictures/RemoteSensoringImage/" <<frameID<<".png";
		imwrite(str.str(), frame);
		frameID++;
    }

    return 0;
}

/**
 * @brief 将输入图像转化为pgm格式的图像
 * @param filename 输出的pgm文件名字
 * @param imagePNG 输入的IplImage*文件
 */
void cvConvertoPNG2PGM(const char* filename, const char* imagePNG)
{
	cv::Mat frame = imread(imagePNG,0);
	int width = frame.cols;
	int height = frame.rows;

	std::ofstream file(filename, std::ios::out | std::ios::binary);

	file << "P5\n" << width << " " << height << "\n" << UCHAR_MAX << "\n";
	file.write((char *)frame.ptr<uchar>(0, 0), width * height * sizeof(uchar));
	file.close();
}


/**
 * @brief png图像转换为pgm图像批处理
 * @param outfolderPGM pgm图像集的存储地址
 * @param infolderPNG png图像集的存储地址
 */
void cvConvertoPNG2PGM_Batch(const char* outfolderPGM, const char* infolderPNG)
{
	vector<string> paths;
	for (const auto & entry : fs::directory_iterator(infolderPNG))
	{
		paths.push_back(entry.path());
	}
	for (int i = 0; i < paths.size(); i++)
	{
		stringstream infolderName(paths[i]);
		char split = '/';
		vector<string> results;
		string str;

		while (getline(infolderName, str, split))
		{
			results.push_back(str);
		}
		
		stringstream ss;
		ss << outfolderPGM << str << ".pgm";
		string filename = ss.str();
		cvConvertoPNG2PGM(filename.c_str(), paths[i].c_str());
	}
}

void PrintFilesNameOfDirectory(string directory)
{
	for (const auto & entry : fs::directory_iterator(directory))
		cout << entry.path() << endl;
}

#endif
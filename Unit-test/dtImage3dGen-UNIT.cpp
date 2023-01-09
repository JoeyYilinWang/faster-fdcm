#include "../Image/Image.h"
#include "../Image/ImageIO.h"
#include "../Image/DistanceTransform.h"
#include "../Fdcm/EIEdgeImage.h"
#include "../LineFit/LineFitter.h"
#include "../LineFit/LineSegment.h"
#include "../Fdcm/LMDistanceImage.h"

#include <iostream>
using namespace std;
using namespace cv;

// 测试二维距离变换积分图生成单元
int main(int argc, char* argv[])
{
    string LineFittedImageName;
    if ( argc != 2)
    {
        cout <<"[Syntax] dtImage3dGen-UNIT lineFittedImage.pgm"<<std::endl;
        cout << "Switch to default parameters" << endl;    
        LineFittedImageName = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/queryImages/ShanghaiNetlines.pgm";
        cout << "LineFittedImageName: " << LineFittedImageName << endl;
    }
    else
    {
        LineFittedImageName = argv[1];
    }

    Image<uchar> *inputImage=NULL;
    inputImage = ImageIO::LoadPGM(LineFittedImageName.c_str());
 
    if(inputImage==NULL)
	{
		std::cerr<<"[ERROR] Fail in reading image "<<LineFittedImageName<<std::endl;
		exit(0);
	}
    
    Image<float> dtImage(inputImage->width(), inputImage->height(), true);
    Image<float> *ptrDTimage = &dtImage;
    DistanceTransform::CompDT(inputImage, ptrDTimage, false);

    // 初始化EIEdgeImage对象
    EIEdgeImage queryImage;

    // 设定60个方向
    int nDirections = 60;
    queryImage.SetNumDirections(nDirections);

    // 初始化Line Fitter
    LFLineFitter lf;


    // 从EdgeMap.txt中获取直线段信息
    const char* edgeMapTXT = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/queryImages/ShanghaiNetlines.txt";
    lf.LoadEdgeMap(edgeMapTXT);

    queryImage.Read(lf);
    queryImage.Scale(1);
    LMDistanceImage queryDistanceImage;
    queryDistanceImage.SetImage(queryImage);
    cout << "3d Distance Transform Integral Image success" << endl;
}

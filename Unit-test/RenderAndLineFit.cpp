#include "../OSM/Element.h"
#include "../OSM/ImageDraw.h"
#include "../Image/ImageIO.h"
#include "../LineFit/LineFitter.h"
#include "../Image/DistanceTransform.h"
#include "../Image/Image.h"


// test OSM nodes rendering unit
int main(int argc,char *argv[])
{

    // 验证参数给定正确性
    string node_csv ;
    string link_csv ;
    double x, y, z;
    string outputImage;
	if(argc < 7)
	{
		//std::cerr<<"[Syntax] render_unit node.csv link.csv x y height outputImage.png"<<std::endl;
        // exit(0);
		cout <<"[Syntax] render_unit node.csv link.csv x y height outputImage.png"<<std::endl;
        cout << "Switch to default parameters" << endl;
        node_csv = "/home/joey/Projects/OSM_Analysis/output/node.csv";
        link_csv = "/home/joey/Projects/OSM_Analysis/output/link.csv";
        x = 0; y = 0; z = 5000;
        outputImage = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/queryImages/shanghaiEdge.png";
	}
    else
    {
        node_csv = argv[1];
        link_csv = argv[2];
        x = atof(argv[3]);
        y = atof(argv[4]);
        z = atof(argv[5]);
        outputImage = argv[6];       
    }

    nodes Nodes;
    vector<vector<string>> data;
    
    Nodes.ReadFromCSV(node_csv, data);
    Nodes.Extract(data);

    data.clear();
    links Links;
    Links.ReadFromCSV(link_csv, data);
    Links.Extract(data);
    
    // 相机内参假设（使用VINS MONO的相机内参）
    cv::Mat intrisicMat(3, 3, cv::DataType<float>::type);
    intrisicMat.at<float>(0, 0) = 4.616e+02;
    intrisicMat.at<float>(1, 0) = 0;
    intrisicMat.at<float>(2, 0) = 0;
 
    intrisicMat.at<float>(0, 1) = 0;
    intrisicMat.at<float>(1, 1) = 4.603e+02;
    intrisicMat.at<float>(2, 1) = 0;
    
    intrisicMat.at<float>(0, 2) = 3.630e+02;
    intrisicMat.at<float>(1, 2) = 2.481e+02;
    intrisicMat.at<float>(2, 2) = 1;

    // 将旋转矩阵转化为旋转向量
    cv::Mat rVec(3, 1, cv::DataType<float>::type);
    // cv::Rodrigues(R, rVec);
    // cout << rVec << endl;
    
    // 固定姿态，相机始终垂直向下
    rVec.at<float>(0) = 0;
    rVec.at<float>(1) = 0;
    rVec.at<float>(2) = 0;
    
    cv::Mat tVec(3, 1, cv::DataType<float>::type); // Translation vector
    tVec.at<float>(0) = x;
    tVec.at<float>(1) = y; 
    tVec.at<float>(2) = z;

    cv::Mat distCoeffs(5, 1, cv::DataType<float>::type);   // Distortion vector
    distCoeffs.at<float>(0) = 0;
    distCoeffs.at<float>(1) = 0;
    distCoeffs.at<float>(2) = 0;
    distCoeffs.at<float>(3) = 0;
    distCoeffs.at<float>(4) = 0;

    Nodes.LongLatHeight2ENU();

    Nodes.ENU2NED();
 
    Nodes.World2Img(intrisicMat, distCoeffs, rVec, tVec);
    
    // 定义图像画布，设定图像分辨率
    int w = 1280;
    int h = 720;
    cv::Mat binImage = cv::Mat::zeros(h, w, CV_8UC1);

    
    // 在binImage上画Links
    DrawLinesOfLinks(binImage, Links, Nodes);    

    SaveImage(binImage, outputImage.c_str());  

    const char *outputImagePGM = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/queryImages/shanghaiEdgePGM.pgm";

    ImageIO::SavePGM(binImage, outputImagePGM);

    Image<uchar> *inputImage = ImageIO::LoadPGM(outputImagePGM);

	if(inputImage==NULL)
	{
		std::cerr<<"[ERROR] Fail in reading image "<< outputImagePGM <<std::endl;
		exit(0);
	}

    LFLineFitter lf;

    lf.Init(); // line fit
	
	// 配置文件读取
	lf.Configure("/home/joey/Projects/faster-fdcm/Unit-test/Config/LFlineFitterConfig.txt");
	
	// 对输入图像进行边线拟合
	lf.FitLine(inputImage); 
    
    // const char* outEdgeMap = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/queryImages/shanghaiEdgePGMSimplifed.pgm";
	// // 将被拟合出来的直线段进行图像化显示
	// lf.DisplayEdgeMap(inputImage,outEdgeMap);	
    const char* outEdgeMap = "/home/joey/Projects/faster-fdcm/Query/four_edges.pgm";
    Image<uchar> *inputSimplifiedEdgeMapName = ImageIO::LoadPGM(outEdgeMap);

    if(inputImage==NULL)
	{
		std::cerr<<"[ERROR] Fail in reading image "<<outEdgeMap<<std::endl;
		exit(0);
	}
    
    Image<float> dtImage(w, h, true);
    Image<float>* ptrdtImage = &dtImage;
    DistanceTransform::CompDT(inputSimplifiedEdgeMapName, ptrdtImage, false);

	//cvReleaseImage(&inputImage);
	delete inputImage;

    return 0;
}  
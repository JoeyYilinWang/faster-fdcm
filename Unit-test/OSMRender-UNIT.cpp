#include "../OSM/Element.h"
#include "../OSM/ImageDraw.h"
#include "../Image/ImageIO.h"

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
		//std::cerr<<"[Syntax] render_unit node.csv link.csv x y height outputImage.pgm"<<std::endl;
        // exit(0);
		cout <<"[Syntax] render_unit node.csv link.csv x y height outputImage.pgm"<<std::endl;
        cout << "Switch to default parameters" << endl;
        node_csv = "/home/joey/Projects/OSM_Analysis/output/node.csv";
        link_csv = "/home/joey/Projects/OSM_Analysis/output/link.csv";
        x = 0; y = 0; z = 5000; 
        outputImage = "/home/joey/Projects/faster-fdcm/Unit-test/OutputImages/queryImages/ShanghaiNet.pgm";
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
    
    // 预定义的旋转矩阵（根据ENU到相机坐标的旋转关系实际计算得来）
    // cv::Mat R(3, 3, cv::DataType<float>::type);
    // R.at<float>(0, 0) = 0;
    // R.at<float>(0, 1) = -1;
    // R.at<float>(0, 2) = 0;
    // R.at<float>(1, 0) = -1;
    // R.at<float>(1, 1) = 0;
    // R.at<float>(1, 2) = 0;
    // R.at<float>(2, 0) = 0;
    // R.at<float>(2, 1) = 0;
    // R.at<float>(2, 2) = -1;

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
    // distCoeffs.at<float>(0) = -7.9134632415085826e-001;
    // distCoeffs.at<float>(1) = 1.5623584435644169e+000;
    // distCoeffs.at<float>(2) = -3.3916502741726508e-002;
    // distCoeffs.at<float>(3) = -1.3921577146136694e-002;
    // distCoeffs.at<float>(4) = 1.1430734623697941e-002;

    Nodes.LongLatHeight2ENU();

    Nodes.ENU2NED();
 
    Nodes.World2Img(intrisicMat, distCoeffs, rVec, tVec);
    
    // 定义图像画布，设定图像分辨率
    // 其实固定相机内参和畸变参数之后，相机成像效果就固定了，这里设置的分辨率其实就是设定的显示尺寸
    int w = 1280;
    int h = 720;
    cv::Mat binImage = cv::Mat::zeros(h, w, CV_8UC1);
        
    // 在binImage上画节点
    // DrawNodes(binImage, w, h, Nodes);
    
    // 在binImage上画Links
    DrawLinesOfLinks(binImage, Links, Nodes);    
    
    // Show image
    // cv::namedWindow("binImage", cv::WINDOW_NORMAL);
    // cv::imshow("binImage", binImage);
    // cv::waitKey();

    ImageIO::SavePGM(binImage, outputImage.c_str());

    return 0;
}  
#include "../OSM/Element.h"
#include "../OSM/ImageDraw.h"
#include "../Image/ImageIO.h"
using namespace std;

// 将render某些成员变量定义为静态全局变量，目的是保证该变量能够被其他类或函数调用
class render
{
    public:
    static nodes Nodes;
    static links Links;
    static cv::Mat intrisicMat;
    static cv::Mat distCoeffs;
    static double render_height;
    static vector<int> image_size;
    void Render(string node_csv_file, string link_csv_file, string rendered_Image);

}
;


void render::Render(string node_csv_file, string link_csv_file, string rendered_Image)
{
    string node_csv;
    string link_csv;
    double x = 0;
    double y = 0;
    double z = render_height;
    string outputImage = rendered_Image;

    vector<vector<string>> data;
    
    Nodes.ReadFromCSV(node_csv, data);
    Nodes.Extract(data);

    data.clear();
    Links.ReadFromCSV(link_csv, data);
    Links.Extract(data);

    cv::Mat rVec(3, 1, cv::DataType<float>::type);
    // 固定姿态，相机始终垂直向下
    rVec.at<float>(0) = 0;
    rVec.at<float>(1) = 0;
    rVec.at<float>(2) = 0;

    cv::Mat tVec(3, 1, cv::DataType<float>::type); // Translation vector
    tVec.at<float>(0) = x;
    tVec.at<float>(1) = y; 
    tVec.at<float>(2) = z;

    Nodes.LongLatHeight2ENU();

    Nodes.ENU2NED();
 
    Nodes.World2Img(intrisicMat, distCoeffs, rVec, tVec);

    int h = image_size[-1];
    int w = image_size[-2];

    cv::Mat binImage = cv::Mat::zeros(h, w, CV_8UC1);
    
    DrawLinesOfLinks(binImage, Links, Nodes);    

    ImageIO::SavePGM(binImage, outputImage.c_str());

}
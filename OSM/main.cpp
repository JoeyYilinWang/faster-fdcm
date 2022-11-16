#include "Element.h"
#include "ImageDraw.h"
// test OSM nodes rendering unit
int main(int argc,char *argv[])
{
    nodes Nodes;
    vector<vector<string>> data;

    // test if nodes object function correctly, test correctly
    Nodes.ReadFromCSV("/home/joey/Projects/OSM_Analysis/output/node.csv", data);
    Nodes.Extract(data);

    // test links::ReadFromCSV and links::Extract functions
    data.clear();
    links Links;
    Links.ReadFromCSV("/home/joey/Projects/OSM_Analysis/output/link.csv", data);
    Links.Extract(data);
    
    // test nodes::World2Img member function
    cv::Mat intrisicMat(3, 3, cv::DataType<float>::type);
    intrisicMat.at<float>(0, 0) = 1.6415318549788924e+003;
    intrisicMat.at<float>(1, 0) = 0;
    intrisicMat.at<float>(2, 0) = 0;
 
    intrisicMat.at<float>(0, 1) = 0;
    intrisicMat.at<float>(1, 1) = 1.7067753507885654e+003;
    intrisicMat.at<float>(2, 1) = 0;
 
    intrisicMat.at<float>(0, 2) = 5.3262822453148601e+002;
    intrisicMat.at<float>(1, 2) = 3.8095355839052968e+002;
    intrisicMat.at<float>(2, 2) = 1;
 
    cv::Mat rVec(3, 1, cv::DataType<float>::type); // Rotation vector
    rVec.at<float>(0) = -3.9277902400761393e-002;
    rVec.at<float>(1) = 3.7803824407602084e-002;
    rVec.at<float>(2) = 2.6445674487856268e-002;
 
    cv::Mat tVec(3, 1, cv::DataType<float>::type); // Translation vector
    tVec.at<float>(0) = 2.1158489381208221e+000;
    tVec.at<float>(1) = -7.6847683212704716e+000; 
    cv::Mat distCoeffs(5, 1, cv::DataType<float>::type);   // Distortion vector
    distCoeffs.at<float>(0) = -7.9134632415085826e-001;
    distCoeffs.at<float>(1) = 1.5623584435644169e+000;
    distCoeffs.at<float>(2) = -3.3916502741726508e-002;
    distCoeffs.at<float>(3) = -1.3921577146136694e-002;
    distCoeffs.at<float>(4) = 1.1430734623697941e-002;

    Nodes.LongLatHeight2ENU();
    for (int i = 0; i < 3; i++)
    {
        cout << Nodes.nodes[i].E_coord << ", " << Nodes.nodes[i].N_coord << ", " << Nodes.nodes[i].U_coord << endl;
    }

    Nodes.ENU2NED();
    for (int i = 0; i < 3; i++)
    {
        cout << Nodes.nodes[i].point_NED << endl;
    }
    // Nodes.World2Img(intrisicMat, distCoeffs, rVec, tVec);
    
    // for (int i = 0; i < 3; i++)
    // {
    //     cout << Nodes.nodes[i].point_P << endl;
    // }

    // test function draw points 
    // cv::Mat binImage;
    // int w = 1920;
    // int h = 1080;
    // DrawNodes(binImage, w, h, Nodes);

    return 0;
}  
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <GeographicLib/LocalCartesian.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/calib3d.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/imgproc/imgproc_c.h>


#ifndef _ELEMENT_H
#define _ELEMENT_H
using namespace std;
struct node
{
    public:
    unsigned int node_id;
    unsigned int osm_node_id;
    double x_coord; // longtitude in degree
    double y_coord; // latitude in degree
    double z_coord = 0; // height in m
    double E_coord;
    double N_coord;
    double U_coord;
    double x_body;
    double y_body;
    double z_body;
    double x_cam;
    double y_cam;
    double z_cam;
    cv::Point2d point_P;
    cv::Vec3d point_NED;
    ~node();
};

class nodes
{
    public:
    vector<node> nodes;
    void ReadFromCSV(string str_csv_file, vector<vector<string>> &data);
    void Extract(vector<vector<string>> &data);  
    void LongLatHeight2ENU(vector<double> &Origin_ENU);
    void LongLatHeight2ENU();
    void ENU2Body(cv::Matx33f &Rotate);
    void ENU2NED();
    void Body2Cam(cv::Matx44f &T);
    void World2Img(const cv::Mat &intrinsicMat, const cv::Mat distCoeffs, const cv::Mat &rVec, const cv::Mat &tVec);
    void AppendZcoord(double &z);

    ~nodes();
};

struct link
{
    public:
    unsigned int link_id;
    unsigned int osm_way_id;
    unsigned int from_node_id;
    unsigned int to_node_id;
    double length;
    ~link();
};

class links
{
    public:
    vector<link> links;
    void ReadFromCSV(string str_csv_file, vector<vector<string>> & data);
    void Extract(vector<vector<string>> &data); 
    ~links();
};

void findLinkFromNodeCoord(link &Link, nodes &Nodes, pair<double, double> &coord);
void findLinkToNodeCoord(link &Link, nodes &Nodes, pair<double, double> &coord);
void AppendZcoord(nodes &Nodes, double &z);

#endif